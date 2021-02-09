/*!
 * \file reader.cpp
 *
 * \brief 封装ffmpeg对视频流的操作
 * \author Yue Wu,cyy
 */

#include <iostream>
#include <memory>
#include <mutex>
#include <regex>
#include <thread>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavformat/avio.h>
#include <libavutil/imgutils.h>
#include <libavutil/pixfmt.h>
#include <libswscale/swscale.h>
}

#include "data_structure/thread_safe_container.hpp"
#include "ffmpeg_base.hpp"
#include "ffmpeg_video_reader.hpp"
#include "log/log.hpp"
#include "util/runnable.hpp"

namespace cyy::naive_lib::video::ffmpeg {

  //! \brief 封装ffmpeg对视频流的讀操作
  template <bool decode_frame>
  class reader_impl : private cyy::naive_lib::runnable {
  public:
    reader_impl() {}

    ~reader_impl() override { close(); }

    //! \brief 打开视频
    //! \param url 视频地址，如果是本地文件，使用file://协议
    //! \note 先关闭之前打开的视频再打开此url对应的视频
    bool open(const std::string &url) {
      init_library();
      int ret = 0;
      this->close();

      input_ctx = avformat_alloc_context();
      if (!input_ctx) {
        LOG_ERROR("avformat_alloc_context failed");
        return false;
      }

      if constexpr (decode_frame) {
        input_ctx->interrupt_callback.callback = interrupt_cb;
        input_ctx->interrupt_callback.opaque = this;
      }

      std::regex scheme_regex("([a-z][a-z0-9+-.]+)://",
                              std::regex_constants::ECMAScript |
                                  std::regex_constants::icase);

      std::smatch match;
      if (std::regex_search(url, match, scheme_regex)) {
        url_scheme = match[1].str();
        for (auto &c : url_scheme) {
          if (c >= 'A' && c <= 'Z') {
            c = c - 'A' + 'a';
          }
        }
      } else {
        url_scheme = "file";
      }
      if (is_live_stream()) {
        ret = av_dict_set(&opts, "rtsp_transport", "tcp", 0);
        if (ret != 0) {
          LOG_ERROR("av_dict_set failed:{}", errno_to_str(ret));
          return false;
        }
        ret = av_dict_set(&opts, "stimeout", "2000000", 0); // us
        if (ret != 0) {
          LOG_ERROR("av_dict_set failed:{}", errno_to_str(ret));
          return false;
        }
      }
      if (url_scheme != "file") {
        ret = av_dict_set(&opts, "allowed_media_types", "video", 0);
        if (ret != 0) {
          LOG_ERROR("av_dict_set failed:{}", errno_to_str(ret));
          return false;
        }

        ret = av_dict_set(&opts, "fflags", "nobuffer", 0);
        if (ret != 0) {
          LOG_ERROR("av_dict_set failed:{}", errno_to_str(ret));
          return false;
        }
      }

      ret = avformat_open_input(&input_ctx, url.c_str(), nullptr, &opts);
      if (ret != 0) {
        LOG_ERROR("avformat_open_input {} failed:{}", url, errno_to_str(ret));
        return false;
      }

      ret = avformat_find_stream_info(input_ctx, nullptr);
      if (ret < 0) {
        LOG_ERROR("avformat_find_stream_info failed:{}", errno_to_str(ret));
        return false;
      }

      ret = av_find_best_stream(input_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr,
                                0);
      if (ret < 0) {
        LOG_ERROR("av_find_best_stream failed");
        return false;
      }

      stream_index = ret;
      auto video_stream = input_ctx->streams[ret];

      AVCodec *codec = nullptr;

      //尝试用nvidia解码
      if (video_stream->codecpar->codec_id == AV_CODEC_ID_H264) {
        codec = avcodec_find_decoder_by_name("h264_cuvid");
      }
      if (!codec) {
        codec = avcodec_find_decoder(video_stream->codecpar->codec_id);
      }
      if (!codec) {
        LOG_ERROR("can't find decoder for {}", url);
        return false;
      }
      LOG_WARN("use codec {}", codec->long_name);
      decode_ctx = avcodec_alloc_context3(codec);
      if (!decode_ctx) {
        LOG_ERROR("avcodec_alloc_context3 failed");
        return false;
      }

      ret = avcodec_parameters_to_context(decode_ctx, video_stream->codecpar);
      if (ret < 0) {
        LOG_ERROR("avcodec_parameters_to_context failed:{}", errno_to_str(ret));
        return false;
      }

      ret = avcodec_open2(decode_ctx, nullptr, nullptr);
      if (ret < 0) {
        LOG_ERROR("avcodec_open2 failed:{}", errno_to_str(ret));
        return false;
      }

      video_width = video_stream->codecpar->width;
      video_height = video_stream->codecpar->height;
      if (video_width <= 0 || video_height <= 0) {
        LOG_ERROR("invalid video size [{} * {}]", video_width, video_height);
        return false;
      }

      avframe = av_frame_alloc();
      if (!avframe) {
        LOG_ERROR("av_frame_alloc failed");
        return false;
      }

      opened = true;

      if (is_live_stream()) {
        if constexpr (decode_frame) {
          frame_buffer =
              std::make_unique<cyy::naive_lib::thread_safe_linear_container<
                  std::vector<std::pair<int, frame>>>>();

        } else {
          packet_buffer =
              std::make_unique<cyy::naive_lib::thread_safe_linear_container<
                  std::vector<std::pair<int, std::shared_ptr<AVPacket>>>>>();
        }
        start("ffmpeg_reader_impl");
      }

      return true;
    }

