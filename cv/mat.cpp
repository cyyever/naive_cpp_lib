/*!
 * \file mat.cpp
 *
 * \brief cv::Mat的cpu/gpu操作
 * \author cyy
 */
#include <array>
#include <cassert>
#include <cstring>
#include <iostream>
#include <memory>
#include <stdexcept>

#ifdef HAVE_GPU_MAT
#include <cuda_buddy/pool.hpp>
#include <cuda_runtime.h>
#include <opencv2/core/core_c.h>
#include <opencv2/core/cuda.hpp>
#include <opencv2/core/cuda/common.hpp>
#include <opencv2/cudaarithm.hpp>
#include <opencv2/cudaimgproc.hpp>
#include <opencv2/cudawarping.hpp>
#include <opencv2/cudev/common.hpp>
#include <shared_mutex>

#include "hardware/cuda.hpp"
#include "hardware/hardware.hpp"
#endif
#include "log/log.hpp"
#include "util/file.hpp"

#include "mat.hpp"

#ifdef HAVE_GPU_MAT
namespace {
  //! \brief cv::cuda::GpuMat的分配器
  class device_allocator final : public cv::cuda::GpuMat::Allocator {
  public:
    device_allocator() = default;

    bool allocate(cv::cuda::GpuMat *mat, int rows, int cols,
                  size_t elemSize) override {

      auto texture_pitch_alignment = get_texture_pitch_alignment();

      size_t step = elemSize * cols + texture_pitch_alignment - 1;
      step -= step % texture_pitch_alignment;
      mat->data =
          static_cast<decltype(mat->data)>(get_pool().alloc(step * rows, 1));
      if (mat->data) {
        mat->step = step;
      } else {
        LOG_WARN("buddy alloc device {} bytes failed", step * rows);
        if (rows > 1 && cols > 1) {
          cudaSafeCall(
              cudaMallocPitch(&mat->data, &mat->step, elemSize * cols, rows));
        } else {
          // Single row or single column must be continuous
          cudaSafeCall(cudaMalloc(&mat->data, elemSize * cols * rows));
          mat->step = elemSize * cols;
        }
      }
      mat->refcount = static_cast<int *>(cv::fastMalloc(sizeof(int)));
      return true;
    }
    void free(cv::cuda::GpuMat *mat) override {
      if (!get_pool().free(mat->datastart)) {
        cudaSafeCall(cudaFree(mat->datastart));
      }
      cv::fastFree(mat->refcount);
    }

  private:
    int get_device_id() {
      static thread_local int device_id = -1;
      if (device_id < 0) {
        device_id = ::cyy::naive_lib::hardware::gpu_no();
        if (device_id < 0) {
          throw std::runtime_error("no in GPU");
        }
      }
      return device_id;
    }

    cuda_buddy::pool &get_pool() {
      auto device_id = get_device_id();

      if (!pools.count(device_id)) {
        pools.emplace(device_id, device_id);
      }
      auto it = pools.find(device_id);
      return it->second;
    }

    size_t get_texture_pitch_alignment() {
      auto device_id = get_device_id();

      if (!texture_pitch_alignments.count(device_id)) {
        cudaDeviceProp props{};
        cudaSafeCall(cudaGetDeviceProperties(&props, 0));
        texture_pitch_alignments[device_id] = props.texturePitchAlignment;
      }
      return texture_pitch_alignments[device_id];
    }

  private:
    static inline thread_local std::unordered_map<int, size_t>
        texture_pitch_alignments{};
    static inline thread_local std::unordered_map<int, cuda_buddy::pool>
        pools{};
  };

  struct buddy_allocator_initer final {
    std::unique_ptr<device_allocator> device_allocator_ptr;
    bool has_nvidia_driver{cyy::naive_lib::hardware::gpu_num() != 0};
    buddy_allocator_initer() {
      if (!has_nvidia_driver) {
        return;
      }
      cuda_buddy::pool::set_device_pool_size(40);
      device_allocator_ptr = std::make_unique<device_allocator>();
      cv::cuda::setBufferPoolUsage(false);
      cv::cuda::GpuMat::setDefaultAllocator(device_allocator_ptr.get());
    }
  };

  buddy_allocator_initer initer;

} // namespace
#endif

