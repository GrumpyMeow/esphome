#pragma once

#include "esphome/core/component.h"
#include "cc1101.h"
#include "itho_cc1101_protocol.h"

namespace esphome {
namespace itho_ecofanrft {

class IthoCC1101 {
 
 public:
  IthoCC1101(CC1101 *cc1101) : cc1101_(cc1101) {};
 
  void init_receive_mode();
  void enable_receive_mode() {
     this->cc1101_->flush_rxfifo();
     this->cc1101_->receive();
  }
  std::vector<uint8_t> get_data();
  bool has_valid_crc();

  uint8_t calc_crc(std::vector<uint8_t> data);

  void send_command(std::string command);
 
 protected:

  void manchester_decode_();
  void remove_sync_bits_and_reverse_();

  void manchester_encode_();
  void add_sync_bits_and_reverse_();
  
  std::vector<uint8_t> data_;
  std::uint8_t counter_ = 0;

  CC1101 *cc1101_{nullptr};
};


} // namespace itho_ecofanrft
} // namespace esphome