    bool has_open() const { return opened; }

    //! \brief 获取下一個AVPacket
    //! \return first>0 成功
    //	      first=0 EOF
    //	      first<0 失敗
    //	如果first<=0，返回空内容

    std::pair<int, std::shared_ptr<AVPacket>> next_packet() {
      if (!has_open()) {
        LOG_ERROR("video is not opened");
        return {-1, {}};
      }

      if (url_scheme == "rtsp") {
        auto packet_opt = packet_buffer->pop_front(std::chrono::seconds(5));
        if (!packet_opt) {
          LOG_ERROR("pop packet timeout");
          return {-1, {}};
        }
        return packet_opt.value();
      }
      if (url_scheme == "file" && play_frame_rate) {
        if (!next_play_time) {
          next_play_time =
              std::chrono::steady_clock::now() +
              std::chrono::milliseconds(1000 * play_frame_rate.value()[1] /
                                        play_frame_rate.value()[0]);
        } else {
          std::this_thread::sleep_until(next_play_time.value());
          next_play_time =
              next_play_time.value() +
              std::chrono::milliseconds(1000 * play_frame_rate.value()[1] /
                                        play_frame_rate.value()[0]);
        }
      }

      return get_packet();
    }

    //! \brief 获取下一帧
    //! \return first>0 成功
    //	      first=0 EOF
    //	      first<0 失敗
    //	如果first<=0，返回空内容

    std::pair<int, frame> next_frame() {
      if (!has_open()) {
        LOG_ERROR("video is not opened");
        return {-1, {}};
      }

      std::pair<int, frame> p;
      if (is_live_stream()) {
        auto frame_opt = frame_buffer->pop_front(std::chrono::seconds(5));
        if (!frame_opt) {
          LOG_ERROR("pop frame timeout");
          return {-1, {}};
        }
        p = frame_opt.value();
      } else {
        p = get_frame();
      }
      if (p.first <= 0) {
        return p;
      }

      if (url_scheme == "file" && play_frame_rate) {
        if (!next_play_time) {
          next_play_time =
              std::chrono::steady_clock::now() +
              std::chrono::milliseconds(1000 * play_frame_rate.value()[1] /
                                        play_frame_rate.value()[0]);
        } else {
          std::this_thread::sleep_until(next_play_time.value());
          next_play_time =
              next_play_time.value() +
              std::chrono::milliseconds(1000 * play_frame_rate.value()[1] /
                                        play_frame_rate.value()[0]);
        }
      }
      return p;
    }

    //! \brief 获取視頻寬
    //! \note 如果失败，返回-1
    int get_video_width() const { return video_width; }
    //! \brief 获取視頻高
    //! \note 如果失败，返回-1
    int get_video_height() const { return video_height; }

    //! \brief 获取视频帧率
    //! \note 如果无法获取视频帧率，則返回空
    std::optional<std::array<size_t, 2>> get_frame_rate() {
      if (!has_open()) {
        LOG_ERROR("video is not opened");
        return {};
      }
      auto res = av_guess_frame_rate(input_ctx,
                                     input_ctx->streams[stream_index], nullptr);

      if (res.den <= 0 || res.num <= 0) {
        LOG_ERROR("invalid frame rate [{} {}]", res.num, res.den);
        return {};
      }
      return {{static_cast<size_t>(res.num), static_cast<size_t>(res.den)}};
    }