namespace cyy::naive_lib::opencv {
  //! \brief cv::Mat的cpu/gpu操作
  class mat::mat_impl final {
  private:
    //! \brief cv::Mat的存储位置
    enum class storage_location { synced = 0, cpu, gpu };

  public:
    mat_impl() = default;

    mat_impl(const cv::Mat &cv_mat)
        : cpu_mat(cv_mat), location(storage_location::cpu) {}

#ifdef HAVE_GPU_MAT
    mat_impl(const cv::cuda::GpuMat &cv_mat)
        : can_use_gpu(true), gpu_mat(cv_mat), location(storage_location::gpu) {}
#endif

    mat_impl(const mat_impl &) = default;

    mat_impl &operator=(const mat_impl &) = default;

    mat_impl(mat_impl &&) = default;
    mat_impl &operator=(mat_impl &&) = default;

    ~mat_impl() = default;

    bool operator==(const mat_impl &rhs) const {
      download();
      rhs.download();

      return cpu_mat.type() == rhs.cpu_mat.type() &&
             cpu_mat.size() == rhs.cpu_mat.size() &&
             std::equal(cpu_mat.begin<uchar>(), cpu_mat.end<uchar>(),
                        rhs.cpu_mat.begin<uchar>());
    }

    void use_gpu(bool use) const {
      if (!use) {
        download();
      }
      can_use_gpu = use;
    }

    mat_impl &operator+=(const cv::Scalar &scalar) {
#ifdef HAVE_GPU_MAT
      upload();
      if (location != storage_location::cpu) {
        cv::cuda::add(gpu_mat, scalar, gpu_mat, cv::noArray(), -1,
                      get_thread_stream());
        location = storage_location::gpu;
      } else
#endif
      {
        cv::add(cpu_mat, scalar, cpu_mat, cv::noArray(), -1);
        location = storage_location::cpu;
      }
      return *this;
    }

    mat_impl &operator+=(float scalar) {
      return operator+=(cv::Scalar::all(scalar));
    }

    mat_impl &operator/=(float scalar) {
#ifdef HAVE_GPU_MAT
      upload();
      if (location != storage_location::cpu) {
        if (channels() == 1) {
          cv::cuda::divide(gpu_mat, scalar, gpu_mat, 1, -1,
                           get_thread_stream());
        } else {
          cv::cuda::divide(gpu_mat, cv::Scalar(scalar, scalar, scalar), gpu_mat,
                           1, -1, get_thread_stream());
        }
        location = storage_location::gpu;
      } else
#endif
      {
        if (channels() == 1) {
          cv::divide(cpu_mat, scalar, cpu_mat, 1, -1);
        } else {
          cv::divide(cpu_mat, cv::Scalar(scalar, scalar, scalar), cpu_mat, 1,
                     -1);
        }
        location = storage_location::cpu;
      }
      return *this;
    }

    mat_impl operator()(const cv::Rect &roi) const {
#ifdef HAVE_GPU_MAT
      if (location != storage_location::cpu) {
        return {gpu_mat(roi)};
      }
#endif
      return {cpu_mat(roi)};
    }

    //! \brief 获取封装的cv::Mat
    const cv::Mat &get_cv_mat() const {
      download();
      return cpu_mat;
    }

    //! \brief 获取封装的cv::cuda::GpuMat
    const cv::cuda::GpuMat &get_cv_gpu_mat() const {
#ifdef HAVE_GPU_MAT
      upload();
      return gpu_mat;
#else
      throw std::runtime_error("no GPU support");
#endif
    }

