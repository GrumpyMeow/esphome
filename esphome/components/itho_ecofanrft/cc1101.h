#pragma once

#include "esphome/core/component.h"
#include "esphome/components/spi/spi.h"
//#include "itho_ecofanrft.h"

namespace esphome {
namespace itho_ecofanrft {

class CC1101 {
 
 public:
  CC1101(spi::SPIActions *spi, GPIOPin *miso, GPIOPin *cs) : spi_(spi), miso_(miso), cs_(cs) {};
 
  bool init();

  void send_data(const std::vector<uint8_t> &data);
  std::vector<uint8_t> receive_data(const uint8_t max_length);

  uint8_t write_command_strobe(const uint8_t command);
  uint8_t sidle();
  void flush_rxfifo();
  void flush_txfifo();
  uint8_t receive();
  uint8_t send();

  void write_register(const uint8_t address, const uint8_t data);
  uint8_t read_register(const uint8_t address);
 
  void write_burst_register(const uint8_t address, const std::vector<uint8_t> &data);
  std::vector<uint8_t> read_burst_register(const uint8_t address, const uint8_t max_length);
 
  int16_t read_rssi();

 protected:
  void select_() {
    spi_->enable();
    while (miso_->digital_read());
  }
  void deselect_() { spi_->disable(); };

  uint8_t read_register_(const uint8_t address);
  uint8_t read_register_with_sync_problem_(const uint8_t address);
 
  void reset_(bool power_on_reset = false);

  spi::SPIActions *spi_{nullptr};
  GPIOPin *miso_{nullptr};
  GPIOPin *cs_{nullptr};
};


} // namespace itho_ecofanrft
} // namespace esphome

