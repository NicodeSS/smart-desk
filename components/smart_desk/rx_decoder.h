#pragma once

#include <stdint.h>
#include <string>
#include <unordered_map>
#include <cctype>
#include <cstdlib>
#include <regex>
#include "esphome/core/log.h"
namespace esphome
{
    namespace nicode_smart_desk
    {
        extern const char segment_map[128];
        class RxDecoder
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
            data_state_t data_state = SYNC;
            uint8_t buf[5];

            typedef enum
            {
                DISPLAYING,
                SETTING_MEM,
                SETTING_ALARM,
                LOCKING,
                WAITING_RESET,
                ALARMING_ERROR,
            } state_t;
            state_t state = DISPLAYING;

            // 日.日.日.
            std::string display;

            char display_bit[8];

        public:
            RxDecoder() {}
            ~RxDecoder() {}

            bool put(uint8_t b);
            char *decode();
            float get_height();
            state_t get_state();
            const uint8_t *get_buffer() const;
        };
    }
}