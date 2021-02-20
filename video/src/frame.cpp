#include "frame.hpp"

#include "cv/mat.hpp"
namespace cyy::naive_lib::video {
  bool frame::operator==(const frame &rhs) const {
    return seq == rhs.seq && is_key == rhs.is_key &&
           cyy::naive_lib::opencv::mat(content) ==
               cyy::naive_lib::opencv::mat(rhs.content);
  }
} // namespace cyy::naive_lib::video
