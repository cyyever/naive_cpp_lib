/*!
 * \file ffmpeg_audio_reader_impl.hpp
 *
 * \brief 封装ffmpeg对音频流的操作
 * \author Yue Wu,cyy
 */

#include <filesystem>
#include <memory>
#include <regex>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavformat/avio.h>
#include <libavutil/imgutils.h>
#include <libavutil/pixfmt.h>
}

#include "ffmpeg_audio_reader.hpp"
#include "ffmpeg_base.hpp"
#include "log/log.hpp"

namespace cyy::naive_lib::audio::ffmpeg {

  //! \brief 封装ffmpeg对视频流的讀操作
  class reader_impl final : public video::ffmpeg_base {
  public:
    reader_impl() = default;

    //! \brief 打开视频
    //! \param url 视频地址，如果是本地文件，使用file://协议
    //! \note 先关闭之前打开的视频再打开此url对应的视频
    bool open(const std::string &url) {
      if (!ffmpeg_base::open(url)) {
        LOG_ERROR("ffmpeg_base failed");
        return false;
      }

      int ret = 0;

      input_ctx = avformat_alloc_context();
      if (!input_ctx) {
        LOG_ERROR("avformat_alloc_context failed");
        return false;
      }

      AVInputFormat *iformat = nullptr;
      if (url_scheme == "file") {
        auto ext = std::filesystem::path(url).extension();
        if (ext == ".pcm_mulaw") {

          iformat = av_find_input_format("mulaw");

          ret = av_dict_set(&opts, "ar", "8000", 0);
          if (ret != 0) {
            LOG_ERROR("av_dict_set failed:{}", errno_to_str(ret));
            return false;
          }
        }
      }

      ret = avformat_open_input(&input_ctx, url.c_str(), iformat, &opts);
      if (ret != 0) {
        LOG_ERROR("avformat_open_input failed:{}", errno_to_str(ret));
        return false;
      }

      ret = avformat_find_stream_info(input_ctx, &opts);
      if (ret < 0) {
        LOG_ERROR("avformat_find_stream_info failed:{}", errno_to_str(ret));
        return false;
      }

      opened = true;
      return true;
    }

    bool has_open() const { return opened; }

    std::optional<std::chrono::milliseconds> get_duration() const {
      if (input_ctx->duration == AV_NOPTS_VALUE) {
        return {};
      }
      return {
          std::chrono::milliseconds(static_cast<std::chrono::milliseconds::rep>(
              input_ctx->duration * 1000.0 / AV_TIME_BASE))};
    }

    //! \brief 关闭已经打开的视频，如果之前没调用过open，调用该函数无效果
    void close() override {

      if (input_ctx) {
        avformat_close_input(&input_ctx);
        avformat_free_context(input_ctx);
        input_ctx = nullptr;
      }
      if (opts) {
        av_dict_free(&opts);
        opts = nullptr;
      }
      ffmpeg_base::close();
    }

  private:
    AVFormatContext *input_ctx{nullptr};
    AVDictionary *opts{nullptr};
  };
} // namespace cyy::naive_lib::audio::ffmpeg
