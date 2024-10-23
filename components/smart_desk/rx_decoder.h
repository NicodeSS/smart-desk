#pragma once

#include <iostream>
#include <string>
#include <sstream>
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
        extern float max_desk_height;
        extern float min_desk_height;
        extern const char segment_map[128];
        class RxDecoder
        {
        protected:
            std::regex numeric_regex = std::regex("^[0-9]\\s[0-9](\\s|.)[0-9]\\s");
            std::regex setting_alarm_regex = std::regex("^[0-9\\.\\s]{4}h\\s");

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
            uint8_t buf_last[5];

            typedef enum
            {
                DISPLAYING_HEIGHT,
                SETTING_MEM,
                SETTING_ALARM,
                LOCKING,
                RESETTING,
                ALARMING_ERROR,
            } state_t;
            state_t state = DISPLAYING_HEIGHT;

            /** Not Implemented. */
            bool counting_time = false;

            // 日.日.日.
            std::string display;
            bool is_data_updated = false;

            float desk_height;

            std::string decode();

        public:
            RxDecoder() {}
            ~RxDecoder() {}

            bool put(uint8_t b);

            void update_state();
            const uint8_t *get_buffer() const;
            float get_desk_height();
            std::string get_desk_state();
            std::string get_desk_display();
            bool is_updated();
            void set_updated();
        };
    }
}