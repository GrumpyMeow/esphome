#pragma once

#include "esphome/core/component.h"
#include "esphome/core/automation.h"
#include "esphome/components/fan/fan_state.h"
#include "esphome/components/spi/spi.h"

namespace esphome {
namespace itho_ecofanrft {

class IthoEcoFanRftComponent;

class IthoEcoFanRftFan : public fan::FanState {
 public:
  IthoEcoFanRftFan(IthoEcoFanRftComponent *parent) : parent_(parent) {};
  //void setup() override;
  //void loop() override;
  //void dump_config() override;
  //float get_setup_priority() const override;

 protected:
  IthoEcoFanRftComponent *parent_;
};

class IthoEcoFanRftComponent : public Component,
                               public spi::SPIDevice<spi::BIT_ORDER_MSB_FIRST, spi::CLOCK_POLARITY_LOW,
                                                     spi::CLOCK_PHASE_LEADING, spi::DATA_RATE_1MHZ> {

 public:
  void setup() override;
  void loop() override;
  void dump_config() override;
  float get_setup_priority() const override;

  void set_mosi_pin(GPIOPin *mosi) { mosi_ = mosi; }

  IthoEcoFanRftFan *get_fan() {
      auto f = new IthoEcoFanRftFan(this);
      this->fan_ = f;
      return f;
  }

  void join();


 protected:
  IthoEcoFanRftFan *fan_;
  GPIOPin *mosi_;

  bool next_update_{true};
};
template<typename... Ts> class JoinAction : public Action<Ts...> {
 public:
  explicit JoinAction(IthoEcoFanRftComponent *component) : component_(component) {}

  void play(Ts... x) override {
    this->component_->join();
  }
 protected:
  IthoEcoFanRftComponent *component_;
};

}  // namespace itho_ecofanrft
}  // namespace esphome
