#pragma once

#include "esphome/components/waterdrop_serial/frame.h"
#include "esphome/components/waterdrop_serial/waterdrop_serial.h"
#include "esphome/components/waterdrop_serial_ro/filter.h"

#include <array>
#include <cstddef>
#include <cstdint>

namespace esphome::sensor {
class Sensor;
}

namespace esphome::waterdrop_serial::ro {

class WaterdropSerialRo : public WaterdropSerial {
 public:
  WaterdropSerialRo();
  void dump_config() override;

  void set_filter_sensors(filter::Type filter, filter::Sensors sensors);
  void set_tds_sensor(sensor::Sensor *sensor);

 protected:
  void handle_frame(const frame::Frame &frame) override;

 private:
  void send_request_message_();
  void handle_response_message_(const frame::Frame &frame);
  filter::Filter &filter_(filter::Type filter);
  void publish_tds_(uint16_t tds);

  uint8_t request_slot_ = 0;
  std::array<filter::Filter, static_cast<size_t>(filter::Type::COUNT_)> filters_{};
  sensor::Sensor *tds_sensor_ = nullptr;
};

}  // namespace esphome::waterdrop_serial::ro
