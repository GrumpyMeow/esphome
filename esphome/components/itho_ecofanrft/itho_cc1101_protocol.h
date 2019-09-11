#pragma once

//https://gathering.tweakers.net/forum/list_message/54115879#54115879

#include <unordered_map>
#include "esphome/components/fan/fan_state.h"
#include "cc1101_reg.h"

namespace esphome {
namespace itho_ecofanrft {

static const uint8_t MAX_PACKET_LEN = 0x3C;     // 60 bytes max in the air. Less than max RX fifo size.

typedef struct {
    uint8_t address;
    uint8_t data;
} register_setting;

static const std::vector<register_setting> itho_cc1101_config_receive2
{   
  {CC1101_IOCFG0,      0x2E},       //
  {CC1101_IOCFG2,      0x01},       //
  {CC1101_FIFOTHR,     0x0E},
  {CC1101_SYNC1,       0xAB},
  {CC1101_SYNC0,       0xFE},
  {CC1101_PKTLEN,      MAX_PACKET_LEN},
  {CC1101_FSCTRL1,    0x06},        //
  {CC1101_FSCTRL0,    0x00},        //
  {CC1101_PKTCTRL1,    0x60},
  {CC1101_PKTCTRL0,    0x00},
  {CC1101_FREQ2,       0x21},       //
  {CC1101_FREQ1,       0x65},       //
  {CC1101_FREQ0,       0x6A},       //
  {CC1101_MDMCFG4,     0x9A},       //
  {CC1101_MDMCFG3,     0x83},       //
  {CC1101_MDMCFG2,     0x06},       //+
  {CC1101_MDMCFG1,     0x22},       //+
  {CC1101_MDMCFG0,     0xF8},       //+
  {CC1101_CHANNR,     0x00},       //+
  {CC1101_DEVIATN,     0x50},       //
  {CC1101_FREND0,      0x17},       //
  {CC1101_FREND1,      0x56},       //
  {CC1101_FSCAL3,      0xA9},       //
  {CC1101_FSCAL2,      0x2A},       // //
  {CC1101_FSCAL1,      0x00},       //
  {CC1101_FSCAL0,      0x1F},       //
  {CC1101_TEST2,       0x81},
  {CC1101_TEST1,       0x35},
  {CC1101_TEST0,       0x09},       //
  {CC1101_FSTEST,       0x59},       //
  {CC1101_MCSM0,       0x18},       // +    +
  {CC1101_FOCCFG,     0x16},       // +   
  {CC1101_BSCFG,     0x6C},       // +   
  {CC1101_AGCCTRL2,     0x43},       // +   
  {CC1101_AGCCTRL1,     0x40},       // +   
  {CC1101_AGCCTRL0,     0x91},       // +   
};

//static const registerSetting_t preferredSettings[]= 
//{
static const std::vector<register_setting> itho_cc1101_config_receive
{   
  {CC1101_IOCFG2,      0x01},
  {CC1101_IOCFG0,      0x2E},
  {CC1101_FIFOTHR,     0x4E},   // ADC retention, 6dB att, 33/32 fifothr
  {CC1101_SYNC1,       0xAB},
  {CC1101_SYNC0,       0xFE},
  {CC1101_PKTLEN,      0x3F},
  {CC1101_PKTCTRL1,    0x60},
  {CC1101_PKTCTRL0,    0x00},
  {CC1101_FSCTRL1,     0x06},
  {CC1101_FREQ2,       0x21},
  {CC1101_FREQ1,       0x65},
  {CC1101_FREQ0,       0x6A},
  {CC1101_MDMCFG4,     0x9A},
  {CC1101_MDMCFG3,     0x83},
  {CC1101_MDMCFG2,     0x06},
  {CC1101_MDMCFG1,     0x42},
  {CC1101_DEVIATN,     0x50},
  {CC1101_MCSM0,       0x18},
  {CC1101_FOCCFG,      0x16},
  {CC1101_AGCCTRL2,    0x43},
  {CC1101_AGCCTRL1,    0x49},
  {CC1101_WORCTRL,     0xFB},
  {CC1101_FREND0,      0x17},
  {CC1101_FSCAL3,      0xE9},
  {CC1101_FSCAL2,      0x2A},
  {CC1101_FSCAL1,      0x00},
  {CC1101_FSCAL0,      0x1F},
  {CC1101_TEST2,       0x81},
  {CC1101_TEST1,       0x35},
  {CC1101_TEST0,       0x09},
};



//static const std::vector<uint8_t> itho_cc1101_patable_receive = {0x00,0x03,0x0f,0x27,0x51,0x81,0xcc,0xcd};
static const std::vector<uint8_t> itho_cc1101_patable_receive = {0x00,0x03,0x0F,0x27,0x50,0xC8,0xC3,0xC5};
//static const std::vector<uint8_t> itho_cc1101_patable_receive = {0x6F,0x26,0x2E,0x7F,0x8A,0x84,0xCA,0xC4};

const uint8_t ithoMessage2JoinCommandBytes[] = {9,90,170,90,165,165,89,106,85,149,102,89,150,170,165};

static const std::unordered_map<std::string, std::vector<uint8_t>> itho_commands = {
    { "min",    { 0x22, 0xf1, 0x03, 0x00, 0x01, 0x04 } },        // ok
    { "low",    { 0x22, 0xf1, 0x03, 0x00, 0x02, 0x04 } },        // ok
    { "medium", { 0x22, 0xf1, 0x03, 0x00, 0x03, 0x04 } },        // ok
    { "high",   { 0x22, 0xf1, 0x03, 0x00, 0x04, 0x04 } },        // ok
    { "max",    { 0x22, 0xf1, 0x03, 0x00, 0x05, 0x04 } },        // ok  // full power

    { "timer1", { 0x22, 0xf3, 0x03, 0x00, 0x80, 0x01 } },        // ok
    { "timer2", { 0x22, 0xf3, 0x03, 0x00, 0x80, 0x02 } },
    { "timer3", { 0x22, 0xf3, 0x03, 0x00, 0x80, 0x03 } },

    { "join",   { 0x1f, 0xc9, 0x0c, 0x00, 0x22, 0xf1 } },   // ... <join> <id> <join_2> <id> <crc>      //ok
    { "join_2", { 0x01, 0x10, 0xe0 } },
    { "leave",  { 0x1f, 0xc9, 0x06, 0x00, 0x1f, 0xc9 } },   // ... <leave> <id> <crc>
};

/*
 * Status messages:
 * lead_in: 1 byte (0x14)
 * id: 3 bytes (e.g. 0x50 0x72 0xA2)
 * msg: 5 bytes
 * value: 1 byte
 * crc: 1 byte
 */
static const std::vector<uint8_t> itho_status = { 0x31, 0xd9, 0x03, 0x00, 0x00 };

static const std::unordered_map<uint8_t, fan::FanSpeed> itho_status_speed = {
    { 0x01, fan::FAN_SPEED_LOW },
    { 0x50, fan::FAN_SPEED_MEDIUM },
    { 0x8c, fan::FAN_SPEED_HIGH },
};



// non mancher packet start bytes
static const std::vector<uint8_t> itho_cc1101_header = {0x00, 0xb3, 0x2a, 0xab, 0x2a};

// packet terminators 
static const std::vector<uint8_t> itho_cc1101_footer = {0xac, 0xca};

namespace packet {
static const std::vector<uint8_t> lead_in = { 0x16 };
//static const std::vector<uint8_t> remote_id = { 0x0a, 0x57, 0x51 };
static const std::vector<uint8_t> remote_id = { 0x1a, 0x57, 0x51 };
static const std::vector<uint8_t> peer_id = { 0x50, 0x72, 0xA2 };
static const uint8_t footer_even = 0xac;
static const uint8_t footer_odd = 0xca;
static const uint8_t post_amble = 0xaa;
static const std::vector<uint8_t> footer = { 0xac };
static const std::vector<uint8_t> footer_join = { 0xca };
}



};  // namespace itho_ecofanrft
} // namespace itho_ecofanrft


