// This file is part of CAF, the C++ Actor Framework. See the file LICENSE in
// the main distribution directory for license terms and copyright or visit
// https://github.com/actor-framework/actor-framework/blob/master/LICENSE.

#include "caf/detail/pretty_type_name.hpp"

#include "caf/config.hpp"

#if defined(CAF_LINUX) || defined(CAF_MACOS) || defined(CAF_NET_BSD)
#  define CAF_HAS_CXX_ABI
#endif

#ifdef CAF_HAS_CXX_ABI
#  include <cxxabi.h>
#  include <sys/types.h>
#  include <unistd.h>
#endif

#include "caf/string_algorithms.hpp"

namespace caf::detail {

void prettify_type_name(std::string& class_name) {
  // replace_all(class_name, " ", "");
  replace_all(class_name, "::", ".");
  replace_all(class_name, "(anonymous namespace)", "ANON");
  replace_all(class_name, ".__1.", "."); // gets rid of weird Clang-lib names
  // hide CAF magic in logs
  auto strip_magic = [&](const char* prefix_begin, const char* prefix_end) {
    auto last = class_name.end();
    auto i = std::search(class_name.begin(), last, prefix_begin, prefix_end);
    auto comma_or_angle_bracket = [](char c) { return c == ',' || c == '>'; };
    auto e = std::find_if(i, last, comma_or_angle_bracket);
    if (i != e) {
      std::string substr(i + (prefix_end - prefix_begin), e);
      class_name.swap(substr);
    }
  };
  char prefix1[] = "caf.detail.embedded<";
  strip_magic(prefix1, prefix1 + (sizeof(prefix1) - 1));
  // Drop template parameters, only leaving the template class name.
  auto i = std::find(class_name.begin(), class_name.end(), '<');
  if (i != class_name.end())
    class_name.erase(i, class_name.end());
  // Finally, replace any whitespace with %20 (should never happen).
  replace_all(class_name, " ", "%20");
}

void prettify_type_name(std::string& class_name, const char* c_class_name) {
#ifdef CAF_HAS_CXX_ABI
  int stat = 0;
  std::unique_ptr<char, decltype(free)*> real_class_name{nullptr, free};
  auto tmp = abi::__cxa_demangle(c_class_name, nullptr, nullptr, &stat);
  real_class_name.reset(tmp);
  class_name = stat == 0 ? real_class_name.get() : c_class_name;
#else
  class_name = c_class_name;
#endif
  prettify_type_name(class_name);
}

std::string pretty_type_name(const std::type_info& x) {
  std::string result;
  prettify_type_name(result, x.name());
  return result;
}

} // namespace caf::detail
