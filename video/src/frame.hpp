/*!
 * \file frame.hpp
 *
 * \brief 视频帧类
 * \author yuewu,cyy
 */

#ifndef DEEPIR_VIDEO_FRAME_HPP
#define DEEPIR_VIDEO_FRAME_HPP

#include <opencv2/opencv.hpp>

namespace deepir {
namespace video {

//! \brief 视频帧
//! \note 为了提高性能，我们希望尽量模拟ffmpeg等的帧结构，避免无谓的复制和转换
struct frame final {
  uint64_t seq{};     //!< 帧序号
  cv::Mat content;    //!< 帧内容
  bool is_key{false}; //!< 标志是否关键帧
};
} // namespace video
} // namespace deepir

#endif