#include <cxxflags.inc>
#include "waterdrop_serial_ro.h"

#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/waterdrop_serial/message.h"
#include "esphome/core/log.h"

#include <algorithm>
#include <cassert>

namespace esphome::waterdrop_serial::ro {

static const char *const TAG = "waterdrop_serial_ro";
static constexpr std::array<uint8_t, WaterdropSerialRo::ERROR_TYPES_COUNT> ERROR_MASKS{
    0x80,  // E01
    0x04,  // E02
    0x01,  // E03
    0x02,  // E04
};

void DiagnosticSwitch::write_state(bool new_state) {
  publish_state(new_state);
}

WaterdropSerialRo::WaterdropSerialRo() : WaterdropSerial(TAG, frame::Source::RO) {}

void WaterdropSerialRo::dump_config() {
  ESP_LOGCONFIG(TAG, "Waterdrop Serial RO:");
  WaterdropSerial::dump_config();
  LOG_SWITCH("  ", "Faucet State", faucet_state_switch_);
}

void WaterdropSerialRo::set_filter_sensors(filter::Type filter, filter::Sensors sensors) {
  filter_(filter).set_sensors(sensors);
}

void WaterdropSerialRo::set_tds_sensor(sensor::Sensor *sensor) {
  tds_sensor_ = sensor;
}

void WaterdropSerialRo::set_booting_sensor(binary_sensor::BinarySensor *sensor) {
  booting_sensor_ = sensor;
}

void WaterdropSerialRo::set_flushing_sensor(binary_sensor::BinarySensor *sensor) {
  flushing_sensor_ = sensor;
}

void WaterdropSerialRo::set_error_sensors(
    std::array<binary_sensor::BinarySensor *, ERROR_TYPES_COUNT> sensors) {
  assert(std::all_of(sensors.begin(), sensors.end(), [](auto *sensor) {
    return sensor != nullptr;
  }));
  error_sensors_ = sensors;
}

void WaterdropSerialRo::set_faucet_state_switch(DiagnosticSwitch *faucet_state_switch) {
  assert(faucet_state_switch != nullptr);
  faucet_state_switch_ = faucet_state_switch;
}

void WaterdropSerialRo::handle_frame(const frame::Frame &frame) {
  ESP_LOGD(TAG, "RX %s", frame.to_string().c_str());

  if (frame.command == frame::Command::COMMAND_C2) {
    handle_state_message_(frame);
    send_request_message_();
  } else if (frame.command == frame::Command::COMMAND_22) {
    handle_response_message_(frame);
  } else if (frame.command == frame::Command::COMMAND_C5) {
    // None of C5 slots are figured out yet.
  } else {
    ESP_LOGW(TAG, "Unhandled command 0x%02X", static_cast<uint8_t>(frame.command));
  }
}

void WaterdropSerialRo::handle_state_message_(const frame::Frame &frame) {
  const auto &message = frame.as<message::MessageC2>();
  if (booting_sensor_ != nullptr) {
    booting_sensor_->publish_state(message.state == 0xFF);
  }

  if (flushing_sensor_ != nullptr) {
    const bool is_flushing = message.state == 0xF3 || message.state == 0xF5
        || message.state == 0xF6;
    flushing_sensor_->publish_state(is_flushing);
  }

  for (size_t i = 0; i < error_sensors_.size(); i++) {
    if (error_sensors_[i] != nullptr) {
      error_sensors_[i]->publish_state((message.error & ERROR_MASKS[i]) != 0);
    }
  }
}

void WaterdropSerialRo::handle_response_message_(const frame::Frame &frame) {
  const auto &response = frame.as<message::Message22Response>();
  switch (response.tag()) {
    case message::Message22Slot::SLOT_0F: {
      const auto &slot = response.get<message::Message22Slot0F>();
      filter_(filter::Type::CF).set_total_life(slot.filter_total_life_cf);
      filter_(filter::Type::RO).set_total_life(slot.filter_total_life_ro);
      filter_(filter::Type::CB).set_total_life(slot.filter_total_life_cb);
      break;
    }
    case message::Message22Slot::SLOT_02: {
      const auto &slot = response.get<message::Message22Slot02>();
      filter_(filter::Type::CF).set_used_life(slot.filter_used_life_cf);
      filter_(filter::Type::RO).set_used_life(slot.filter_used_life_ro);
      filter_(filter::Type::CB).set_used_life(slot.filter_used_life_cb);
      break;
    }
    case message::Message22Slot::SLOT_03: {
      const auto &slot = response.get<message::Message22Slot03>();
      if (tds_sensor_ != nullptr) {
        tds_sensor_->publish_state(slot.tds);
      }
      break;
    }
    default:
      break;
  }
}

filter::Filter &WaterdropSerialRo::filter_(filter::Type filter) {
  const auto index = static_cast<size_t>(filter);
  assert(index < filters_.size());
  return filters_[index];
}

void WaterdropSerialRo::send_request_message_() {
  static constexpr std::array<message::Message22Slot, 6> request_slots{
      message::Message22Slot::SLOT_0D, message::Message22Slot::SLOT_01, message::Message22Slot::SLOT_0E,
      message::Message22Slot::SLOT_0F, message::Message22Slot::SLOT_03, message::Message22Slot::SLOT_02
  };
  auto slot = request_slots[request_slot_++];
  request_slot_ %= request_slots.size();

  const auto faucet_state =
      faucet_state_switch_ != nullptr && faucet_state_switch_->state
          ? message::Message22Request::FaucetState::OPEN
          : message::Message22Request::FaucetState::CLOSED;

  send_message(message::Message22Request{
    .slot = slot,
    .faucetState = faucet_state,
  });
}

}  // namespace esphome::waterdrop_serial::ro
