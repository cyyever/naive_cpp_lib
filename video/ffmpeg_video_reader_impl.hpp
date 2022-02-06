/*!
 * \file ffmpeg_video_reader_impl.hpp
 *
 * \brief 封装ffmpeg对视频流的操作
 * \author Yue Wu,cyy
 */

#pragma once
#include <iostream>
#include <memory>
#include <stdlib.h>
#include <thread>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavformat/avio.h>
#include <libavutil/imgutils.h>
#include <libavutil/pixfmt.h>
#include <libswscale/swscale.h>
}

#include <cyy/algorithm/thread_safe_container.hpp>

#include "ffmpeg_base.hpp"
#include "ffmpeg_video_reader.hpp"
#include "log/log.hpp"
#include "util/runnable.hpp"

namespace cyy::naive_lib::video {

  //! \brief 封装ffmpeg对视频流的讀操作
  template <bool decode_frame>
  class ffmpeg_reader_impl : private cyy::naive_lib::runnable,
                             public ffmpeg_base {
  public:
    ~ffmpeg_reader_impl() override { ffmpeg_reader_impl::close(); }
    //! \brief 打开视频
    //! \param url 视频地址，如果是本地文件，使用file://协议
    //! \note 先关闭之前打开的视频再打开此url对应的视频
    bool open(const std::string &url_) {
      if (!ffmpeg_base::open(url_)) {
        LOG_ERROR("ffmpeg_base failed");
        return false;
      }
      int ret = 0;

      input_ctx = avformat_alloc_context();
      if (!input_ctx) {
        LOG_ERROR("avformat_alloc_context failed");
        return false;
      }

      if constexpr (decode_frame) {
        input_ctx->interrupt_callback.callback = interrupt_cb;
        input_ctx->interrupt_callback.opaque = this;
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

      const AVCodec *codec = nullptr;

      /* //尝试用nvidia解码 */
      if (getenv("use_cuvid") != nullptr &&
          video_stream->codecpar->codec_id == AV_CODEC_ID_H264) {
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
              std::make_unique<cyy::algorithm::thread_safe_linear_container<
                  std::vector<std::pair<int, frame>>>>();

        } else {
          packet_buffer =
              std::make_unique<cyy::algorithm::thread_safe_linear_container<
                  std::vector<std::pair<int, std::shared_ptr<AVPacket>>>>>();
        }
        start("ffmpeg_reader_impl");
      }

      return true;
    }

    //! \brief jump to a frame
    bool seek_frame(size_t frame_seq) {
      auto it = key_frame_timestamps.find(frame_seq);
      if (it == key_frame_timestamps.end()) {
        LOG_ERROR("can't find the past key frame {} record", frame_seq);
        return false;
      }
      auto pts = it->second;

      auto res =
          av_seek_frame(input_ctx, stream_index, pts, AVSEEK_FLAG_BACKWARD);
      if (res < 0) {
        LOG_ERROR("av_seek_frame failed:{}", errno_to_str(res));
        return false;
      }
      avformat_flush(input_ctx);
      avcodec_flush_buffers(decode_ctx);
      next_frame_seq = frame_seq;
      if (frame_buffer) {
        frame_buffer->clear();
      }
      return true;
    }

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
      LOG_DEBUG("frame rate [{} {}]", res.num, res.den);
      return {{static_cast<size_t>(res.num), static_cast<size_t>(res.den)}};
    }

    //! 設置播放的幀率，用於控制next_frame的速度
    void set_play_frame_rate(const std::array<size_t, 2> &frame_rate) {
      if (play_frame_rate == frame_rate) {
        return;
      }
      play_frame_rate = frame_rate;
      next_play_time.reset();
    }

    //! \brief 关闭已经打开的视频，如果之前没调用过open，调用该函数无效果
    void close() noexcept override {
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
      if (!is_live_stream()) {
        next_frame_seq = 1;
      }
      ffmpeg_base::close();
    }

