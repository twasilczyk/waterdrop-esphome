#pragma once

#include "esphome/components/waterdrop_serial/frame.h"
#include "esphome/components/waterdrop_serial/waterdrop_serial.h"

#include <chrono>
#include <cstdint>
#include <map>
#include <vector>

namespace esphome::waterdrop_serial::faucet {

class WaterdropSerialFaucet : public WaterdropSerial {
 public:
  WaterdropSerialFaucet();
  void dump_config() override;
  void loop() override;

 private:
  void handle_frame_(const frame::Frame &frame) override;

  // TODO: remove
  void send_frame(frame::Command command, const std::vector<uint8_t> &payload);

  void send_c2_();
  void send_c5_();

  frame::Command next_command_ = frame::Command::COMMAND_C2;
  std::chrono::steady_clock::time_point next_command_at_ = {};

  uint8_t cc5_counter_ = 0;
  std::vector<std::vector<uint8_t>> cc5_data_{
    // A: 00 (rare) or 0A
    //                                            A
    {0x01, 0x00, 0x40, 0x22, 0x00, 0x00, 0x00, 0x0A},

    // A: 00, 34, 35
    //                         A
    {0x02, 0x00, 0x00, 0x00, 0x35, 0x08, 0x34, 0x00},

    // A: 0C, 0E, 0D (rare), 19 (rare)
    // B: 01, 03
    // C: 90, B0
    //             A     B     C
    {0x03, 0x00, 0x0E, 0x01, 0xB0, 0x00, 0x0A, 0x00},

    // A: F1, F3, FF
    // B: F1, F3
    //       A     B
    {0x04, 0xF1, 0xF1, 0x00, 0x00, 0x00, 0x0E, 0xFF},
    {0x05, 0x00, 0x00, 0x0E, 0x0E, 0x00, 0x00, 0x00},
  };

  std::map<uint8_t, std::vector<uint8_t>> c22_data_{
    {0x0E, {0x0E, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x11}},

    // CF, RO, CB (H, L): filter total life (when it turns red).
    //            >- CF   -<  >-  RO  -<  >-  CB  <-
    {0x0F, {0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFe, 0x99, 0xFF, 0x33}},

    // TDS: 1-999 value with 1000-9999 displaying as F-codes.
    // A: 0C or 0E
    //                                   |  H  TDS L|         A
    {0x03, {0x03, 0x00, 0x00, 0x00, 0x00,    0,    1, 0x00, 0x0C}},

    // CF, RO, CB (H, L): filter used life. Turns orange at 360 units before red
    //            >- CF   -<  >-  RO  -<  >-  CB  <-
    {0x02, {0x02, 0xFE, 0x96, 0xFE, 0x97, 0xFE, 0x98, 0x00, 0x00}},

    // A: 0A or 00 (rare) - A always the same and same in 01 and 0D slots
    //                    A           A
    {0x0D, {0x0D, 0x00, 0x0A, 0x00, 0x0A, 0x00, 0x0A, 0x00, 0x00}},

    // Seems to be the same as 0D slot
    {0x01, {0x01, 0x00, 0x0A, 0x00, 0x0A, 0x00, 0x0A, 0x00, 0x00}},
  };
};

}  // namespace esphome::waterdrop_serial::faucet
