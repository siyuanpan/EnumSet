#pragma once
/*
Copyright 2021 Siyuan Pan <pansiyuan.cs@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

#include <array>
#include <bitset>
#include <cassert>
#include <string_view>
#include <type_traits>
#include <utility>

#define ENUM_MIN -128
#define ENUM_MAX 128

template <class E>
struct enum_range {
  static_assert(std::is_enum_v<E>, "enum_range requires enum type.");
  static constexpr int min = ENUM_MIN;
  static constexpr int max = ENUM_MAX;
  static_assert(max > min, "enum_range requires max > min.");
};

template <std::size_t N>
class static_string {
 public:
  constexpr explicit static_string(std::string_view str) noexcept
      : static_string{str, std::make_index_sequence<N>{}} {
    assert(str.size() == N);
  }

  constexpr const char* data() const noexcept { return chars_; }

  constexpr std::size_t size() const noexcept { return N; }

  constexpr operator std::string_view() const noexcept {
    return {data(), size()};
  }

 private:
  template <std::size_t... I>
  constexpr static_string(std::string_view str,
                          std::index_sequence<I...>) noexcept
      : chars_{str[I]..., '\0'} {}

  char chars_[N + 1];
};

template <>
class static_string<0> {
 public:
  constexpr explicit static_string(std::string_view) noexcept {}

  constexpr const char* data() const noexcept { return nullptr; }

  constexpr std::size_t size() const noexcept { return 0; }

  constexpr operator std::string_view() const noexcept { return {}; }
};

constexpr std::string_view pretty_name(std::string_view name) noexcept {
  for (std::size_t i = name.size(); i > 0; --i) {
    if (!((name[i - 1] >= '0' && name[i - 1] <= '9') ||
          (name[i - 1] >= 'a' && name[i - 1] <= 'z') ||
          (name[i - 1] >= 'A' && name[i - 1] <= 'Z') || (name[i - 1] == '_'))) {
      name.remove_prefix(i);
      break;
    }
  }

  if (name.size() > 0 &&
      ((name.front() >= 'a' && name.front() <= 'z') ||
       (name.front() >= 'A' && name.front() <= 'Z') || (name.front() == '_'))) {
    return name;
  }

  return {};
}

template <class E, E V>
constexpr auto n() noexcept {
  static_assert(std::is_enum_v<E>, "n requires enum type.");

#if defined(__clang__) || defined(__GNUC__)
  constexpr auto name =
      pretty_name({__PRETTY_FUNCTION__, sizeof(__PRETTY_FUNCTION__) - 2});
#elif defined(_MSC_VER)
  constexpr auto name = pretty_name({__FUNCSIG__, sizeof(__FUNCSIG__) - 17});
#endif
  return static_string<name.size()>{name};
}

template <class E, auto V>
constexpr bool is_valid() noexcept {
  static_assert(std::is_enum_v<E>, "is_valid requires enum type.");

  return n<E, static_cast<E>(V)>().size() != 0;
}

template <class E, int O, typename U = std::underlying_type_t<E>>
constexpr E value(std::size_t i) noexcept {
  static_assert(std::is_enum_v<E>, "value requires enum type.");

  return static_cast<E>(static_cast<int>(i) + O);
}

template <std::size_t N>
constexpr std::size_t values_count(const bool (&valid)[N]) noexcept {
  auto count = std::size_t{0};
  for (std::size_t i = 0; i < N; ++i) {
    if (valid[i]) ++count;
  }
  return count;
}

template <class T, std::size_t N, std::size_t... I>
constexpr std::array<std::remove_cv_t<T>, N> to_array(
    T (&a)[N], std::index_sequence<I...>) {
  return {{a[I]...}};
}

template <class E, int Min, std::size_t... I>
constexpr auto values(std::index_sequence<I...>) noexcept {
  static_assert(std::is_enum_v<E>, "values requires enum type.");
  constexpr bool valid[sizeof...(I)] = {is_valid<E, value<E, Min>(I)>()...};
  constexpr std::size_t count = values_count(valid);

  if constexpr (count > 0) {
    E values[count] = {};
    for (std::size_t i = 0, v = 0; v < count; ++i) {
      if (valid[i]) {
        values[v++] = value<E, Min>(i);
      }
    }
    return to_array(values, std::make_index_sequence<count>{});
  } else {
    std::array<E, 0>{};
  }
}

template <class E, typename U = std::underlying_type_t<E>>
constexpr auto values() noexcept {
  static_assert(std::is_enum_v<E>, "values requires enum type.");
  constexpr auto min = enum_range<E>::min;
  constexpr auto max = enum_range<E>::max;
  constexpr auto range_size = max - min + 1;
  static_assert(range_size > 0, "enum_range requires valid size.");

  return values<E, enum_range<E>::min>(std::make_index_sequence<range_size>{});
}

template <class E>
[[nodiscard]] constexpr auto EnumCount() noexcept
    -> std::enable_if_t<std::is_enum_v<std::decay_t<E>>, std::size_t> {
  using D = std::decay_t<E>;

  return values<E>().size();
}

template <class T>
class EnumSet {
  static_assert(std::is_enum_v<T>,
                "EnumSet type must be a strongly typed enum");

 public:
  typedef typename std::underlying_type<T>::type UnderlyingType;

  EnumSet() noexcept = default;

  constexpr EnumSet(const T& val) noexcept {
    values.set(static_cast<UnderlyingType>(val));
  }

  constexpr bool operator==(EnumSet<T> other) const {
    return values == other.values;
  }

  constexpr bool operator!=(EnumSet<T> other) const {
    return !operator==(other);
  }

  EnumSet<T> operator|=(const EnumSet<T>& other) noexcept {
    values |= other.values;
    return *this;
  }

  constexpr EnumSet<T> operator|(const EnumSet<T>& val) const {
    EnumSet<T> ret(*this);
    ret |= val;

    return ret;
  }

  EnumSet<T> operator&=(const EnumSet<T>& other) noexcept {
    values &= other.values;
    return *this;
  }

  constexpr EnumSet<T> operator&(const EnumSet<T>& val) const {
    EnumSet<T> ret(*this);
    ret &= val;

    return ret;
  }

  EnumSet<T> operator^=(const EnumSet<T>& other) noexcept {
    values ^= other.values;
    return *this;
  }

  constexpr EnumSet<T> operator^(const EnumSet<T>& val) const {
    EnumSet<T> ret(*this);
    ret ^= val;

    return ret;
  }

  constexpr EnumSet<T> operator~() const {
    EnumSet<T> ret(*this);
    ret.values.flip();

    return ret;
  }

  constexpr explicit operator bool() const { return values.any(); }

  std::size_t size() const { return values.size(); }

  std::size_t count() const { return values.count(); }

 private:
  std::bitset<EnumCount<T>()> values;
};

template <class T>
std::enable_if_t<std::is_enum_v<T>, EnumSet<T>> operator|(const T& lhs,
                                                          const T& rhs) {
  EnumSet<T> ret;
  ret |= lhs;
  ret |= rhs;

  return ret;
}
