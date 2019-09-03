#include "itho_ecofanrft.h"
#include "esphome/core/log.h"

namespace esphome {
namespace itho_ecofanrft {

static const char *TAG = "itho_ecofanrft.component";

void itho_ecofanrft::IthoEcoFanRftComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "Fan '%s':", this->fan_->get_name().c_str());
}
void IthoEcoFanRftComponent::setup() {

  GPIOPin *miso = this->parent_->get_miso();

  uint8_t r = 0xfe;
  uint8_t s = 0xce;
  ESP_LOGCONFIG(TAG, "Setting up IthoEcoFanRft...");
//  this->spi_setup();
//
  delay(5);
  this->spi_setup();
  delay(5);

  this->cs_->digital_write(true);
  delayMicroseconds(10);
  this->cs_->digital_write(false);
  delayMicroseconds(10);
  this->cs_->digital_write(true);
  delayMicroseconds(40);

  // RESET_CCxxx0
  this->enable();
  while (miso->digital_read() );
  this->write_byte(0x30);
  this->disable();

  //this->cs_->digital_write(true);
  //delayMicroseconds(5);
  delay(1);

  ESP_LOGCONFIG(TAG, "Done with reset...(%x)", r);

  this->enable();
  while (miso->digital_read() );
  this->write_byte(0x3b);
  delayMicroseconds(100);
  this->disable();

  this->enable();
  while (miso->digital_read() );
  this->write_byte(0x3a);
  delayMicroseconds(100);
  this->disable();


  this->enable();
  while (miso->digital_read() );
  this->write_byte(0xf0);
  r = this->transfer_byte(0x00);
  //r = this->transfer_byte(0xf0);
  this->disable();

  this->enable();
  while (miso->digital_read() );
  this->write_byte(0xf1);
  s = this->transfer_byte(0x00);
  //s = this->transfer_byte(0xf1);
  this->disable();


#if 0
  r = 0xfe;
  this->enable();
  while (miso->digital_read()){ };
  r = this->transfer_byte(0xf1);        // Nop
  this->disable();

  this->enable();
  while (miso->digital_read()){ };
  s = this->transfer_byte(0xf5);
  this->disable();
#endif


  ESP_LOGCONFIG(TAG, "Done with SPI...(%x) state: %x", r, s);

#if 0
  this->enable();
  while (miso->digital_read()) yield();
  s = this->transfer_byte(0x3d | 0x80);        // Nop
  ESP_LOGCONFIG(TAG, "Result...(%x)", s);
  this->disable();

  this->enable();
  while (miso->digital_read()) yield();
  s = this->transfer_byte(0x82);
  ESP_LOGCONFIG(TAG, "Result...(%x)", s);
  this->disable();

  this->enable();
  while (miso->digital_read()) yield();
  this->write_byte(0x02);
  s = this->transfer_byte(0x2E);
  ESP_LOGCONFIG(TAG, "Result...(%x)", s);
  this->disable();

  this->enable();
  while (miso->digital_read()) yield();
  s = this->transfer_byte(0x82);
  ESP_LOGCONFIG(TAG, "Result...(%x)", s);
  this->disable();
#endif

  auto traits = fan::FanTraits(false, true);    // No oscillating, just speed
  this->fan_->set_traits(traits);
  this->fan_->add_on_state_callback([this]() { this->next_update_ = true; });
}
void IthoEcoFanRftComponent::loop() {
  if (!this->next_update_) {
    return;
  }
  this->next_update_ = false;

  {
    bool enable = this->fan_->state;
    if (enable) {
     // this->output_->turn_on();
    } else {
     // this->output_->turn_off();
    }
    ESP_LOGD(TAG, "Setting itho_ecofanrft state: %s", ONOFF(enable));
  }
}
float IthoEcoFanRftComponent::get_setup_priority() const { return setup_priority::DATA; }

void IthoEcoFanRftComponent::join() {
  ESP_LOGVV(TAG, "Fan '%s': join() called", this->fan_->get_name().c_str());
}

} // namespace itho_ecofanrft
}  // namespace esphome