    //! \brief 复制mat_impl内容到cpu buffer
    template <typename T> void to_cpu_buffer(T *buf) const {
      assert(elem_size() / channels() == sizeof(T));
      download();
      if (cpu_mat.isContinuous()) {
        memcpy(buf, cpu_mat.ptr<T>(0),
               static_cast<size_t>(cpu_mat.total()) * cpu_mat.elemSize());
      } else {
        for (decltype(cpu_mat.rows) i = 0; i < cpu_mat.rows; i++) {
          auto cnt = static_cast<size_t>(cpu_mat.cols) * cpu_mat.elemSize();
          memcpy(buf, cpu_mat.ptr<T>(i), cnt);
          buf += cnt;
        }
      }
    }

//! \brief 复制mat_impl内容到gpu buffer
#ifdef HAVE_GPU_MAT
    void to_gpu_buffer(float *buf) const {
      assert(elem_size() / channels() == sizeof(float));
      upload();
      if (location == storage_location::cpu) {
        throw std::runtime_error("no GPU support");
      }

      // wait for current compution
      cudaSafeCall(cudaStreamSynchronize(
          cv::cuda::StreamAccessor::getStream(get_thread_stream())));

      //参考opencv-3.2.0/modules/core/src/cuda/gpu_mat.cu
      // cv::cuda::GpuMat::copyTo
      cudaSafeCall(cudaMemcpy2DAsync(
          buf, gpu_mat.cols * gpu_mat.elemSize(), gpu_mat.data, gpu_mat.step,
          gpu_mat.cols * gpu_mat.elemSize(), gpu_mat.rows,
          cudaMemcpyDeviceToDevice,
          cv::cuda::StreamAccessor::getStream(get_thread_stream())));
      cudaSafeCall(cudaStreamSynchronize(
          cv::cuda::StreamAccessor::getStream(get_thread_stream())));
    }
#endif

    int width() const {
      return
#ifdef HAVE_GPU_MAT
          location == storage_location::gpu ? gpu_mat.cols :
#endif
                                            cpu_mat.cols;
    }

    int height() const {
      return
#ifdef HAVE_GPU_MAT
          location == storage_location::gpu ? gpu_mat.rows :
#endif
                                            cpu_mat.rows;
    }

    int channels() const {
      return
#ifdef HAVE_GPU_MAT
          location == storage_location::gpu ? gpu_mat.channels() :

#endif
                                            cpu_mat.channels();
    }

    int type() const {
      return
#ifdef HAVE_GPU_MAT
          location == storage_location::gpu ? gpu_mat.type() :

#endif
                                            cpu_mat.type();
    }

    size_t elem_size() const {
      return
#ifdef HAVE_GPU_MAT
          location == storage_location::gpu ? gpu_mat.elemSize() :

#endif
                                            cpu_mat.elemSize();
    }

    mat_impl transpose() const {
      /*
#ifdef HAVE_GPU_MAT
      upload();
      if (location != storage_location::cpu) {
        cv::cuda::GpuMat tmp;
        cv::cuda::transpose(gpu_mat, tmp, get_thread_stream());
        return {tmp};
      }
#endif
*/
      cv::Mat tmp;
      cv::transpose(cpu_mat, tmp);
      return {tmp};
    }

    mat_impl resize(int new_width, int new_height, int interpolation) const {
      //由於gpu的resize實現和cpu的不一致,我們目前只使用cpu實現
      /*
  #ifdef HAVE_GPU_MAT
      upload();
      if (location != storage_location::cpu) {
        cv::cuda::GpuMat tmp;
        cv::cuda::resize(gpu_mat, tmp, cv::Size(new_width, new_height), 0, 0,
                         interpolation,get_thread_stream());
        return {tmp};
      }
  #endif
      */
      {
        download();
        cv::Mat tmp;
        cv::resize(cpu_mat, tmp, cv::Size(new_width, new_height), 0, 0,
                   interpolation);
        mat_impl tmp_impl(tmp);
        tmp_impl.can_use_gpu = this->can_use_gpu;
        return tmp_impl;
      }
    }

    mat_impl copy_make_border(int top, int bottom, int left, int right,
                              const ::cv::Scalar &value) const {
#ifdef HAVE_GPU_MAT
      upload();
      if (location != storage_location::cpu) {
        cv::cuda::GpuMat tmp(gpu_mat.rows + top + bottom,
                             gpu_mat.cols + left + right, gpu_mat.type());
        cv::cuda::copyMakeBorder(gpu_mat, tmp, top, bottom, left, right,
                                 cv::BORDER_CONSTANT, value,
                                 get_thread_stream());
        return {tmp};
      }
#endif
      cv::Mat tmp(cpu_mat.rows + top + bottom, cpu_mat.cols + left + right,
                  cpu_mat.type());
      copyMakeBorder(cpu_mat, tmp, top, bottom, left, right,
                     cv::BORDER_CONSTANT, value);
      return {tmp};
    }

    mat_impl clone() const {
#ifdef HAVE_GPU_MAT
      upload();
      if (location != storage_location::cpu) {
        return {gpu_mat.clone()};
      }
#endif
      return {cpu_mat.clone()};
    }