    //! 設置播放的幀率，用於控制next_frame的速度
    void set_play_frame_rate(const std::array<size_t, 2> &frame_rate) {
      if (play_frame_rate && play_frame_rate.value() == frame_rate) {
        return;
      }
      play_frame_rate = frame_rate;
      next_play_time.reset();
    }

    std::optional<AVCodecParameters *> get_codec_parameters() {
      if (!has_open()) {
        LOG_ERROR("video is not opened");
        return {};
      }
      auto stream = input_ctx->streams[stream_index];
      return {stream->codecpar};
    }

    //! \brief 关闭已经打开的视频，如果之前没调用过open，调用该函数无效果
    void close() {
      stop();
      frame_buffer.reset();
      packet_buffer.reset();

      if (sws_ctx) {
        sws_freeContext(sws_ctx);
        sws_ctx = nullptr;
      }

      if (avframe) {
        av_frame_free(&avframe);
        avframe = nullptr;
      }

      if (decode_ctx) {
        avcodec_free_context(&decode_ctx);
        decode_ctx = nullptr;
      }

      if (input_ctx) {
        avformat_close_input(&input_ctx);
        avformat_free_context(input_ctx);
        input_ctx = nullptr;
      }
      if (opts) {
        av_dict_free(&opts);
        opts = nullptr;
      }

      stream_index = -1;
      video_width = -1;
      video_height = -1;
      opened = false;
      // frame_seq我們不清零
    }

    void drop_non_key_frames() {
      add_frame_filter("key_frame",
                       [](auto const &frame) { return is_key_frame(frame); });
    }

  private:
    static int interrupt_cb(void *ctx) {
      if (reinterpret_cast<reader_impl<decode_frame> *>(ctx)->needs_stop()) {
        LOG_WARN("stop decode thread");
        return 1;
      }
      return 0;
    }
    static bool is_key_frame(const AVFrame &frame) {

      return ((frame.key_frame == 1) || (frame.pict_type == AV_PICTURE_TYPE_I));
    }

    void add_frame_filter(std::string_view name,
                          std::function<bool(const AVFrame &)> filter) {
      frame_filters.emplace(name, filter);
    }

    void run() override {
      avformat_flush(input_ctx);
      while (!needs_stop()) {
        if constexpr (decode_frame) {
          auto res = get_frame();
          frame_buffer->push_back(res);
          if (res.first <= 0) {
            LOG_ERROR("get frame failed,thread exit");
            break;
          }
        } else {
          auto res = get_packet();
          packet_buffer->push_back(res);
          if (res.first <= 0) {
            LOG_ERROR("get packet failed,thread exit");
            break;
          }
        }
      }
    }

    //! \brief 获取下一AVPacket
    //! \return >0 成功
    //	      =0 EOF
    //	      <0 失敗
    //	如果<=0，返回空内容
    int get_packet(AVPacket &packet) {

      av_packet_unref(&packet);
      while (true) {
        auto ret = av_read_frame(input_ctx, &packet);
        if (ret == AVERROR_EOF) {
          return 0;
        }
        if (ret != 0) {
          LOG_ERROR("av_read_frame failed:{}", errno_to_str(ret));
          return -1;
        }

        if (packet.stream_index == stream_index) { // packet is video
          break;
        }
      }

      return 1;
    }

    //! \brief 获取下一AVPacket
    //! \return first>0 成功
    //	      first=0 EOF
    //	      first<0 失敗
    //	如果first<=0，返回空内容
    std::pair<int, std::shared_ptr<AVPacket>> get_packet() {
      if (!has_open()) {
        LOG_ERROR("reader is not opened");
        return {-1, nullptr};
      }

      auto packet = av_packet_alloc();
      if (!packet) {
        LOG_ERROR("av_packet_alloc failed");
        return {-1, nullptr};
      }

      std::shared_ptr<AVPacket> packet_ptr(
          packet, [](auto pkg) { av_packet_free(&pkg); });
      auto res = get_packet(*packet);
      if (res <= 0) {
        return {res, nullptr};
      }

      //根據av_read_frame的註釋，爲了保證返回的packet在下次av_read_frame調用後不失效，我們做一些處理
      if (packet->buf) {
        return {1, packet_ptr};
      }

      auto packet2 = av_packet_clone(packet);
      if (!packet2) {
        LOG_ERROR("av_packet_clone failed");
        return {-1, nullptr};
      }

      packet_ptr.reset(packet2);
      return {1, packet_ptr};
    }

