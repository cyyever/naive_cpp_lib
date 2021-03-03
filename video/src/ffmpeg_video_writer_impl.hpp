/*!
 * \file ffmpeg_writer_impl.hpp
 *
 * \brief
 * 封装ffmpeg对视频流的寫操作，參考ffmpeg源碼目錄下的doc/examples/muxing.c文件
 * \author Yue Wu,cyy
 */
#pragma once
#include <memory>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavformat/avio.h>
#include <libavutil/imgutils.h>
#include <libavutil/pixfmt.h>
#include <libswscale/swscale.h>
}

#include "cv/mat.hpp"
#include "ffmpeg_base.hpp"
#include "log/log.hpp"

namespace cyy::naive_lib::video {

  //! \brief 封装ffmpeg对视频流的讀操作
  class ffmpeg_writer_impl : public ffmpeg_base {
  public:
    ffmpeg_writer_impl() = default;

    //! \brief 打开视频
    //! \param url 视频地址，如果是本地文件，使用file://协议
    //! \note 先关闭之前打开的视频再打开此url对应的视频
    bool open(const std::string &url, const std::string &format_name,
              int video_width, int video_height) {
      if (!ffmpeg_base::open(url)) {
        LOG_ERROR("ffmpeg_base failed");
        return false;
      }

      int ret = 0;
      ret = avformat_alloc_output_context2(&output_ctx, nullptr,
                                           format_name.c_str(), url.c_str());
      if (ret < 0) {
        LOG_ERROR("avformat_alloc_output_context2 failed:{}",
                  errno_to_str(ret));
        return false;
      }
      output_ctx->oformat->video_codec = AV_CODEC_ID_H264;

      auto codec = avcodec_find_encoder(output_ctx->oformat->video_codec);
      if (!codec) {
        LOG_ERROR("can't find encoder for url {}", url);
        return false;
      }

      output_stream = avformat_new_stream(output_ctx, nullptr);
      if (!output_stream) {
        LOG_ERROR("avformat_new_stream failed");
        return false;
      }
      encode_ctx = avcodec_alloc_context3(codec);
      if (!encode_ctx) {
        LOG_ERROR("avcodec_alloc_context3 failed");
        return false;
      }

      if (video_width <= 0 || video_height <= 0) {
        LOG_ERROR("invalid video size [{} * {}]", video_width, video_height);
        return false;
      }

      encode_ctx->width = video_width;
      encode_ctx->height = video_height;
      output_stream->time_base = av_inv_q(frame_rate);
      encode_ctx->time_base = output_stream->time_base;
      encode_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
      encode_ctx->codec_type = AVMEDIA_TYPE_VIDEO;
      encode_ctx->gop_size = 50;
      encode_ctx->max_b_frames = 1;
      encode_ctx->codec_tag = 0;
      encode_ctx->framerate = frame_rate;
      /* Some formats want stream headers to be separate. */
      if (output_ctx->oformat->flags & AVFMT_GLOBALHEADER)
        encode_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

      if (strcmp(encode_ctx->codec->name, "libx264") == 0) {
        // setting profile to main crf to 25 in order to reduce packet size
        ret = av_dict_set(&opts, "crf", "25", 0);
        if (ret != 0) {
          LOG_ERROR("av_dict_set failed :", errno_to_str(ret));
          return false;
        }
        ret = av_dict_set(&opts, "profile", "main", 0);
        if (ret != 0) {
          LOG_ERROR("av_dict_set failed :", errno_to_str(ret));
          return false;
        }
        ret = av_dict_set(&opts, "preset", "ultrafast", 0);
        if (ret != 0) {
          LOG_ERROR("av_dict_set failed :", errno_to_str(ret));
          return false;
        }
        ret = av_dict_set(&opts, "tune", "zerolatency", 0);
        if (ret != 0) {
          LOG_ERROR("av_dict_set failed :", errno_to_str(ret));
          return false;
        }
      }

      if (encode_ctx->codec->id == AV_CODEC_ID_H264) {
        // setting bitrate for H264
        ret = av_dict_set(&opts, "b", "40960000", 0);
        if (ret != 0) {
          LOG_ERROR("av_dict_set failed :", errno_to_str(ret));
          return false;
        }
        ret = av_dict_set(&opts, "stimeout", "2000000", 0);
        if (ret != 0) {
          LOG_ERROR("av_dict_set failed :", errno_to_str(ret));
          return false;
        }
      }

      ret = avcodec_open2(encode_ctx, nullptr, &opts);
      if (ret < 0) {
        LOG_ERROR("avcodec_open2 failed:{}", errno_to_str(ret));
        return false;
      }

      /* open the output file, if needed */
      if (!(output_ctx->oformat->flags & AVFMT_NOFILE)) {
        ret = avio_open(&output_ctx->pb, url.c_str(), AVIO_FLAG_WRITE);
        if (ret < 0) {
          LOG_ERROR("avio_open failed:{}", errno_to_str(ret));
          return false;
        }
      }

      /* copy the stream parameters to the muxer */
      ret =
          avcodec_parameters_from_context(output_stream->codecpar, encode_ctx);
      if (ret < 0) {
        LOG_ERROR("avcodec_parameters_from_context failed:{}",
                  errno_to_str(ret));
        return false;
      }

      /* Write the stream header, if any. */
      ret = avformat_write_header(output_ctx, nullptr);
      if (ret < 0) {
        LOG_ERROR("avformat_write_header failed:{}", errno_to_str(ret));
        return false;
      }

      avframe = av_frame_alloc();
      if (!avframe) {
        LOG_ERROR("av_frame_alloc failed");
        return false;
      }

      avframe->width = encode_ctx->width;
      avframe->height = encode_ctx->height;
      avframe->format = encode_ctx->pix_fmt;

      /* allocate the buffers for the frame data */
      ret = av_frame_get_buffer(avframe, 32);
      if (ret < 0) {
        LOG_ERROR("av_frame_get_buffer failed:{}", errno_to_str(ret));
        return false;
      }
      packet = av_packet_alloc();
      if (!packet) {
        LOG_ERROR("av_packet_alloc failed");
        return false;
      }
      opened = true;
      return true;
    }

