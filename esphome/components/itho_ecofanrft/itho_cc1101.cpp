//#include "esphome/core/component.h"
//#include "esphome/components/spi/spi.h"
//#include "itho_ecofanrft.h"

#include <algorithm>

#include "esphome/core/log.h"
#include "itho_cc1101.h"

namespace esphome {
namespace itho_ecofanrft {

static const char *TAG = "itho_ecofanrft.itho_cc1101";

void IthoCC1101::init_receive_mode() {
    
  for (register_setting r : itho_cc1101_config_receive) {
      this->cc1101_->write_register(r.address, r.data);
  }
  this->cc1101_->write_burst_register(CC1101_PATABLE, itho_cc1101_patable_receive);

  this->cc1101_->write_command_strobe(CC1101_SCAL);
  delay(1);
}

std::vector <uint8_t> IthoCC1101::get_data() {

    std::vector <uint8_t> out;
    std::vector <uint8_t> data = this->cc1101_->receive_data(MAX_PACKET_LEN);

    auto pos = std::search(data.begin(), data.end(), itho_cc1101_header.begin(), itho_cc1101_header.end());
    if (pos != data.end()) {
        uint8_t offset = pos - data.begin();
        ESP_LOGV(TAG, "Found Itho header at offset: %d", offset);

        auto end = std::find_first_of(pos, data.end(), itho_cc1101_footer.begin(), itho_cc1101_footer.end());
#if 0
        auto end = std::find(pos, data.end(), 0xAC);
        if (end == data.end()) {
            end = std::find(pos, data.end(), 0xCA);
        }
#endif

        if (end != data.end()) {
            uint8_t endoffset = end - pos;
            uint8_t packet_size = end - pos - itho_cc1101_header.size();
            ESP_LOGV(TAG, "Found Itho footer at offset: %d", endoffset);
            ESP_LOGV(TAG, "Packet size: %d", packet_size);

#if 0
            if (packet_size % 2 == 1) {
                ESP_LOGV(TAG, "Last: %02x %02x %02x %02x", data[offset + 5 + packet_size - 2], 
                        data[offset + 5 + packet_size - 1], data[offset + 5 + packet_size], data[offset + 5 + packet_size + 1]);
            }
#endif

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

            out.resize(raw3.size() / 8, 0);

            for (unsigned int i = 0; i < out.size(); i++) {
                uint8_t x;
                x = raw3[i*8] << 7 | raw3[i*8+1] << 6 | raw3[i*8+2] << 5 | raw3[i*8+3] << 4;
                x |= raw3[i*8+4] << 3 | raw3[i*8+5] << 2 | raw3[i*8+6] << 1 | raw3[i*8+7];
                out[i] = x;
            }

#ifdef ESPHOME_LOG_HAS_VERY_VERBOSE
            for (uint8_t i : out) {
                ESP_LOGVV(TAG, "Packet payload (%02X)", i);
            }
#endif

            uint8_t crc = 0;
            for (unsigned int i = 0; i < out.size(); i++) {
                crc = (crc + out[i]) & 0xFF;
            }
            //printf("%02x\n", crc);
            //uint8_t crc2 = 0 - crc;
            ESP_LOGV(TAG, "Packet CRC: 0x%02X (%s)", crc, crc ? "invalid" : "valid");

        } // Footer
    } // Header
    return out;
}

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


bool IthoCC1101::get_fan_speed(std::vector<uint8_t> peer_rf_address, uint8_t *speed) {

    std::vector<uint8_t> data = this->get_data();
    *speed = 0xFF;

    if (has_valid_crc(data)) {

        auto id = std::search(data.begin(), data.end(), peer_rf_address.begin(), peer_rf_address.end());
        auto pos = std::search(data.begin(), data.end(), itho_status.begin(), itho_status.end());
        if (data.size() == 11 &&              // 1 + 3 + 5 + 1 + 1
                data[0] == 0x14 &&
                pos - data.begin() == 4) {

            pos += itho_status.size();
            *speed = *pos;

            if (peer_rf_address.size() > 0 && id - data.begin() != 1) {
                // Not our peer rf address
                return false;
            }

            ESP_LOGD(TAG, "Received fan speed status: peer rf address %02X:%02X:%02X, speed 0x%02X", data[1], data[2], data[3], *speed);

            if (peer_rf_address.size() ==  0) {
                ESP_LOGI(TAG, "No peer rf address configured, ignoring above status!");
                return false;
            }
            return true;
        }
    }

    return false;
}

uint8_t IthoCC1101::calc_crc(std::vector<uint8_t> data) {

    uint8_t crc = 0;
    for (uint8_t i = 0; i < data.size(); i++) {
        crc = (crc + data[i]) & 0xFF;
    }
    return 0 - crc;
}


void IthoCC1101::send_command(std::string command) {

    auto cmd_it = itho_commands.find(command);
    if (cmd_it == itho_commands.end()) {
        ESP_LOGD(TAG, "Unknown command '%s'", command.c_str());
        return;
    }

    std::vector<uint8_t> cmd = packet::lead_in;
    cmd.insert(cmd.end(), this->rf_address_.begin(), this->rf_address_.end());
    cmd.push_back(this->counter_++);
    cmd.insert(cmd.end(), cmd_it->second.begin(), cmd_it->second.end());

    if (command == "join") {
        cmd.insert(cmd.end(), this->rf_address_.begin(), this->rf_address_.end());
        cmd.insert(cmd.end(), itho_commands.at("join_2").begin(), itho_commands.at("join_2").end());
        cmd.insert(cmd.end(), this->rf_address_.begin(), this->rf_address_.end());
    } else if (command == "leave") {
        cmd.insert(cmd.end(), this->rf_address_.begin(), this->rf_address_.end());
    }

    cmd.push_back(this->calc_crc(cmd));

    //for (uint8_t i : cmd) {
    //   ESP_LOGV(TAG, "Packet send payload (%02X)", i);
    //}

    std::vector <bool> raw1, raw2;
    // FIXME:  should round up to nearest multiple of 4, with bits to true
    //raw1.resize(5 * cmd.size() * 2, false);       // 2 nibbles per byte, 5 bits per nibble
    raw1.resize(((5 * cmd.size() * 2 + 4 - 1) & -4), true);       // 2 nibbles per byte, 5 bits per nibble, round up to multiple of 4
    uint16_t idx;
    bool v;
    for (uint8_t i = 0; i < cmd.size(); i++) {
        for (uint8_t j = 0; j < 2; j++) {
            uint8_t n = cmd[i] >> (4 * (1 - j));
            for (uint8_t k = 0; k < 4; k++) {
                idx = 5 * (2 * i + j) + k;
                v = (n >> k) & 1;
                raw1[idx] = v;
            }
            //raw1[5 * (2 * i + j) + 4] = true;  //is already tre
        }
    }

    //for (bool bb : raw1) {
    //    ESP_LOGV(TAG, "Packet b payload (%02X)", bb);
    //}

    // manchester encode
    raw2.resize(2 * raw1.size());
    for (uint16_t i = 0; i < raw1.size(); i++)
    {
        raw2[(2 * i)] = raw1[i];
        raw2[(2 * i) + 1] = !raw1[i];
    }

    std::vector <uint8_t> out;
    out.resize(raw2.size() / 8, 0);

    for (unsigned int i = 0; i < out.size(); i++) {
        uint8_t x;
        x = raw2[i*8] << 7 | raw2[i*8+1] << 6 | raw2[i*8+2] << 5 | raw2[i*8+3] << 4;
        x |= raw2[i*8+4] << 3 | raw2[i*8+5] << 2 | raw2[i*8+6] << 1 | raw2[i*8+7];
        out[i] = x;
    }
#if 0
    for (uint8_t ii : out) {
        ESP_LOGV(TAG, "Packet rf payload (%02X)", ii);
    }
#endif   

    std::vector<uint8_t> rf_data = itho_cc1101_header;
    rf_data.insert(rf_data.end(), out.begin(), out.end());
    //rf_data.insert(rf_data.end(), packet::footer.begin(), packet::footer.end());
#if 1
    if (cmd.size() % 2 == 0) {
        //rf_data.insert(rf_data.end(), packet::footer_even.begin(), packet::footer_even.end());
        rf_data.push_back(packet::footer_even);
    } else {
        //rf_data.insert(rf_data.end(), packet::footer_odd.begin(), packet::footer_odd.end());
        rf_data.push_back(packet::footer_odd);
    }
#endif
    while (rf_data.size() < MAX_PACKET_LEN) {
        //rf_data.push_back(0xAA);
        rf_data.push_back(packet::post_amble);
    }

    this->cc1101_->send_data(rf_data);

 
#if 0

     unsigned int cmdLength = 1 + cmd.id().length() + 1 + cmd.command().length() + 1;
    if (DEBUG)
    {
        Serial.printf("IthoDecode::encode cmd=%s  l=%d\n", cmd.toString().c_str(), cmdLength);
    }
    BitArray tmp(0, cmdLength * 8);
    tmp.append(cmd.lead());
    tmp.append(cmd.id());
    tmp.append(cmd.counter());
    tmp.append(cmd.command());
    tmp.append(cmd.crc());

    if (DEBUG)
    {
        printf("After concat\n");
        tmp.print();
        Serial.println(tmp.toString(4));
    }

    size_t numOct = (cmdLength * 8) / 4;
    BitArray tmp2(numOct * 5);
    for (size_t i = 0; i < numOct; i++)
    {
        // use a half byte, reverse
        for (size_t j = 0; j < 4; j++)
        {
            tmp2.set((5 * i) + (3 - j), tmp.get((4 * i) + j));
        }
        // set and extra bit to 1 to complete the octet
        tmp2.set((5 * i) + 4, true);
    }

    if (DEBUG)
    {
        printf("After de oct reverse (n=%d, l=%d)\n", numOct, tmp2.length());
        //tmp2.print();
        Serial.println(tmp2.toString(4));
    }

    BitArray tmp3(0, (6 * 8) + (tmp2.length() * 2) + (16 * 8));
    tmp3.append(ByteArray(_preamble, 6));
    for (size_t i = 0; i < tmp2.length(); i++)
    {
        tmp3.set((6 * 8) + (i * 2), tmp2.get(i));
        tmp3.set((6 * 8) + (i * 2) + 1, !tmp2.get(i));
    }

    tmp3.append(ByteArray(_postamble, 16));
    if (DEBUG)
    {
        printf("After bit white (l=%d)\n", tmp3.length());
        tmp3.print();
    }
    return ByteArray(tmp3);
#endif

}
   

} // namespace itho_ecofanrft
} // namespace esphome

