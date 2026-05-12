#include <cxxflags.inc>
#include "waterdrop_serial.h"

#include "esphome/core/log.h"
#ifdef USE_ESP32
#include "esphome/components/uart/uart_component_esp_idf.h"
#endif

namespace esphome::waterdrop_serial {

WaterdropSerial::WaterdropSerial(const char *tag, frame::Source source) : tag_(tag),
    parser_(source), outgoing_source_(frame::opposite(source)) {}

void WaterdropSerial::loop() {
  int budget = 2 * frame::_details::FRAME_MAX_LENGTH;

  while (budget-- > 0 && available()) {
    uint8_t byte;
    if (!read_byte(&byte)) break;

    frame::Frame parsed_frame;
    if (parser_.feed(byte, parsed_frame)) {
      handle_frame(parsed_frame);
    }
  }
}

bool WaterdropSerial::is_tx_idle() const {
#ifdef USE_ESP32
  if (parent_ != nullptr) {
    auto idf_uart = static_cast<uart::IDFUARTComponent *>(parent_);
    auto uart_num = static_cast<uart_port_t>(idf_uart->get_hw_serial_number());
    return uart_wait_tx_done(uart_num, 0) == ESP_OK;
  }
#else
#  error "is_tx_idle not implemented for this platform"
#endif
  return true;
}

void WaterdropSerial::dump_config() {
#ifdef USE_ESP32
  if (parent_ != nullptr) {
    auto idf_uart = static_cast<uart::IDFUARTComponent *>(parent_);
    ESP_LOGCONFIG(tag_, "  UART Bus: %u", idf_uart->get_hw_serial_number());
  }
#endif

  const auto &counters = parser_.counters();
  ESP_LOGCONFIG(tag_, "  Counters:");
  ESP_LOGCONFIG(tag_, "    RX bytes: %zu", counters.rx_bytes);
  ESP_LOGCONFIG(tag_, "    Correct frames: %zu", counters.frames_ok);
  ESP_LOGCONFIG(tag_, "    Junk bytes: %zu", counters.rx_junk_bytes);
  ESP_LOGCONFIG(tag_, "    Invalid lengths: %zu", counters.invalid_lengths);
  ESP_LOGCONFIG(tag_, "    Invalid payload checksums: %zu", counters.invalid_checksum_payload);
  ESP_LOGCONFIG(tag_, "    Invalid frame checksums: %zu", counters.invalid_checksum_frame);
  ESP_LOGCONFIG(tag_, "    Invalid commands: %zu (last: 0x%02X)", counters.invalid_command,
      counters.last_invalid_command);
}

void WaterdropSerial::send_message(frame::Command command, const void *payload, size_t payload_length) {
  frame::Frame frame;
  assert(payload_length <= frame.payload.size());
  frame.source = outgoing_source_;
  frame.command = command;
  frame.payload_length = static_cast<uint8_t>(payload_length);
  std::copy(static_cast<const uint8_t *>(payload), static_cast<const uint8_t *>(payload) + payload_length, frame.payload.begin());

  frame.write(*this);
}

}  // namespace esphome::waterdrop_serial
