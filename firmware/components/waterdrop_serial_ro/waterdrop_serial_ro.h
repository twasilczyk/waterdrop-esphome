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
#include "esphome/core/component.h"
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

#ifdef USE_WATERDROP_SERIAL_RO_REPORT_UNKNOWN_VALUES
namespace esphome::text_sensor {
class TextSensor;
}
#endif

namespace esphome::waterdrop_serial::ro {

#ifdef USE_WATERDROP_SERIAL_RO_REQUEST_UNKNOWN_VALUES
class DiagnosticNumber : public number::Number {
 public:
  void bind_value(uint8_t &value);

 protected:
  void control(float value) override;

 private:
  uint8_t *value_ = nullptr;
};
#else
class ConfigSwitch : public switch_::Switch, public Component {
 public:
  void setup() override;

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

#ifdef USE_WATERDROP_SERIAL_RO_REPORT_UNKNOWN_VALUES
  enum class UnknownSensor : size_t {
    C2_STATE,
    C2_UNKNOWN,
    C2_UNKNOWN_ERROR,
    MAGIC_COUNTER1C,
    MAGIC_SENSOR1,
    C5_SLOT_02_UNKNOWN7,
    MAGIC_COUNTER1D,
    C5_SLOT_04_UNKNOWN1,
    C5_SLOT_04_UNKNOWN2,
    C5_SLOT_04_UNKNOWN6,
    C5_SLOT_04_UNKNOWN7,
    MAGIC_COUNTER1B_CF,
    MAGIC_COUNTER1B_RO,
    MAGIC_COUNTER1B_CB,
    SLOT_22_05_UNKNOWN4,
    MAGIC_COUNTER1A_CF,
    MAGIC_COUNTER1A_RO,
    MAGIC_COUNTER1A_CB,
    SLOT_22_0E_UNKNOWN1,
    SLOT_22_0E_UNKNOWN2,
    SLOT_22_0E_UNKNOWN3,
    SLOT_22_0E_UNKNOWN4,
    SLOT_22_0E_UNKNOWN5,
    SLOT_22_0E_UNKNOWN6,
    SLOT_22_0E_UNKNOWN7,
    SLOT_22_0F_UNKNOWN1,
    TEMPERATURE2,
    TEMPERATURE3,
    TEMPERATURE4,
    COUNT_,
  };
  static constexpr size_t UNKNOWN_SENSOR_SENSOR_TYPES_COUNT =
      static_cast<size_t>(UnknownSensor::COUNT_);
#endif

  WaterdropSerialRo();
  void dump_config() override;

  void set_filter_sensors(filter::Type filter, filter::Sensors sensors);
  void set_tds_sensor(sensor::Sensor *sensor);
  void set_air_temperature_sensor(sensor::Sensor *sensor);
  void set_last_filter_change_sensor(sensor::Sensor *sensor);
  void set_pump_active_sensor(binary_sensor::BinarySensor *sensor);
  void set_flushing_sensor(binary_sensor::BinarySensor *sensor);
  void set_error_sensors(
      std::array<binary_sensor::BinarySensor *, ERROR_TYPES_COUNT> sensors);
#ifdef USE_WATERDROP_SERIAL_RO_REPORT_UNKNOWN_VALUES
  void set_unknown_sensors(
      std::array<sensor::Sensor *, UNKNOWN_SENSOR_SENSOR_TYPES_COUNT> sensors);
  void set_unexpected_frame_sensor(text_sensor::TextSensor *sensor);
#endif
#ifdef USE_WATERDROP_SERIAL_RO_REQUEST_UNKNOWN_VALUES
  void set_request_unknown_numbers(
      std::array<DiagnosticNumber *, REQUEST_UNKNOWN_NUMBER_TYPES_COUNT> numbers);
#else
  void set_faucet_state_switch(ConfigSwitch *faucet_state_switch);
#endif

 protected:
  void handle_frame(const frame::Frame &frame) override;

 private:
  void send_request_message_();
  void handle_state_message_(const frame::Frame &frame);
  void handle_c5_message_(const frame::Frame &frame);
  void handle_response_message_(const frame::Frame &frame);
#ifdef USE_WATERDROP_SERIAL_RO_REPORT_UNKNOWN_VALUES
  void publish_unknown_sensor_(UnknownSensor sensor, int value);
  void publish_unexpected_frame_(const frame::Frame &frame, const char *reason);
#endif
  filter::Filter &filter_(filter::Type filter);
#ifdef USE_WATERDROP_SERIAL_RO_REQUEST_UNKNOWN_VALUES
  uint8_t request_unknown_value_(RequestUnknownNumber number) const;
#endif

  uint8_t request_slot_ = 0;
  std::array<filter::Filter, static_cast<size_t>(filter::Type::COUNT_)> filters_{};
  sensor::Sensor *tds_sensor_ = nullptr;
  sensor::Sensor *air_temperature_sensor_ = nullptr;
  sensor::Sensor *last_filter_change_sensor_ = nullptr;
  binary_sensor::BinarySensor *pump_active_sensor_ = nullptr;
  binary_sensor::BinarySensor *flushing_sensor_ = nullptr;
  std::array<binary_sensor::BinarySensor *, ERROR_TYPES_COUNT> error_sensors_{};
#ifdef USE_WATERDROP_SERIAL_RO_REPORT_UNKNOWN_VALUES
  std::array<sensor::Sensor *, UNKNOWN_SENSOR_SENSOR_TYPES_COUNT> unknown_sensors_{};
  text_sensor::TextSensor *unexpected_frame_sensor_ = nullptr;
#endif
#ifdef USE_WATERDROP_SERIAL_RO_REQUEST_UNKNOWN_VALUES
  std::array<DiagnosticNumber *, REQUEST_UNKNOWN_NUMBER_TYPES_COUNT>
      request_unknown_numbers_{};
  std::array<uint8_t, REQUEST_UNKNOWN_NUMBER_TYPES_COUNT> request_unknown_values_{
      static_cast<uint8_t>(message::Message22Slot::SLOT_01),
      0x00,
      0x09,
      static_cast<uint8_t>(message::Message22Request::FaucetState::CLOSED),
      0x00,
      0x00,
      0x00,
      0x00,
  };
#else
  ConfigSwitch *faucet_state_switch_ = nullptr;
#endif
};

}  // namespace esphome::waterdrop_serial::ro
