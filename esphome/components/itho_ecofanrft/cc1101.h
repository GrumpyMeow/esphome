#pragma once

#include "esphome/core/component.h"

namespace esphome {
namespace itho_ecofanrft {


class CC1101 {
 
 public:
   CC1101(GPIOPin *cs, GPIOPin *miso): cs_(cs), miso_(miso) {};
 
 void init();

 void send_data(const std::vector<uint8_t> &data);
 std::vector<uint8_t> &receive_data(const uint8_t max_length);

 uint8_t write_command_strobe(const uint8_t command);

 void write_register(const uint8_t address, const uint8_t data);
 uint8_t read_register(const uint8_t address);
 
 void write_burst_register(const uint8_t address, const std::vector<uint8_t> &data);
 std::vector<uint8_t> read_burst_register(const uint8_t address, const uint8_t max_length);
 
 protected:
  void select_();
  void deselect_();

  uint8_t read_register_(uint8_t address);
  uint8_t read_register_median3_(uint8_t address);
  uint8_t read_register_with_sync_problem_(const uint8_t address);
 
  void reset_();

 GPIOPin *cs_{nullptr};
 GPIOPin *miso_{nullptr};
};


};  // namespace itho_ecofanrft
} // namespace itho_ecofanrft