    mat_impl convert_to(int rtype, double alpha = 1, double beta = 0) const {
#ifdef HAVE_GPU_MAT
      upload();
      if (location != storage_location::cpu) {
        cv::cuda::GpuMat tmp;
        gpu_mat.convertTo(tmp, rtype, alpha, beta, get_thread_stream());
        return {tmp};
      }
#endif
      cv::Mat tmp;
      cpu_mat.convertTo(tmp, rtype, alpha, beta);
      return {tmp};
    }

    void cvt_color(int code) {
#ifdef HAVE_GPU_MAT
      if (location != storage_location::cpu) {
        cv::cuda::cvtColor(gpu_mat, gpu_mat, code, 0, get_thread_stream());
        location = storage_location::gpu;
        return;
      }
#endif
      cv::cvtColor(cpu_mat, cpu_mat, code);
      location = storage_location::cpu;
      return;
    }

    std::vector<mat_impl> split() const {
      std::vector<mat_impl> res;

#ifdef HAVE_GPU_MAT
      upload();
      if (location != storage_location::cpu) {
        std::vector<cv::cuda::GpuMat> channels;
        cv::cuda::split(gpu_mat, channels, get_thread_stream());
        for (auto &channel : channels) {
          res.emplace_back(channel);
        }
      } else
#endif
      {
        std::vector<cv::Mat> channels;
        cv::split(cpu_mat, channels);

        for (auto &channel : channels) {
          res.emplace_back(channel);
        }
      }
      return res;
    }

    mat_impl flip(int flip_code) const {
#ifdef HAVE_GPU_MAT
      upload();
      if (location != storage_location::cpu) {
        cv::cuda::GpuMat tmp;
        cv::cuda::flip(gpu_mat, tmp, flip_code, get_thread_stream());
        return {tmp};
      }
#endif
      cv::Mat tmp;
      cv::flip(cpu_mat, tmp, flip_code);
      return {tmp};
    }

  private:
    //! \brief 同步mat_impl到GPU
    void upload() const {
      if (!can_use_gpu) {
        download();
        location = storage_location::cpu;
        return;
      }

      if (location != storage_location::cpu) {
        return;
      }

#ifdef HAVE_GPU_MAT
      if (initer.has_nvidia_driver) {
        gpu_mat.upload(cpu_mat, get_host_to_device_stream());
        cudaSafeCall(cudaStreamSynchronize(
            cv::cuda::StreamAccessor::getStream(get_host_to_device_stream())));
        location = storage_location::synced;
      }
#endif
    }

    //! \brief 同步mat_impl到CPU
    void download() const {
#ifdef HAVE_GPU_MAT
      if (location == storage_location::gpu) {
        cudaSafeCall(cudaStreamSynchronize(
            cv::cuda::StreamAccessor::getStream(get_thread_stream())));
        gpu_mat.download(cpu_mat, get_device_to_host_stream());
        cudaSafeCall(cudaStreamSynchronize(
            cv::cuda::StreamAccessor::getStream(get_device_to_host_stream())));
        location = storage_location::synced;
      }
#endif
    }

#ifdef HAVE_GPU_MAT
    static cv::cuda::Stream &get_thread_stream() {
      static std::shared_ptr<cv::cuda::Stream> thread_stream =
          std::make_shared<cv::cuda::Stream>(
              cv::cuda::StreamAccessor::wrapStream(cudaStreamPerThread));
      return *thread_stream;
    }

    static cv::cuda::Stream &get_host_to_device_stream() {
      static std::shared_ptr<cv::cuda::Stream> to_device_stream =
          std::make_shared<cv::cuda::Stream>(
              cv::cuda::StreamAccessor::wrapStream(
                  cyy::naive_lib::hardware::cuda::get_copy_to_device_stream()
                      .value()));
      return *to_device_stream;
    }

    static cv::cuda::Stream &get_device_to_host_stream() {
      static std::shared_ptr<cv::cuda::Stream> to_host_stream =
          std::make_shared<cv::cuda::Stream>(
              cv::cuda::StreamAccessor::wrapStream(
                  cyy::naive_lib::hardware::cuda::get_copy_to_host_stream()
                      .value()));
      return *to_host_stream;
    }
#endif

