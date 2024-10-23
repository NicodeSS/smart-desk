#include "rx_decoder.h"

// std::unordered_map<uint8_t, char> segmentMap = {
//     {0b00111111, '0'}, // 0
//     {0b00000110, '1'}, // 1
//     {0b01011011, '2'}, // 2
//     {0b01001111, '3'}, // 3
//     {0b01100110, '4'}, // 4
//     {0b01101101, '5'}, // 5,S
//     {0b01111101, '6'}, // 6
//     {0b00000111, '7'}, // 7
//     {0b01111111, '8'}, // 8
//     {0b01101111, '9'}, // 9
//     {0b01110111, 'A'}, // A,R
//     {0b01111100, 'b'}, // b
//     {0b00111001, 'C'}, // C
//     {0b01011000, 'c'}, // c
//     {0b01011110, 'd'}, // d
//     {0b01111001, 'E'}, // E
//     {0b01111011, 'e'}, // e
//     {0b01110001, 'F'}, // F
//     {0b01110110, 'H'}, // H
//     {0b01110100, 'h'}, // h
//     {0b00110000, 'I'}, // I(left)
//     {0b00001110, 'J'}, // J
//     {0b00111000, 'L'}, // L
//     {0b01010100, 'n'}, // n
//     {0b01011100, 'o'}, // o
//     {0b01110011, 'P'}, // P,p
//     {0b01100111, 'q'}, // q
//     {0b01010000, 'r'}, // r
//     {0b01111000, 't'}, // t
//     {0b00111110, 'U'}, // U
//     {0b00011100, 'u'}, // u
//     {0b01101110, 'y'}, // y
//     {0b01000000, '-'}, // -
//     {0b01001000, '='}, // =
//     {0b00000000, ' '}, // Blank
// };
namespace esphome
{
    namespace nicode_smart_desk
    {
        extern float max_desk_height;
        extern float min_desk_height;
        extern const char segment_map[128];

        bool RxDecoder::put(uint8_t b)
        {
            bool flag = false;
            switch (data_state)
            {
            case SYNC:
                if (b == 0x5A)
                {
                    buf[0] = b;
                    data_state = SCREEN1;
                    break;
                }
                else
                {
                    data_state = SYNC;
                    break;
                }
            case SCREEN1:
            {
                buf[1] = b;
                data_state = SCREEN2;
                break;
            }
            case SCREEN2:
            {
                buf[2] = b;
                data_state = SCREEN3;
                break;
            }
            case SCREEN3:
            {
                buf[3] = b;
                data_state = CHECKSUM;
                break;
            }
            case CHECKSUM:
            {

                buf[4] = b;
                uint8_t computed_checksum = (buf[1] + buf[2] + buf[3]) & 0xFF;
                if (computed_checksum == b)
                {
                    flag = true;
                    update_state();
                }
                // else
                // {
                //     ESP_LOGW("UART RX DECODER", "Controlbox --> Headset [Mismatch]: %X %X %X %X %Xvs%X", buf[0], buf[1], buf[2], buf[3], buf[4], computed_checksum);
                // }
                data_state = SYNC;
                break;
            }
            }
            return flag;
        }

        const uint8_t *RxDecoder::get_buffer() const
        {
            return buf;
        }

        std::string RxDecoder::decode()
        {
            display = "";
            for (size_t i = 1; i < 4; i++)
            {
                uint8_t segment = buf[i];

                bool has_dot = segment & 0x80;
                segment &= 0x7F;

                display += esphome::nicode_smart_desk::segment_map[segment];
                display += has_dot ? '.' : ' ';

                // ESP_LOGW("UART RX DECODER", "Current data: %X, hasDot %d, remove dot: %X", buf[i], hasDot, segment);
            }
            return display;
        }

        void RxDecoder::update_state()
        {
            if (memcmp(buf, buf_last, 5) == 0)
            {
                // same as last, do nothing;
                return;
            }

            decode();

            if (display == "      ")
            {
                // hibernating, pass
                return;
            }
            else if (std::regex_match(display, numeric_regex))
            {
                state = DISPLAYING_HEIGHT;
                std::string temp;
                for (auto ch : display)
                {
                    if (!std::isspace(ch))
                    {
                        temp += ch;
                    }
                }
                float result = std::stof(temp);
                if (abs(desk_height - result) < 3.0f || desk_height < min_desk_height || desk_height > max_desk_height)
                    desk_height = result;
                else
                {
                    ESP_LOGD("UART RX DECODER", "New height %.1f has huge gap with old height %.1f, ignoring.", result, desk_height);
                }
            }
            else if (display == "L o c ")
            {
                state = LOCKING;
            }
            else if (display == "A 5 t ")
            {
                state = RESETTING;
            }
            else if (display[0] == 'E' || display == "H 0 t ")
            {
                state = ALARMING_ERROR;
            }
            else if (display[0] == '5' && (display[2] == ' ' || display[2] == '-'))
            {
                state = SETTING_MEM;
            }
            else if (std::regex_match(display, setting_alarm_regex))
            {
                state = SETTING_ALARM;
            }
            else
            {
                ESP_LOGW("UART RX DECODER", "Decode failed: %s", display);
            }
            memcpy(buf_last, buf, 5);
            is_data_updated = false;
        }

        float RxDecoder::get_desk_height()
        {
            return desk_height;
        }

        std::string RxDecoder::get_desk_state()
        {
            switch (state)
            {
            case DISPLAYING_HEIGHT:
                return "正常";
            case SETTING_MEM:
                return "设置记忆中";
            case SETTING_ALARM:
                return "设置闹钟中";
            case LOCKING:
                return "锁定";
            case RESETTING:
                return "复位中";
            case ALARMING_ERROR:
                return "报错";
            }
            return "未知";
        }

        std::string RxDecoder::get_desk_display()
        {
            std::string result;
            for (auto ch : display)
            {
                if (!std::isspace(ch))
                {
                    result += ch;
                }
            }
            return result;
        }
        bool RxDecoder::is_updated()
        {
            return is_data_updated;
        }
        void RxDecoder::set_updated()
        {
            is_data_updated = true;
        }
    }
}
