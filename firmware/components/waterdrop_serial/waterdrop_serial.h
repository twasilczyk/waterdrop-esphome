#pragma once

#include "esphome/components/uart/uart.h"
#include "esphome/components/waterdrop_serial/frame.h"
#include "esphome/core/component.h"

namespace esphome::waterdrop_serial {

class WaterdropSerial : public Component, protected uart::UARTDevice {
 public:
  WaterdropSerial(frame::Source source);
  using uart::UARTDevice::set_uart_parent;
  void loop() override;

 protected:
  virtual void handle_frame_(const frame::Frame &frame) = 0;
  void dump_base_config(const char *tag) const;
  bool is_tx_idle() const;

 private:
  frame::Parser parser_;
};

}  // namespace esphome::waterdrop_serial
