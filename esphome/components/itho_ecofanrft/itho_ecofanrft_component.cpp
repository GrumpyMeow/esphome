#include "esphome/core/log.h"
#include "itho_ecofanrft.h"
#include "cc1101.h"
#include "itho_cc1101_protocol.h"


namespace esphome {
namespace itho_ecofanrft {

static const char *TAG = "itho_ecofanrft.component";

void ICACHE_RAM_ATTR IthoEcoFanRftComponentStore::gpio_intr(IthoEcoFanRftComponentStore *arg) {
  arg->data_available = true;
  arg->count = (arg->count + 1) % 0xFF;
}
void ICACHE_RAM_ATTR IthoEcoFanRftComponentStore::reset() {
  data_available = false;
}

void itho_ecofanrft::IthoEcoFanRftComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "Fan '%s':", this->fan_->get_name().c_str());

  std::vector<uint8_t> config = this->cc1101_->read_burst_register(0x00, 47);
  for (uint8_t i = 0; i < config.size(); i++) {
    ESP_LOGCONFIG(TAG, "Config register [%02X] => [%02X]", i, config[i]);
  }
}
void IthoEcoFanRftComponent::setup() {

  if (this->cc1101_ == nullptr) {
    this->cc1101_ = new CC1101(this, this->parent_->get_miso(), this->cs_);
  }

  if (this->itho_cc1101_ == nullptr) {
    this->itho_cc1101_ = new IthoCC1101(this->cc1101_);
  }

  this->spi_setup();

  if (!this->cc1101_->init()) {
    this->mark_failed();
    return;
  }

  // Setup module for receiving RF event
  this->itho_cc1101_->init_receive_mode();

  // Enable interrupt on packet in RX FIFO
  this->store_.data_available = false;
  this->store_.count = 0;
  this->store_.pin = this->irq_->to_isr();
  this->irq_->attach_interrupt(IthoEcoFanRftComponentStore::gpio_intr, &this->store_, RISING);
 
  auto traits = fan::FanTraits(false, true);    // No oscillating, just speed
  this->fan_->set_traits(traits);
  this->fan_->add_on_state_callback([this]() { this->next_update_ = true; });

  // Set CC1101 in receive mode
  this->itho_cc1101_->enable_receive_mode();
}
void IthoEcoFanRftComponent::loop() {

  if (this->store_.data_available) {

    this->store_.reset();

    int16_t rssi = this->cc1101_->read_rssi();

    ESP_LOGD(TAG, "Data available in RX FIFO! (%02x) (%4d dBm)", this->store_.count, rssi);

    if (rssi > -75) {
        std::vector <uint8_t> payload = this->itho_cc1101_->get_data();

        for (uint8_t i : payload) {
            ESP_LOGV(TAG, "Packet payload (%02X)", i);
        }
    }

    //this->itho_cc1101_->send_command("timer1");
#if 0                   

    { 

        if (rssi > -75) {
            std::vector <uint8_t> data = this->cc1101_->receive_data(64);
            //this->cc1101_->write_command_strobe(CC1101_SFRX);
            //
            ESP_LOGD(TAG, "Good RSSI, probing..");

            //static const std::vector<uint8_t> itho_cc1101_header = {0x00, 0xb3, 0x2a, 0xab, 0x2a};
            auto pos = std::search(data.begin(), data.end(), itho_cc1101_header.begin(), itho_cc1101_header.end());
            if (pos != data.end()) {
                uint8_t offset = pos - data.begin();
                ESP_LOGV(TAG, "Found Itho header at offset %d", offset);

#if 0
                if (offset > 0) {
                    for (uint8_t i : data) {
                        //ESP_LOGV(TAG, "Packet data (%02X)", i);
                    }
                }
#endif

                auto end = std::find(pos, data.end(), 0xAC);
                if (end == data.end()) {
                    end = std::find(pos, data.end(), 0xCA);
                }

                if (end != data.end()) {
                    uint8_t endoffset = end - pos;
                    ESP_LOGV(TAG, "Found Itho footer at offset %d", endoffset);
                    ESP_LOGV(TAG, "Packet size = %d", endoffset - 5);

                    std::vector <bool> raw1, raw2, raw3;
                    raw1.resize((endoffset - 5) * 8, false);
                    for (uint8_t i = 0; i < (endoffset - 5); i++) {
                        uint8_t c = data[i + 5 + offset];
                        for (uint8_t j = 0; j < 8; j++) {
                            uint16_t idx = i * 8 + 7 - j;
                            bool v = (c >> j) & 1;
                            raw1[idx] = v;
                        }
                    }

                    raw2.resize(raw1.size() / 2, false);
                    for (uint16_t i = 0; i < raw2.size(); i++) {
                        raw2[i] = raw1[i*2];
                    }

                    // div into group of 5, drop last bit and reverse
                    unsigned int nf = (raw2.size() / 5);
                    unsigned int fl = nf * 4;
                    raw3.resize(fl, false);
                    for (unsigned int i = 0; i < nf; i++)
                    {
                        for (unsigned int j = 0; j < 4; j++)
                        {
                            raw3[(i * 4 + j)] = raw2[(i * 5 + (3 - j))];
                        }
                        //std::cout << raw2[i*5+4];
                    }

                    std::vector <uint8_t> out;
                    out.resize(raw3.size() / 8, 0);

                    for (unsigned int i = 0; i < out.size(); i++) {
                        uint8_t x;
                        x = raw3[i*8] << 7 | raw3[i*8+1] << 6 | raw3[i*8+2] << 5 | raw3[i*8+3] << 4;
                        x |= raw3[i*8+4] << 3 | raw3[i*8+5] << 2 | raw3[i*8+6] << 1 | raw3[i*8+7];
                        out[i] = x;
                    }

                    for (uint8_t i : out) {
                        ESP_LOGV(TAG, "Packet payload (%02X)", i);
                    }

                    uint8_t crc = 0;
                    for (unsigned int i = 0; i < out.size() - 1; i++) {
                        crc = (crc + out[i]) & 0xFF;
                    }
                    //printf("%02x\n", crc);
                    uint8_t crc2 = 0 - crc;
                    ESP_LOGV(TAG, "Packet crc (%02X)", crc2);



#if 0

                    std::vector <uint8_t> p;

                    for (uint8_t i = offset + 5; i < endoffset - 1; i+=2) {

                        uint16_t x = data[i] << 8 | data[i+1];
                        if (i == offset + 5) {
                            ESP_LOGV(TAG, "Packet (%04X)", x);
                        }
                        x = ((x & 0x8888) >> 2) | ((x & 0x2222) >> 1);
                        if (i == offset + 5) {
                            ESP_LOGV(TAG, "Packet (%04X)", x);
                        }
                        x = ((x & 0x3030) >> 2) | ((x & 0x0303) >> 0);
                        if (i == offset + 5) {
                            ESP_LOGV(TAG, "Packet (%04X)", x);
                        }
                        x = ((x & 0x0F00) >> 4) | ((x & 0x000F) >> 0);
                        if (i == offset + 5) {
                            ESP_LOGV(TAG, "Packet (%04X)", x);
                        }
                        p.push_back((uint8_t) x);
                    }


                    for (uint8_t i : p) {
                        ESP_LOGV(TAG, "Packet payload (%02X)", i);
                    }

#endif
   
                } else {

                    for (uint8_t i : data) {
                        ESP_LOGV(TAG, "Packet payload (%02X)", i);
                    }
                }
            } else {
                ESP_LOGV(TAG, "No header found");
            }
        } else {
            ESP_LOGV(TAG, "Too low RSSI");
        }
#endif
    this->itho_cc1101_->enable_receive_mode();
  }


  if (!this->next_update_) {
    return;
  }
  this->next_update_ = false;

  {
    std::string speed = "";

    if (this->fan_->state) {
      if (this->fan_->speed == fan::FAN_SPEED_LOW)
        speed = "low";
      else if (this->fan_->speed == fan::FAN_SPEED_MEDIUM)
        speed = "medium";
      else if (this->fan_->speed == fan::FAN_SPEED_HIGH)
        speed = "high";
    }
    ESP_LOGD(TAG, "Setting speed: '%s'", speed.c_str());
    this->itho_cc1101_->send_command(speed);
  }

#if 0
    bool enable = this->fan_->state;
    if (enable) {
     // this->output_->turn_on();
    } else {
     // this->output_->turn_off();
    }
    ESP_LOGD(TAG, "Setting itho_ecofanrft state: %s", ONOFF(enable));
  }
#endif
}
float IthoEcoFanRftComponent::get_setup_priority() const { return setup_priority::DATA; }

void IthoEcoFanRftComponent::join() {
  ESP_LOGVV(TAG, "Fan '%s': join() called", this->fan_->get_name().c_str());
}

} // namespace itho_ecofanrft
}  // namespace esphome
