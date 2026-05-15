#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#include "esphome/components/switch/switch.h"
#pragma GCC diagnostic pop
#include "esphome/components/waterdrop_serial/frame.h"
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

namespace esphome::waterdrop_serial::ro {

class DiagnosticSwitch : public switch_::Switch {
 protected:
  void write_state(bool new_state) override;
};

class WaterdropSerialRo : public WaterdropSerial {
 public:
  static constexpr size_t ERROR_TYPES_COUNT = 4;

  WaterdropSerialRo();
  void dump_config() override;

  void set_filter_sensors(filter::Type filter, filter::Sensors sensors);
  void set_tds_sensor(sensor::Sensor *sensor);
  void set_booting_sensor(binary_sensor::BinarySensor *sensor);
  void set_flushing_sensor(binary_sensor::BinarySensor *sensor);
  void set_error_sensors(
      std::array<binary_sensor::BinarySensor *, ERROR_TYPES_COUNT> sensors);
  void set_faucet_state_switch(DiagnosticSwitch *faucet_state_switch);

 protected:
  void handle_frame(const frame::Frame &frame) override;

 private:
  void send_request_message_();
  void handle_state_message_(const frame::Frame &frame);
  void handle_response_message_(const frame::Frame &frame);
  filter::Filter &filter_(filter::Type filter);

  uint8_t request_slot_ = 0;
  std::array<filter::Filter, static_cast<size_t>(filter::Type::COUNT_)> filters_{};
  sensor::Sensor *tds_sensor_ = nullptr;
  binary_sensor::BinarySensor *booting_sensor_ = nullptr;
  binary_sensor::BinarySensor *flushing_sensor_ = nullptr;
  std::array<binary_sensor::BinarySensor *, ERROR_TYPES_COUNT> error_sensors_{};
  DiagnosticSwitch *faucet_state_switch_ = nullptr;
};

}  // namespace esphome::waterdrop_serial::ro
