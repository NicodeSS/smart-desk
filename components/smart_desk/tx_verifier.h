#pragma once

#include <stdint.h>
#include <cctype>
#include <cstdlib>
#include <regex>
#include "esphome/core/log.h"

namespace esphome
{
    namespace nicode_smart_desk
    {
        class TxVerifier
        {
        protected:
            typedef enum
            {
                SYNC,
                UNKNOWN_DATA,
                PUSHED,
                RELEASED,
                CHECKSUM,
            } data_state_t;
            data_state_t data_state = SYNC;
            uint8_t buf[5];

        public:
            TxVerifier() {}
            ~TxVerifier() {}

            bool put(uint8_t b);
            std::string decode();
            const uint8_t *get_buffer() const;
        };
    }
}