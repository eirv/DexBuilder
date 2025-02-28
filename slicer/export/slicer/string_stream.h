#pragma once

#include <array>
#include <cstdio>
#include <string>
#include <string_view>

namespace slicer {

static constexpr char endl = '\n';

class StringStream {
 public:
  StringStream() = default;

  inline auto& operator<<(int32_t x) {
    s += std::to_string(x);
    return *this;
  }

  inline auto& operator<<(uint32_t x) {
    s += std::to_string(x);
    return *this;
  }

  inline auto& operator<<(int64_t x) {
    s += std::to_string(x);
    return *this;
  }

  inline auto& operator<<(uint64_t x) {
    s += std::to_string(x);
    return *this;
  }

  inline auto& operator<<(float x) {
    s += std::to_string(x);
    return *this;
  }

  inline auto& operator<<(double x) {
    s += std::to_string(x);
    return *this;
  }

  inline auto& operator<<(const void* x) {
    s += std::to_string(reinterpret_cast<uintptr_t>(x));
    return *this;
  }

  inline auto& operator<<(const char* x) {
    s += x;
    return *this;
  }

  inline auto& operator<<(const std::string& x) {
    s += x;
    return *this;
  }

  inline auto& operator<<(const std::string_view& x) {
    s += x;
    return *this;
  }

  inline auto& operator<<(const StringStream& x) {
    s += x.s;
    return *this;
  }

  inline auto& str() const { return s; }

 private:
  std::string s;
};

template <typename... Args>
static inline auto FormatString(const char* fmt, Args... args) {
  std::array<char, 1024> buf;
  auto size = snprintf(buf.data(), buf.size(), fmt, args...);
  return std::string{buf.data(), static_cast<size_t>(size)};
}

}  // namespace slicer
