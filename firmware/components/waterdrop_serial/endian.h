#pragma once

#include <cstdint>
#include <type_traits>

namespace esphome::waterdrop_serial::endian {

namespace details_ {

template<typename Derived, typename ValueType>
class arithmetic_operators {
 public:
  constexpr Derived &operator+=(ValueType other) {
    auto &self = static_cast<Derived &>(*this);
    self = self.value() + other;
    return self;
  }

  constexpr Derived &operator-=(ValueType other) {
    auto &self = static_cast<Derived &>(*this);
    self = self.value() - other;
    return self;
  }

  constexpr Derived &operator*=(ValueType other) {
    auto &self = static_cast<Derived &>(*this);
    self = self.value() * other;
    return self;
  }

  constexpr Derived &operator/=(ValueType other) {
    auto &self = static_cast<Derived &>(*this);
    self = self.value() / other;
    return self;
  }

  constexpr Derived &operator%=(ValueType other) {
    auto &self = static_cast<Derived &>(*this);
    self = self.value() % other;
    return self;
  }

  constexpr Derived &operator&=(ValueType other) {
    auto &self = static_cast<Derived &>(*this);
    self = self.value() & other;
    return self;
  }

  constexpr Derived &operator|=(ValueType other) {
    auto &self = static_cast<Derived &>(*this);
    self = self.value() | other;
    return self;
  }

  constexpr Derived &operator^=(ValueType other) {
    auto &self = static_cast<Derived &>(*this);
    self = self.value() ^ other;
    return self;
  }

  constexpr Derived &operator<<=(ValueType other) {
    auto &self = static_cast<Derived &>(*this);
    self = self.value() << other;
    return self;
  }

  constexpr Derived &operator>>=(ValueType other) {
    auto &self = static_cast<Derived &>(*this);
    self = self.value() >> other;
    return self;
  }

  constexpr Derived &operator++() {
    return static_cast<Derived &>(*this) += 1;
  }

  constexpr Derived operator++(int) {
    auto &self = static_cast<Derived &>(*this);
    auto old = self;
    ++self;
    return old;
  }

  constexpr Derived &operator--() {
    return static_cast<Derived &>(*this) -= 1;
  }

  constexpr Derived operator--(int) {
    auto &self = static_cast<Derived &>(*this);
    auto old = self;
    --self;
    return old;
  }

 private:
  constexpr ValueType value() const {
    return static_cast<const Derived &>(*this);
  }
};

}  // namespace details_

class big_uint16_t : public details_::arithmetic_operators<big_uint16_t, uint16_t> {
 public:
  constexpr big_uint16_t() = default;

  constexpr big_uint16_t(uint16_t value)
      : high_(static_cast<uint8_t>(value >> 8)),
        low_(static_cast<uint8_t>(value)) {}

  constexpr operator uint16_t() const {
    return (static_cast<uint16_t>(high_) << 8) | low_;
  }

 private:
  uint8_t high_ = 0;
  uint8_t low_ = 0;
};

static_assert(sizeof(big_uint16_t) == sizeof(uint16_t));
static_assert(alignof(big_uint16_t) == 1);
static_assert(std::is_trivially_copyable_v<big_uint16_t>);

}  // namespace esphome::waterdrop_serial::endian