    //! \brief 寫入一幀
    bool write_frame(const cv::Mat &frame_mat) {
      if (!has_open()) {
        LOG_ERROR("writer is not opened");
        return false;
      }

      const enum AVPixelFormat pix_fmt { AV_PIX_FMT_BGR24 };
      uint8_t *src_data[4]{};
      int src_linesize[4]{};
      int ret = 0;
      cv::Mat resized_mat;
      if (frame_mat.type() == CV_8UC3) {
        src_linesize[0] = static_cast<int>(frame_mat.step[0]);
        ret = av_image_fill_pointers(src_data, pix_fmt, frame_mat.rows,
                                     frame_mat.data, src_linesize);
      } else {
        resized_mat = ::cyy::naive_lib::opencv::mat(frame_mat)
                          .convert_to(CV_8UC3)
                          .get_cv_mat();
        src_linesize[0] = static_cast<int>(resized_mat.step[0]);
        ret = av_image_fill_pointers(src_data, pix_fmt, resized_mat.rows,
                                     resized_mat.data, src_linesize);
      }
      if (ret <= 0) {
        LOG_ERROR("av_image_fill_pointers failed:{}", errno_to_str(ret));
        return false;
      }

      sws_ctx = sws_getCachedContext(sws_ctx, frame_mat.cols, frame_mat.rows,
                                     pix_fmt, encode_ctx->width,
                                     encode_ctx->height, encode_ctx->pix_fmt,
                                     SWS_BICUBIC, nullptr, nullptr, nullptr);
      if (!sws_ctx) {
        LOG_ERROR("sws_getCachedContext failed");
        return false;
      }
      ret = sws_scale(sws_ctx, src_data, src_linesize, 0, frame_mat.rows,
                      avframe->data, avframe->linesize);
      if (ret <= 0) {
        if (ret < 0) {
          LOG_ERROR("sws_scale failed:{}", errno_to_str(ret));
        } else {
          LOG_ERROR("sws_scale failed");
        }
        return false;
      }

      avframe->pts = next_pts;
      next_pts++;

      ret = avcodec_send_frame(encode_ctx, avframe);
      if (ret != 0) {
        LOG_ERROR("avcodec_send_frame failed:{}", errno_to_str(ret));

        return false;
      }

      while (true) {
        av_packet_unref(packet);
        ret = avcodec_receive_packet(encode_ctx, packet);
        if (ret == AVERROR(EAGAIN)) {
          break;
        } else if (ret != 0) {
          LOG_ERROR("avcodec_receive_packet failed:{}", errno_to_str(ret));
          return false;
        }

        av_packet_rescale_ts(packet, encode_ctx->time_base,
                             output_stream->time_base);
        packet->stream_index = output_stream->index;
        ret = av_write_frame(output_ctx, packet);
        if (ret < 0) {
          LOG_ERROR("av_write_frame failed:{}", errno_to_str(ret));
          return false;
        }
      }
      return true;
    }

    //! \brief 关闭已经打开的视频，如果之前没调用过open，调用该函数无效果
    void close() override {
      if (output_ctx) {
        auto ret = av_write_trailer(output_ctx);
        if (ret != 0) {
          LOG_ERROR("av_write_trailer failed:{}", errno_to_str(ret));
        }
      }

      if (packet) {
        av_packet_free(&packet);
        packet = nullptr;
      }

      if (sws_ctx) {
        sws_freeContext(sws_ctx);
        sws_ctx = nullptr;
      }

      if (avframe) {
        av_frame_free(&avframe);
        avframe = nullptr;
      }

      if (encode_ctx) {
        avcodec_free_context(&encode_ctx);
        encode_ctx = nullptr;
      }
      if (output_ctx) {
        if (!(output_ctx->oformat->flags & AVFMT_NOFILE)) {
          avio_closep(&output_ctx->pb);
        }
        avformat_free_context(output_ctx);
        output_ctx = nullptr;
      }
      if (opts) {
        av_dict_free(&opts);
        opts = nullptr;
      }
      ffmpeg_base::close();
    }

  protected:
    //! \brief 寫入一幀
    bool write_package(AVPacket &pkg) {
      if (!has_open()) {
        LOG_ERROR("writer is not opened");
        return false;
      }

      av_packet_rescale_ts(&pkg, encode_ctx->time_base,
                           output_stream->time_base);
      pkg.stream_index = output_stream->index;
      auto ret = av_write_frame(output_ctx, &pkg);
      if (ret < 0) {
        LOG_ERROR("av_write_frame failed:{}", errno_to_str(ret));
        return false;
      }
      return true;
    }

  private:
    AVRational frame_rate{25, 1};
    AVFormatContext *output_ctx{nullptr};
    AVStream *output_stream{nullptr};
    AVCodecContext *encode_ctx{nullptr};
    AVDictionary *opts{nullptr};
    SwsContext *sws_ctx{nullptr};
    AVFrame *avframe{nullptr};
    AVPacket *packet{nullptr};
    int64_t next_pts{};
  };
} // namespace cyy::naive_lib::video
