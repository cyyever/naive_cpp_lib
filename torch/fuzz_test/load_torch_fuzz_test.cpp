/*!
 * \file load_torch_fuzz_test.cpp
 *
 */

#include <sstream>
#include <torch/csrc/api/include/torch/serialize.h>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
  try {
    std::istringstream is;
    if (Size == 0) {
      is = std::istringstream("");
    } else {
      is = std::istringstream(
          std::string(reinterpret_cast<const char *>(Data), Size));
    }

    torch::Tensor value;
    torch::load(value, is);

  } catch (...) {
  }
  return 0; // Non-zero return values are reserved for future use.
}
