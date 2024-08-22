#pragma once

#include <stdint.h>
#include <string>
#include <unordered_map>
#include <cctype>
#include <cstdlib>
#include <regex>

namespace esphome
{
    namespace standing_desk_height
    {
        class Decoder
        {
        protected:
            typedef enum
            {
                SYNC,
                SCREEN1,
                SCREEN2,
                SCREEN3,
                CHECKSUM,
            } data_state_t;

            typedef enum
            {
                DISPLAYING,
                SETTING_MEM,
                SETTING_ALARM,
                LOCKING,
                WAITING_RESET,
                ALARMING_ERROR,
            } state_t;
            data_state_t data_state = SYNC;
            uint8_t buf[3];
            // 日.日.日.
            std::string display;

            state_t state = DISPLAYING;

        public:
            Decoder() {}
            ~Decoder() {}

            bool put(uint8_t b);
            std::string decode();
            float get_height();
            state_t get_state();
        };
    }
}