/*!
 * \file mat.hpp
 *
 * \brief cv::Mat的cpu/gpu操作
 * \author cyy
 */

#pragma once

#include <filesystem>
#include <memory>
#include <optional>
#include <vector>

#include <opencv2/opencv.hpp>

namespace cyy::naive_lib::opencv {

  //! \brief cv::Mat的cpu/gpu操作
  class mat final {
  public:
    mat();

    explicit mat(const cv::Mat &cv_mat);
#ifdef HAVE_GPU_MAT
    explicit mat(const cv::cuda::GpuMat &cv_gpu_mat);
#endif

    mat(const mat &);
    mat &operator=(const mat &);

    mat(mat &&) noexcept;
    mat &operator=(mat &&) noexcept;

    ~mat();

    mat &operator+=(float scalar);
    mat &operator+=(const cv::Scalar &scalar);

    mat &operator/=(float scalar);

    mat operator()(const cv::Rect &roi) const;

    bool equal(const mat &rhs) const;

    //! \brief 使用GPU
    const mat &use_gpu(bool use) const;

    //! \brief 获取封装的cv::Mat
    const cv::Mat &get_cv_mat() const;

#ifdef HAVE_GPU_MAT
    //! \brief 获取封装的cv::cuda::GpuMat
    const cv::cuda::GpuMat &get_cv_gpu_mat() const;
#endif

    //! \brief 复制mat内容到cpu buffer
    void to_cpu_buffer(float *buf) const;

#ifdef HAVE_GPU_MAT
    //! \brief 复制mat内容到gpu buffer
    void to_gpu_buffer(float *buf) const;
#endif

    int width() const;

    int height() const;

    int channels() const;

    int type() const;

    size_t elem_size() const;

    mat clone() const;

    mat transpose() const;

    mat resize(int new_width, int new_height,
               int interpolation = cv::INTER_LINEAR) const;

    mat convert_to(int rtype, double alpha = 1, double beta = 0,
                   bool self_as_result = false);

    void cvt_color(int code);
    cv::Scalar MSSIM(mat i2) const;

    mat copy_make_border(int top, int bottom, int left, int right,
                         const ::cv::Scalar &value) const;

    std::vector<mat> split() const;

    mat flip(int flip_code, bool self_as_result = false);

    //! \brief 加载指定路径的图片
    //! \return 如果不成功，返回空，否則返回讀取到的Mat
    static std::optional<mat> load(const std::filesystem::path &image_path);
    //! \brief 从内存流中加载图片
    //! \return 如果不成功，返回空，否則返回讀取到的Mat
    static std::optional<mat> load(const void *buf, size_t size);

  private:
    class mat_impl;
    mat(mat_impl &&);

  private:
    std::unique_ptr<mat_impl> pimpl;
  };

} // namespace cyy::naive_lib::opencv