  private:
    mutable cv::Mat cpu_mat{};
    mutable bool can_use_gpu{false};
#ifdef HAVE_GPU_MAT
    mutable cv::cuda::GpuMat gpu_mat{};
    //为了在不支持CUDA的机器上运行，我们必须延迟初始化stream，构造构造函数会抛出异常
    // mutable std::shared_ptr<cv::cuda::Stream> stream;
#endif
    mutable storage_location location{storage_location::synced};
  };

  mat::mat() : pimpl{std::make_unique<mat_impl>()} {}
  mat::mat(const cv::Mat &cv_mat) : pimpl{std::make_unique<mat_impl>(cv_mat)} {}
#ifdef HAVE_GPU_MAT
  mat::mat(const cv::cuda::GpuMat &cv_gpu_mat)
      : pimpl{std::make_unique<mat_impl>(cv_gpu_mat)} {}
#endif

  mat::mat(mat_impl &&tmp_impl) : mat() { *pimpl = tmp_impl; }

  mat::mat(const mat &rhs) : mat() { (*this) = rhs; }
  mat &mat::operator=(const mat &rhs) {
    if (this != &rhs) {
      *pimpl = *(rhs.pimpl);
    }
    return *this;
  }

  mat::mat(mat &&) noexcept = default;

  mat &mat::operator=(mat &&) noexcept = default;

  mat::~mat() = default;

  mat &mat::operator+=(float scalar) {
    (*pimpl) += scalar;
    return *this;
  }

  mat &mat::operator+=(const cv::Scalar &scalar) {
    (*pimpl) += scalar;
    return *this;
  }

  mat &mat::operator/=(float scalar) {
    (*pimpl) /= scalar;
    return *this;
  }

  bool mat::operator==(const mat &rhs) const {
    return (*pimpl) == (*rhs.pimpl);
  }

  const mat &mat::use_gpu(bool use) const {
    pimpl->use_gpu(use);
    return *this;
  }

  mat mat::operator()(const cv::Rect &roi) const { return (*pimpl)(roi); }

  const cv::Mat &mat::get_cv_mat() const { return pimpl->get_cv_mat(); }

#ifdef HAVE_GPU_MAT
  const cv::cuda::GpuMat &mat::get_cv_gpu_mat() const {
    return pimpl->get_cv_gpu_mat();
  }
#endif

  void mat::to_cpu_buffer(float *buf) const {
    pimpl->to_cpu_buffer<float>(buf);
  }

#ifdef HAVE_GPU_MAT
  void mat::to_gpu_buffer(float *buf) const { pimpl->to_gpu_buffer(buf); }
#endif

  int mat::width() const { return pimpl->width(); }

  int mat::height() const { return pimpl->height(); }

  int mat::channels() const { return pimpl->channels(); }

  int mat::type() const { return pimpl->type(); }

  size_t mat::elem_size() const { return pimpl->elem_size(); }

  mat mat::clone() const { return pimpl->clone(); }

  mat mat::transpose() const { return pimpl->transpose(); }

  mat mat::resize(int new_width, int new_height, int interpolation) const {
    return pimpl->resize(new_width, new_height, interpolation);
  }

  mat mat::convert_to(int rtype, double alpha, double beta) const {
    return pimpl->convert_to(rtype, alpha, beta);
  }

  void mat::cvt_color(int code) { pimpl->cvt_color(code); }

  mat mat::copy_make_border(int top, int bottom, int left, int right,
                            const ::cv::Scalar &value) const {
    return pimpl->copy_make_border(top, bottom, left, right, value);
  }

  std::vector<mat> mat::split() const {
    std::vector<mat> res;
    for (auto &tmp : pimpl->split()) {
      res.push_back(mat(std::move(tmp)));
    }
    return res;
  }

  mat mat::flip(int flip_code) const { return pimpl->flip(flip_code); }

  std::optional<mat> mat::load(const std::filesystem::path &image_path) {
    auto res = ::cyy::naive_lib::io::get_file_content(image_path);
    if (!res) {
      return {};
    }
    return load(res.value().data(), res.value().size());
  }

  std::optional<mat> mat::load(const void *buf, size_t size) {
    //先尝试用opencv读取
    auto cv_mat = cv::imdecode(
        cv::_InputArray(static_cast<const unsigned char *>(buf), size),
        cv::IMREAD_ANYDEPTH | cv::IMREAD_COLOR);
    if (cv_mat.total() > 0) {
      return mat{cv_mat};
    }
    return {};
  }
} // namespace cyy::naive_lib::opencv