    void drop_non_key_frames() {
      frame_filters.emplace("key_frame", [](uint64_t, auto const &frame) {
        return is_key_frame(frame);
      });
    }
    void add_named_filter(std::string name,
                          std::function<bool(uint64_t)> filter) {
      frame_filters.emplace(
          name, [filter](uint64_t seq, auto const &) { return filter(seq); });
    }
    void remove_named_filter(std::string name) { frame_filters.erase(name); }

    void keep_non_key_frames() { remove_named_filter("key_frame"); }

  private:
    static int interrupt_cb(void *ctx) {
      if (reinterpret_cast<ffmpeg_reader_impl<decode_frame> *>(ctx)
              ->needs_stop()) {
        LOG_WARN("stop decode thread");
        return 1;
      }
      return 0;
    }
    static bool is_key_frame(const AVFrame &frame) {
      return ((frame.key_frame == 1) || (frame.pict_type == AV_PICTURE_TYPE_I));
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
          packet, [](AVPacket *pkg) { av_packet_free(&pkg); });
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
          packet, [](AVPacket *pkg) { av_packet_free(&pkg); });

      while (true) {
        auto ret = avcodec_receive_frame(decode_ctx, avframe);
        if (ret == 0) {
          if (can_seek()) {
            key_frame_timestamps.emplace(next_frame_seq, avframe->pts);
          }
          bool pass_filters = true;
          for (auto const &[name, filter] : frame_filters) {
            if (!filter(next_frame_seq, *avframe)) {
              pass_filters = false;
              break;
            }
          }
          if (pass_filters) {
            break;
          }
          LOG_DEBUG("ignore frame seq {}", next_frame_seq);
          next_frame_seq++;
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

      frame new_frame;
      new_frame.seq = next_frame_seq;
      next_frame_seq++;
      new_frame.is_key = is_key_frame(*avframe);

      uint8_t *dst_data[4]{};
      int dst_linesize[4]{};

      new_frame.content = cv::Mat(video_height, video_width, CV_8UC3);
      dst_linesize[0] = static_cast<int>(new_frame.content.step[0]);

      auto ret = av_image_fill_pointers(dst_data, pix_fmt, video_height,
                                        new_frame.content.data, dst_linesize);

      if (ret <= 0) {
        LOG_ERROR("av_image_fill_pointers failed:{}", errno_to_str(ret));
        return {-1, {}};
      }

      get_sws_ctx(pix_fmt);
      if (!sws_ctx) {
        LOG_ERROR("get_sws_ctx failed");
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
      return {1, new_frame};
    }

    bool can_seek() const { return !is_live_stream(); }

    void get_sws_ctx(const enum AVPixelFormat pix_fmt) {
      auto new_sws_ctx = sws_getCachedContext(
          sws_ctx, avframe->width, avframe->height,
          static_cast<enum AVPixelFormat>(avframe->format), video_width,
          video_height, pix_fmt, SWS_BICUBIC, nullptr, nullptr, nullptr);

      if (!new_sws_ctx) {
        LOG_ERROR("sws_getContext failed");
        return;
      }
      sws_ctx = new_sws_ctx;
    }

  private:
    int stream_index{-1};
    uint64_t next_frame_seq{1};
    int video_width{-1};
    int video_height{-1};

    std::optional<std::array<size_t, 2>> play_frame_rate;
    std::optional<std::chrono::time_point<std::chrono::steady_clock>>
        next_play_time;
    AVFormatContext *input_ctx{nullptr};
    AVCodecContext *decode_ctx{nullptr};
    AVDictionary *opts{nullptr};
    AVFrame *avframe{nullptr};
    SwsContext *sws_ctx{nullptr};

    std::unordered_map<size_t, int64_t> key_frame_timestamps;

    std::unordered_map<std::string,
                       std::function<bool(uint64_t, const AVFrame &)>>
        frame_filters;
    std::unique_ptr<cyy::algorithm::thread_safe_linear_container<
        std::vector<std::pair<int, frame>>>>
        frame_buffer;
    std::unique_ptr<cyy::algorithm::thread_safe_linear_container<
        std::vector<std::pair<int, std::shared_ptr<AVPacket>>>>>
        packet_buffer;
  };
} // namespace cyy::naive_lib::video
