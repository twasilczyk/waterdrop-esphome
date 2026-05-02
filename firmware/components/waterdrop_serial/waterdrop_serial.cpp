#include <cxxflags.inc>
#include "waterdrop_serial.h"

#include "esphome/core/log.h"
#ifdef USE_ESP32
#include "esphome/components/uart/uart_component_esp_idf.h"
#endif

namespace esphome::waterdrop_serial {

WaterdropSerial::WaterdropSerial(frame::Source source) : parser_(source) {}

void WaterdropSerial::loop() {
  int budget = 2 * frame::_details::FRAME_MAX_LENGTH;

  while (budget-- > 0 && available()) {
    uint8_t byte;
    if (!read_byte(&byte)) break;

    frame::Frame parsed_frame;
    if (parser_.feed(byte, parsed_frame)) {
      // TODO: handle_frame in message.h Parser, then call handle_message_ here
      handle_frame_(parsed_frame);
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

void WaterdropSerial::dump_base_config(const char *tag) const {
#ifdef USE_ESP32
  if (parent_ != nullptr) {
    auto idf_uart = static_cast<uart::IDFUARTComponent *>(parent_);
    ESP_LOGCONFIG(tag, "  UART Bus: %u", idf_uart->get_hw_serial_number());
  }
#endif

  const auto &counters = parser_.counters();
  ESP_LOGCONFIG(tag, "  Counters:");
  ESP_LOGCONFIG(tag, "    RX bytes: %zu", counters.rx_bytes);
  ESP_LOGCONFIG(tag, "    Correct frames: %zu", counters.frames_ok);
  ESP_LOGCONFIG(tag, "    Junk bytes: %zu", counters.rx_junk_bytes);
  ESP_LOGCONFIG(tag, "    Invalid lengths: %zu", counters.invalid_lengths);
  ESP_LOGCONFIG(tag, "    Invalid payload checksums: %zu", counters.invalid_checksum_payload);
  ESP_LOGCONFIG(tag, "    Invalid frame checksums: %zu", counters.invalid_checksum_frame);
  ESP_LOGCONFIG(tag, "    Invalid commands: %zu (last: 0x%02X)", counters.invalid_command,
      counters.last_invalid_command);
}

}  // namespace esphome::waterdrop_serial
