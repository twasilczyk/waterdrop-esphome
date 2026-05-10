#pragma once

#include <cassert>
#include <type_traits>

namespace esphome::waterdrop_serial {

template<typename... Alternatives> union untagged_variant;

template<typename First, typename... Rest>
union untagged_variant<First, Rest...> {
 private:
  struct empty_payload {};
  using RestPayload = std::conditional_t<sizeof...(Rest) == 0, empty_payload,
      untagged_variant<Rest...>>;

 public:
  First first;
  RestPayload rest;

  template<typename T, std::enable_if_t<std::is_same_v<T, First>, int> = 0>
  constexpr untagged_variant(const T &payload) : first(payload) {}

  template<typename T, std::enable_if_t<!std::is_same_v<T, First> && (sizeof...(Rest) > 0), int> = 0>
  constexpr untagged_variant(const T &payload) : rest(payload) {}

  template<typename T> T &get() {
    if constexpr (std::is_same_v<T, First>) {
      return first;
    } else if constexpr (sizeof...(Rest) > 0) {
      return rest.template get<T>();
    } else {
      static_assert(std::is_same_v<T, First>, "Type is not a packed_variant alternative");
    }
  }

  template<typename T> const T &get() const {
    if constexpr (std::is_same_v<T, First>) {
      return first;
    } else if constexpr (sizeof...(Rest) > 0) {
      return rest.template get<T>();
    } else {
      static_assert(std::is_same_v<T, First>, "Type is not a packed_variant alternative");
    }
  }
};

template<typename First, typename... Rest>
class packed_variant {
  using Tag = std::remove_cv_t<decltype(First::TAG)>;
  using Payload = untagged_variant<First, Rest...>;
  template<typename T>
  static constexpr bool is_alternative = std::is_same_v<T, First> || (false || ... || std::is_same_v<T, Rest>);

  template<typename Alternative>
  static constexpr bool has_unique_tags() {
    return true;
  }

  template<typename Alternative, typename Next, typename... Others>
  static constexpr bool has_unique_tags() {
    return Alternative::TAG != Next::TAG && ((Alternative::TAG != Others::TAG) && ...) &&
           has_unique_tags<Next, Others...>();
  }

 public:
  static_assert(std::is_enum_v<Tag>, "packed_variant tag must be an enum");
  static_assert((std::is_same_v<std::remove_cv_t<decltype(Rest::TAG)>, Tag>
                    && ...),
                "packed_variant alternative TAG has the wrong type");
  static_assert(has_unique_tags<First, Rest...>(),
                "packed_variant alternatives must have unique TAG values");
  static_assert(alignof(First) == 1 && ((alignof(Rest) == 1) && ...),
                "packed_variant alternative must have byte alignment");
  static_assert(sizeof(First) == sizeof(Payload) && ((sizeof(Rest) == sizeof(Payload)) && ...),
                "packed_variant alternatives must have the same payload size");
  static_assert(std::is_trivially_copyable_v<First> && (std::is_trivially_copyable_v<Rest> && ...),
                "packed_variant alternative must be trivially copyable");
  static_assert(alignof(Payload) == 1, "packed_variant payload must have byte alignment");

  template<typename T, std::enable_if_t<is_alternative<T>, int> = 0>
  constexpr packed_variant(const T &payload)
      : tag_(T::TAG), payload_(payload) {
    static_assert(sizeof(packed_variant) == sizeof(Tag) + sizeof(Payload),
                  "packed_variant must not contain padding");
  }

  template<typename T, std::enable_if_t<is_alternative<T>, int> = 0>
  packed_variant &operator=(const T &payload) {
    *this = packed_variant(payload);
    return *this;
  }

  constexpr Tag tag() const {
    return tag_;
  }

  template<typename T>
  constexpr bool holds() const {
    static_assert(is_alternative<T>, "Type is not a packed_variant alternative");
    return tag() == T::TAG;
  }

  template<typename T>
  T &get() {
    assert(holds<T>());
    return payload_.template get<T>();
  }

  template<typename T>
  const T &get() const {
    assert(holds<T>());
    return payload_.template get<T>();
  }

 private:
  Tag tag_;
  Payload payload_;
};

}  // namespace esphome::waterdrop_serial
