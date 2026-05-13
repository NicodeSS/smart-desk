#pragma once

#include <cctype>
#include <cmath>
#include <cstring>
#include <stdint.h>
#include <string>
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
            typedef enum
            {
                SYNC,
                SCREEN1,
                SCREEN2,
                SCREEN3,
                CHECKSUM,
            } data_state_t;
            data_state_t data_state = SYNC;
            uint8_t buf[5]{};
            uint8_t buf_last[5]{0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

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

            float desk_height = NAN;

            std::string decode();
            bool is_numeric_display_() const;
            bool is_setting_alarm_display_() const;

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
