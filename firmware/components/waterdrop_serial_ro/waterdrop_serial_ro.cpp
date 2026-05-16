#include <cxxflags.inc>
#include "waterdrop_serial_ro.h"

#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/waterdrop_serial/message.h"
#include "esphome/core/log.h"

#include <algorithm>
#include <cassert>
#include <string>

namespace esphome::waterdrop_serial::ro {

static const char *const TAG = "waterdrop_serial_ro";
static constexpr std::array<uint8_t, WaterdropSerialRo::ERROR_TYPES_COUNT> ERROR_MASKS{
    0x80,  // E01
    0x04,  // E02
    0x01,  // E03
    0x02,  // E04
};
static constexpr uint8_t KNOWN_ERROR_MASK = static_cast<uint8_t>(
    ERROR_MASKS[0] | ERROR_MASKS[1] | ERROR_MASKS[2] | ERROR_MASKS[3]);
static constexpr uint8_t UNKNOWN_ERROR_MASK = static_cast<uint8_t>(~KNOWN_ERROR_MASK);

void DiagnosticSwitch::write_state(bool new_state) {
  publish_state(new_state);
}

WaterdropSerialRo::WaterdropSerialRo() : WaterdropSerial(TAG, frame::Source::RO) {}

void WaterdropSerialRo::dump_config() {
  ESP_LOGCONFIG(TAG, "Waterdrop Serial RO:");
  WaterdropSerial::dump_config();
  LOG_TEXT_SENSOR("  ", "Unexpected Frame", unexpected_frame_sensor_);
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

void WaterdropSerialRo::set_raw_byte_sensors(
    std::array<sensor::Sensor *, RAW_BYTE_SENSOR_TYPES_COUNT> sensors) {
  assert(std::all_of(sensors.begin(), sensors.end(), [](auto *sensor) {
    return sensor != nullptr;
  }));
  raw_byte_sensors_ = sensors;
}

void WaterdropSerialRo::set_unexpected_frame_sensor(text_sensor::TextSensor *sensor) {
  assert(sensor != nullptr);
  unexpected_frame_sensor_ = sensor;
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
    handle_c5_message_(frame);
  } else {
    ESP_LOGW(TAG, "Unhandled command 0x%02X", static_cast<uint8_t>(frame.command));
  }
}

void WaterdropSerialRo::handle_state_message_(const frame::Frame &frame) {
  const auto &message = frame.as<message::MessageC2>();
  publish_raw_byte_(RawByteSensor::C2_STATE, message.state);
  publish_raw_byte_(RawByteSensor::C2_UNKNOWN, message.unknown);
  publish_raw_byte_(
      RawByteSensor::C2_UNKNOWN_ERROR,
      static_cast<uint8_t>(message.error & UNKNOWN_ERROR_MASK));

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

void WaterdropSerialRo::handle_c5_message_(const frame::Frame &frame) {
  const auto &response = frame.as<message::MessageC5>();
  switch (response.tag()) {
    case message::MessageC5Slot::SLOT_01: {
      const auto &slot = response.get<message::MessageC5Slot01>();
      const auto expected = message::MessageC5Slot01{};
      if (slot.unknown1 != expected.unknown1 || slot.unknown2 != expected.unknown2 ||
          slot.unknown3 != expected.unknown3 || slot.unknown4 != expected.unknown4 ||
          slot.unknown5 != expected.unknown5 || slot.unknown6 != expected.unknown6) {
        publish_unexpected_frame_(frame, "Unexpected C5 slot 01");
      }
      publish_raw_byte_(RawByteSensor::C5_SLOT_01_UNKNOWN7, slot.unknown7);
      break;
    }
    case message::MessageC5Slot::SLOT_02: {
      const auto &slot = response.get<message::MessageC5Slot02>();
      const auto expected = message::MessageC5Slot02{};
      if (slot.unknown1 != expected.unknown1 || slot.unknown2 != expected.unknown2 ||
          slot.unknown3 != expected.unknown3 || slot.unknown5 != expected.unknown5 ||
          slot.unknown6 != expected.unknown6) {
        publish_unexpected_frame_(frame, "Unexpected C5 slot 02");
      }
      publish_raw_byte_(RawByteSensor::C5_SLOT_02_UNKNOWN4, slot.unknown4);
      publish_raw_byte_(RawByteSensor::C5_SLOT_02_UNKNOWN7, slot.unknown7);
      break;
    }
    case message::MessageC5Slot::SLOT_03: {
      const auto &slot = response.get<message::MessageC5Slot03>();
      const auto expected = message::MessageC5Slot03{};
      if (slot.unknown1 != expected.unknown1 || slot.unknown5 != expected.unknown5 ||
          slot.unknown7 != expected.unknown7) {
        publish_unexpected_frame_(frame, "Unexpected C5 slot 03");
      }
      publish_raw_byte_(RawByteSensor::C5_SLOT_03_UNKNOWN2, slot.unknown2);
      publish_raw_byte_(RawByteSensor::C5_SLOT_03_UNKNOWN3, slot.unknown3);
      publish_raw_byte_(RawByteSensor::C5_SLOT_03_UNKNOWN4, slot.unknown4);
      publish_raw_byte_(RawByteSensor::C5_SLOT_03_UNKNOWN6, slot.unknown6);
      break;
    }
    case message::MessageC5Slot::SLOT_04: {
      const auto &slot = response.get<message::MessageC5Slot04>();
      const auto expected = message::MessageC5Slot04{};
      if (slot.unknown3 != expected.unknown3 || slot.unknown4 != expected.unknown4 ||
          slot.unknown5 != expected.unknown5) {
        publish_unexpected_frame_(frame, "Unexpected C5 slot 04");
      }
      publish_raw_byte_(RawByteSensor::C5_SLOT_04_UNKNOWN1, slot.unknown1);
      publish_raw_byte_(RawByteSensor::C5_SLOT_04_UNKNOWN2, slot.unknown2);
      publish_raw_byte_(RawByteSensor::C5_SLOT_04_UNKNOWN6, slot.unknown6);
      publish_raw_byte_(RawByteSensor::C5_SLOT_04_UNKNOWN7, slot.unknown7);
      break;
    }
    case message::MessageC5Slot::SLOT_05: {
      const auto &slot = response.get<message::MessageC5Slot05>();
      const auto expected = message::MessageC5Slot05{};
      if (slot.unknown1 != expected.unknown1 || slot.unknown2 != expected.unknown2 ||
          slot.unknown5 != expected.unknown5 || slot.unknown6 != expected.unknown6 ||
          slot.unknown7 != expected.unknown7) {
        publish_unexpected_frame_(frame, "Unexpected C5 slot 05");
      }
      publish_raw_byte_(RawByteSensor::C5_SLOT_05_UNKNOWN3, slot.unknown3);
      publish_raw_byte_(RawByteSensor::C5_SLOT_05_UNKNOWN4, slot.unknown4);
      break;
    }
    default:
      publish_unexpected_frame_(frame, "Unexpected C5 slot");
      break;
  }
}

void WaterdropSerialRo::handle_response_message_(const frame::Frame &frame) {
  const auto &response = frame.as<message::Message22Response>();
  switch (response.tag()) {
    case message::Message22Slot::SLOT_01: {
      const auto &slot = response.get<message::Message22Slot01>();
      const auto expected = message::Message22Slot01{};
      if (slot.unknown1 != expected.unknown1 || slot.unknown3 != expected.unknown3 ||
          slot.unknown5 != expected.unknown5 || slot.unknown7 != expected.unknown7 ||
          slot.unknown8 != expected.unknown8) {
        publish_unexpected_frame_(frame, "Unexpected 22 slot 01");
      }
      publish_raw_byte_(RawByteSensor::SLOT_22_01_UNKNOWN2, slot.unknown2);
      publish_raw_byte_(RawByteSensor::SLOT_22_01_UNKNOWN4, slot.unknown4);
      publish_raw_byte_(RawByteSensor::SLOT_22_01_UNKNOWN6, slot.unknown6);
      break;
    }
    case message::Message22Slot::SLOT_0F: {
      const auto &slot = response.get<message::Message22Slot0F>();
      const auto expected = message::Message22Slot0F{};
      if (slot.unknown2 != expected.unknown2) {
        publish_unexpected_frame_(frame, "Unexpected 22 slot 0F");
      }
      filter_(filter::Type::CF).set_total_life(slot.filter_total_life_cf);
      filter_(filter::Type::RO).set_total_life(slot.filter_total_life_ro);
      filter_(filter::Type::CB).set_total_life(slot.filter_total_life_cb);
      publish_raw_byte_(RawByteSensor::SLOT_22_0F_UNKNOWN1, slot.unknown1);
      break;
    }
    case message::Message22Slot::SLOT_02: {
      const auto &slot = response.get<message::Message22Slot02>();
      const auto expected = message::Message22Slot02{};
      if (slot.unknown1 != expected.unknown1 || slot.unknown2 != expected.unknown2) {
        publish_unexpected_frame_(frame, "Unexpected 22 slot 02");
      }
      filter_(filter::Type::CF).set_used_life(slot.filter_used_life_cf);
      filter_(filter::Type::RO).set_used_life(slot.filter_used_life_ro);
      filter_(filter::Type::CB).set_used_life(slot.filter_used_life_cb);
      break;
    }
    case message::Message22Slot::SLOT_03: {
      const auto &slot = response.get<message::Message22Slot03>();
      const auto expected = message::Message22Slot03{};
      if (slot.unknown1 != expected.unknown1 || slot.unknown2 != expected.unknown2 ||
          slot.unknown3 != expected.unknown3 || slot.unknown4 != expected.unknown4 ||
          slot.unknown5 != expected.unknown5) {
        publish_unexpected_frame_(frame, "Unexpected 22 slot 03");
      }
      if (tds_sensor_ != nullptr) {
        tds_sensor_->publish_state(slot.tds);
      }
      publish_raw_byte_(RawByteSensor::SLOT_22_03_UNKNOWN6, slot.unknown6);
      break;
    }
    case message::Message22Slot::SLOT_0D: {
      const auto &slot = response.get<message::Message22Slot0D>();
      const auto expected = message::Message22Slot0D{};
      if (slot.unknown1 != expected.unknown1 || slot.unknown3 != expected.unknown3 ||
          slot.unknown5 != expected.unknown5 || slot.unknown7 != expected.unknown7 ||
          slot.unknown8 != expected.unknown8) {
        publish_unexpected_frame_(frame, "Unexpected 22 slot 0D");
      }
      publish_raw_byte_(RawByteSensor::SLOT_22_0D_UNKNOWN2, slot.unknown2);
      publish_raw_byte_(RawByteSensor::SLOT_22_0D_UNKNOWN4, slot.unknown4);
      publish_raw_byte_(RawByteSensor::SLOT_22_0D_UNKNOWN6, slot.unknown6);
      break;
    }
    case message::Message22Slot::SLOT_0E: {
      const auto &slot = response.get<message::Message22Slot0E>();
      const auto expected = message::Message22Slot0E{};
      if (slot.unknown8 != expected.unknown8) {
        publish_unexpected_frame_(frame, "Unexpected 22 slot 0E");
      }
      publish_raw_byte_(RawByteSensor::SLOT_22_0E_UNKNOWN1, slot.unknown1);
      publish_raw_byte_(RawByteSensor::SLOT_22_0E_UNKNOWN2, slot.unknown2);
      publish_raw_byte_(RawByteSensor::SLOT_22_0E_UNKNOWN3, slot.unknown3);
      publish_raw_byte_(RawByteSensor::SLOT_22_0E_UNKNOWN4, slot.unknown4);
      publish_raw_byte_(RawByteSensor::SLOT_22_0E_UNKNOWN5, slot.unknown5);
      publish_raw_byte_(RawByteSensor::SLOT_22_0E_UNKNOWN6, slot.unknown6);
      publish_raw_byte_(RawByteSensor::SLOT_22_0E_UNKNOWN7, slot.unknown7);
      break;
    }
    default:
      publish_unexpected_frame_(frame, "Unexpected 22 slot");
      break;
  }
}

void WaterdropSerialRo::publish_raw_byte_(RawByteSensor sensor, uint8_t value) {
  const auto index = static_cast<size_t>(sensor);
  assert(index < raw_byte_sensors_.size());
  if (raw_byte_sensors_[index] != nullptr) {
    raw_byte_sensors_[index]->publish_state(value);
  }
}

void WaterdropSerialRo::publish_unexpected_frame_(const frame::Frame &frame, const char *reason) {
  const auto frame_text = frame.to_string();
  ESP_LOGW(TAG, "%s: %s", reason, frame_text.c_str());

  if (unexpected_frame_sensor_ == nullptr) {
    return;
  }

  std::string state = reason;
  state += ": ";
  state += frame_text;
  unexpected_frame_sensor_->publish_state(state);
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
