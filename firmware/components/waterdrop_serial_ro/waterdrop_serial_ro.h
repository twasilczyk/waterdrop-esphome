#pragma once

#include "esphome/core/defines.h"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#ifdef USE_WATERDROP_SERIAL_RO_REQUEST_UNKNOWN_VALUES
#include "esphome/components/number/number.h"
#else
#include "esphome/components/switch/switch.h"
#endif
#pragma GCC diagnostic pop
#include "esphome/components/waterdrop_serial/frame.h"
#ifdef USE_WATERDROP_SERIAL_RO_REQUEST_UNKNOWN_VALUES
#include "esphome/components/waterdrop_serial/message.h"
#endif
#include "esphome/components/waterdrop_serial/waterdrop_serial.h"
#include "esphome/components/waterdrop_serial_ro/filter.h"

#include <array>
#include <cstddef>
#include <cstdint>

namespace esphome::binary_sensor {
class BinarySensor;
}

namespace esphome::sensor {
class Sensor;
}

namespace esphome::text_sensor {
class TextSensor;
}

namespace esphome::waterdrop_serial::ro {

#ifdef USE_WATERDROP_SERIAL_RO_REQUEST_UNKNOWN_VALUES
class DiagnosticNumber : public number::Number {
 public:
  void set_initial_value(uint8_t value);
  uint8_t value() const { return value_; }

 protected:
  void control(float value) override;

 private:
  uint8_t value_ = 0;
};
#else
class DiagnosticSwitch : public switch_::Switch {
 protected:
  void write_state(bool new_state) override;
};
#endif

class WaterdropSerialRo : public WaterdropSerial {
 public:
  static constexpr size_t ERROR_TYPES_COUNT = 4;

#ifdef USE_WATERDROP_SERIAL_RO_REQUEST_UNKNOWN_VALUES
  static constexpr size_t REQUEST_UNKNOWN_NUMBER_TYPES_COUNT = 8;

  enum class RequestUnknownNumber : size_t {
    SLOT_PICK,
    UNKNOWN1,
    UNKNOWN2,
    FAUCET_STATE,
    UNKNOWN3,
    UNKNOWN4,
    UNKNOWN5,
    UNKNOWN6,
    COUNT_,
  };
  static_assert(REQUEST_UNKNOWN_NUMBER_TYPES_COUNT ==
                static_cast<size_t>(RequestUnknownNumber::COUNT_));
#endif

  enum class RawByteSensor : size_t {
    C2_STATE,
    C2_UNKNOWN,
    C2_UNKNOWN_ERROR,
    C5_SLOT_01_UNKNOWN7,
    C5_SLOT_02_UNKNOWN4,
    C5_SLOT_02_UNKNOWN7,
    C5_SLOT_03_UNKNOWN2,
    C5_SLOT_03_UNKNOWN6,
    C5_SLOT_04_UNKNOWN1,
    C5_SLOT_04_UNKNOWN2,
    C5_SLOT_04_UNKNOWN6,
    C5_SLOT_04_UNKNOWN7,
    C5_SLOT_05_UNKNOWN3,
    C5_SLOT_05_UNKNOWN4,
    SLOT_22_01_UNKNOWN2,
    SLOT_22_01_UNKNOWN4,
    SLOT_22_01_UNKNOWN6,
    SLOT_22_03_UNKNOWN6,
    SLOT_22_0D_UNKNOWN2,
    SLOT_22_0D_UNKNOWN4,
    SLOT_22_0D_UNKNOWN6,
    SLOT_22_0E_UNKNOWN1,
    SLOT_22_0E_UNKNOWN2,
    SLOT_22_0E_UNKNOWN3,
    SLOT_22_0E_UNKNOWN4,
    SLOT_22_0E_UNKNOWN5,
    SLOT_22_0E_UNKNOWN6,
    SLOT_22_0E_UNKNOWN7,
    SLOT_22_0F_UNKNOWN1,
    COUNT_,
  };
  static constexpr size_t RAW_BYTE_SENSOR_TYPES_COUNT =
      static_cast<size_t>(RawByteSensor::COUNT_);

  WaterdropSerialRo();
  void dump_config() override;

  void set_filter_sensors(filter::Type filter, filter::Sensors sensors);
  void set_tds_sensor(sensor::Sensor *sensor);
  void set_operating_lifetime_sensor(sensor::Sensor *sensor);
  void set_booting_sensor(binary_sensor::BinarySensor *sensor);
  void set_flushing_sensor(binary_sensor::BinarySensor *sensor);
  void set_error_sensors(
      std::array<binary_sensor::BinarySensor *, ERROR_TYPES_COUNT> sensors);
  void set_raw_byte_sensors(
      std::array<sensor::Sensor *, RAW_BYTE_SENSOR_TYPES_COUNT> sensors);
  void set_unexpected_frame_sensor(text_sensor::TextSensor *sensor);
#ifdef USE_WATERDROP_SERIAL_RO_REQUEST_UNKNOWN_VALUES
  void set_request_unknown_numbers(
      std::array<DiagnosticNumber *, REQUEST_UNKNOWN_NUMBER_TYPES_COUNT> numbers);
#else
  void set_faucet_state_switch(DiagnosticSwitch *faucet_state_switch);
#endif

 protected:
  void handle_frame(const frame::Frame &frame) override;

 private:
  void send_request_message_();
  void handle_state_message_(const frame::Frame &frame);
  void handle_c5_message_(const frame::Frame &frame);
  void handle_response_message_(const frame::Frame &frame);
  void publish_raw_byte_(RawByteSensor sensor, uint8_t value);
  void publish_unexpected_frame_(const frame::Frame &frame, const char *reason);
  filter::Filter &filter_(filter::Type filter);

  uint8_t request_slot_ = 0;
#ifdef USE_WATERDROP_SERIAL_RO_REQUEST_UNKNOWN_VALUES
  message::Message22Slot request_slot_pick_ = message::Message22Slot::SLOT_01;
#endif
  std::array<filter::Filter, static_cast<size_t>(filter::Type::COUNT_)> filters_{};
  sensor::Sensor *tds_sensor_ = nullptr;
  sensor::Sensor *operating_lifetime_sensor_ = nullptr;
  binary_sensor::BinarySensor *booting_sensor_ = nullptr;
  binary_sensor::BinarySensor *flushing_sensor_ = nullptr;
  std::array<binary_sensor::BinarySensor *, ERROR_TYPES_COUNT> error_sensors_{};
  std::array<sensor::Sensor *, RAW_BYTE_SENSOR_TYPES_COUNT> raw_byte_sensors_{};
  text_sensor::TextSensor *unexpected_frame_sensor_ = nullptr;
#ifdef USE_WATERDROP_SERIAL_RO_REQUEST_UNKNOWN_VALUES
  std::array<DiagnosticNumber *, REQUEST_UNKNOWN_NUMBER_TYPES_COUNT>
      request_unknown_numbers_{};
#else
  DiagnosticSwitch *faucet_state_switch_ = nullptr;
#endif
};

}  // namespace esphome::waterdrop_serial::ro