    //! \brief 获取下一帧
    //! \return first>0 成功
    //	      first=0 EOF
    //	      first<0 失敗
    //	如果first<=0，返回空内容
    std::pair<int, frame> get_frame() {

      //我们在循环中不断解码直到成功获取一帧或者失败
      const enum AVPixelFormat pix_fmt { AV_PIX_FMT_BGR24 };

      if (!has_open()) {
        LOG_ERROR("reader is not opened");
        return {-1, {}};
      }

      auto packet = av_packet_alloc();
      if (!packet) {
        LOG_ERROR("av_packet_alloc failed");
        return {-1, {}};
      }

      std::shared_ptr<AVPacket> packet_ptr(
          packet, [](auto pkg) { av_packet_free(&pkg); });

      while (true) {
        auto ret = avcodec_receive_frame(decode_ctx, avframe);
        if (ret == 0) {
          frame_seq++;
          bool pass_filters = true;
          for (auto const &[name, filter] : frame_filters) {
            if (!filter(*avframe)) {
              pass_filters = false;
              break;
            }
          }
          if (pass_filters) {
            break;
          }
          LOG_DEBUG("ignore frame seq {}", frame_seq - 1);
          continue;
        }
        if (ret != AVERROR(EAGAIN)) {
          LOG_ERROR("avcodec_receive_frame failed:{}", errno_to_str(ret));
          return {-1, {}};
        }

        ret = get_packet(*packet);
        if (ret <= 0) {
          return {ret, {}};
        }

        ret = avcodec_send_packet(decode_ctx, packet);
        if (ret != 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_INVALIDDATA) {
          LOG_ERROR("avcodec_send_packet failed:{}", errno_to_str(ret));
          return {-1, {}};
        }
      }

      auto new_sws_ctx = sws_getCachedContext(
          sws_ctx, avframe->width, avframe->height,
          static_cast<enum AVPixelFormat>(avframe->format), video_width,
          video_height, pix_fmt, SWS_BICUBIC, nullptr, nullptr, nullptr);

      if (!new_sws_ctx) {
        LOG_ERROR("sws_getContext failed");
        return {-1, {}};
      }
      if(sws_ctx && sws_ctx !=new_sws_ctx) {
        sws_freeContext(sws_ctx);
        LOG_WARN("sws_freeContext");
      }
      sws_ctx=new_sws_ctx;

      frame new_frame;
      new_frame.seq = frame_seq - 1;
      new_frame.is_key = is_key_frame(*avframe);

      uint8_t *dst_data[4]{};
      int dst_linesize[4]{};

      new_frame.content = cv::Mat(video_height, video_width, CV_8UC3);
      dst_linesize[0] = new_frame.content.step[0];
      /* dst_linesize[1] = new_frame.content.step[1]; */

      auto ret = av_image_fill_pointers(dst_data, pix_fmt, video_height,
                                        new_frame.content.data, dst_linesize);

      if (ret <= 0) {
        LOG_ERROR("av_image_fill_pointers failed:{}", errno_to_str(ret));
        return {-1, {}};
      }

      ret = sws_scale(sws_ctx, avframe->data, avframe->linesize, 0,
                      avframe->height, dst_data, dst_linesize);

      if (ret <= 0) {
        if (ret < 0) {
          LOG_ERROR("sws_scale failed:{}", errno_to_str(ret));
        } else {
          LOG_ERROR("sws_scale failed");
        }
        return {-1, {}};
      }

      LOG_DEBUG("new frame");
      return {1, new_frame};
    }

    bool is_live_stream() const { return url_scheme == "rtsp"; }

  private:
    int stream_index{-1};
    uint64_t frame_seq{0};
    int video_width{-1};
    int video_height{-1};
    bool opened{false};
    std::string url_scheme;

    std::optional<std::array<size_t, 2>> play_frame_rate;
    std::optional<std::chrono::time_point<std::chrono::steady_clock>>
        next_play_time;
    AVFormatContext *input_ctx{nullptr};
    AVCodecContext *decode_ctx{nullptr};
    AVDictionary *opts{nullptr};
    AVFrame *avframe{nullptr};
    SwsContext *sws_ctx{nullptr};

    std::map<std::string, std::function<bool(const AVFrame &)>> frame_filters;
    std::unique_ptr<cyy::naive_lib::thread_safe_linear_container<
        std::vector<std::pair<int, frame>>>>
        frame_buffer;
    std::unique_ptr<cyy::naive_lib::thread_safe_linear_container<
        std::vector<std::pair<int, std::shared_ptr<AVPacket>>>>>
        packet_buffer;
  };
} // namespace cyy::naive_lib::video::ffmpeg
