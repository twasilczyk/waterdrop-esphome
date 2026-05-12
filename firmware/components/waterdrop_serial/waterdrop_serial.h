#pragma once

#include "esphome/components/uart/uart.h"
#include "esphome/components/waterdrop_serial/frame.h"
#include "esphome/core/component.h"

namespace esphome::waterdrop_serial {

class WaterdropSerial : public Component, protected uart::UARTDevice {
 public:
  WaterdropSerial(const char *tag, frame::Source source);
  using uart::UARTDevice::set_uart_parent;
  void loop() override;

 protected:
  virtual void handle_frame(const frame::Frame &frame) = 0;
  void dump_config() override;
  bool is_tx_idle() const;

  template <typename T>
  void send_message(const T &message) {
    send_message(message.COMMAND, &message, sizeof(message));
  }

 private:
  const char *tag_;
  frame::Parser parser_;
  frame::Source outgoing_source_;

  void send_message(frame::Command command, const void *payload, size_t payload_length);
};

}  // namespace esphome::waterdrop_serial
