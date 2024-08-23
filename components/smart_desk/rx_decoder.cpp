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

        char *RxDecoder::decode()
        {
            for (size_t i = 1; i < 4; i++)
            {
                uint8_t segment = buf[i];

                bool has_dot = segment & 0x80;
                segment &= 0x7F;

                display_bit[2 * (i - 1)] = esphome::nicode_smart_desk::segment_map[segment];
                display_bit[2 * (i - 1) + 1] = has_dot ? '.' : ' ';

                // ESP_LOGW("UART RX DECODER", "Current data: %X, hasDot %d, remove dot: %X", buf[i], hasDot, segment);

                // if (segmentMap.find(segment) != segmentMap.end())
                // {
                //     display_bit[(i - 1) * 2] = segmentMap[segment];
                // }
                // else
                // {
                //     display_bit[(i - 1) * 2] = '?';
                // }
                // if (has_dot)
                // {
                //     display_bit[(i - 1) * 2 + 1] = '.';
                // }
                // else
                // {
                //     display_bit[(i - 1) * 2 + 1] = ' ';
                // }
            }
            display_bit[7] = '\0';
            return display_bit;
        }

        RxDecoder::state_t RxDecoder::get_state()
        {
            state_t current_state;
            std::regex pattern("^[0-9\\.\\s]{4}h\\s$");
            if (display == "5 -   " || display == "5     ")
                return SETTING_MEM;
            else if (display == "L o c ")
                return LOCKING;
            else if (std::regex_match(display, pattern))
                return SETTING_ALARM;
            else if (display == "A 5 t ")
                return WAITING_RESET;
            else if (display == "E 0 4 " || display == "E 0 5 " || display == "H 0 t " || display == "E 1 1 " || display == "E 2 1 ")
                return ALARMING_ERROR;
            else
                return DISPLAYING;
        }

        float RxDecoder::get_height()
        {
            std::string result = "";
            bool decimalPointFound = false;

            for (char c : display)
            {
                if (std::isdigit(c))
                {
                    result += c;
                }
                else if (c == '.')
                {
                    if (decimalPointFound)
                    {
                        return -1.0f; // 多个小数点，不合法
                    }
                    decimalPointFound = true;
                    result += c;
                }
                else if (c == ' ')
                {
                    continue; // 忽略空格
                }
                else
                {
                    return -1.0f; // 非法字符
                }
            }

            if (result.empty() || result == "." || result.front() == '.' || result.back() == '.')
            {
                return -1.0f; // 空字符串或仅包含小数点的字符串，不合法
            }

            return std::stof(result);
        }
    }
}