#include "decoder.h"

std::unordered_map<uint8_t, char> segmentMap = {
    {0b00111111, '0'}, // 0
    {0b00000110, '1'}, // 1
    {0b01011011, '2'}, // 2
    {0b01001111, '3'}, // 3
    {0b01100110, '4'}, // 4
    {0b01101101, '5'}, // 5,S
    {0b01111101, '6'}, // 6
    {0b00000111, '7'}, // 7
    {0b01111111, '8'}, // 8
    {0b01101111, '9'}, // 9
    {0b01110111, 'A'}, // A,R
    {0b01111100, 'b'}, // b
    {0b00111001, 'C'}, // C
    {0b01011000, 'c'}, // c
    {0b01011110, 'd'}, // d
    {0b01111001, 'E'}, // E
    {0b01111011, 'e'}, // e
    {0b01110001, 'F'}, // F
    {0b01110110, 'H'}, // H
    {0b01110100, 'h'}, // h
    {0b00110000, 'I'}, // I(left)
    {0b00001110, 'J'}, // J
    {0b00111000, 'L'}, // L
    {0b01010100, 'n'}, // n
    {0b01011100, 'o'}, // o
    {0b01110011, 'P'}, // P,p
    {0b01100111, 'q'}, // q
    {0b01010000, 'r'}, // r
    {0b01111000, 't'}, // t
    {0b00111110, 'U'}, // U
    {0b00011100, 'u'}, // u
    {0b01101110, 'y'}, // y
    {0b01000000, '-'}, // -
    {0b01001000, '='}, // =
    {0b00000000, ' '}, // Blank

};

namespace esphome
{
    namespace standing_desk_height
    {
        bool Decoder::put(uint8_t b)
        {
            bool flag = false;
            switch (data_state)
            {
            case SYNC:
                if (b == 0xA5)
                    data_state = SCREEN1;
                else
                    data_state = SYNC;
            case SCREEN1:
                buf[0] = b;
            case SCREEN2:
                buf[1] = b;
            case SCREEN3:
                buf[2] = b;
            case CHECKSUM:
                uint8_t computed_checksum = (buf[0] + buf[1] + buf[2]);
                if (computed_checksum == b)
                    flag = true;
                else
                {
                    data_state = SYNC;
                }
            }
            return flag;
        }

        std::string Decoder::decode()
        {
            std::string result = "";

            for (size_t i = 0; i < 3; i++)
            {
                uint8_t segment = buf[i];

                bool hasDot = segment & 0b10000000;
                segment &= 0b01111111;

                if (segmentMap.find(segment) != segmentMap.end())
                {
                    result += segmentMap[segment];
                    if (hasDot)
                    {
                        result += '.';
                    }
                    else
                    {
                        result += ' ';
                    }
                }
                else
                {
                    result += '?';
                }
            }
            display = result;
            return result;
        }

        state_t Decoder::get_state()
        {
            state_t current_state;
            switch (true)
            {
            case display == "5 -   ":
            case display == "5     ":
                return SETTING_MEM;
            case display == "L o c ":
                return LOCKING;
            case std::regex_match(display, regex_pattern("^[0-9\\.\\s]{4}h\\s$")):
                return SETTING_ALARM;
            case display == "A 5 t ":
                return WAITING_RESET;
            case display == "E 0 4 ":
            case display == "E 0 5 ":
            case display == "H 0 t ":
            case display == "E 1 1 ":
            case display == "E 2 1 ":
                return ALARMING_ERROR;
            default:
                return DISPLAYING;
            }
        }

        float Decoder::get_height()
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

            try
            {
                return std::stof(result); // 将合法字符串转换为浮点数
            }
            catch (...)
            {
                return -1.0f; // 转换失败，不合法
            }
        }
    }
}