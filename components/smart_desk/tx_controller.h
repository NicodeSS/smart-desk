#pragma once

#include <stdint.h>
#include <cstring>
#include "esphome/core/log.h"

namespace esphome
{
    namespace nicode_smart_desk
    {
        constexpr size_t TX_COMMAND_QUEUE_SIZE = 16;
        constexpr int TX_COMMAND_DEFAULT_REPEAT = 8;
        struct TxCommand
        {
            int repeat;
            uint8_t command[5];
        };
        class TxController
        {
        protected:
            TxCommand queue[16];
            TxCommand *head = queue, *tail = queue;
            int count = 0;

            static const int default_repeat = 5;

        public:
            TxController() {};
            ~TxController() {};

            bool enqueue(const uint8_t *command, int repeat);
            bool enqueue(const uint8_t *command)
            {
                return enqueue(command, TX_COMMAND_DEFAULT_REPEAT);
            }
            const TxCommand *pop();

            bool is_empty()
            {
                return count == 0;
            }

            bool is_full()
            {
                return count == TX_COMMAND_QUEUE_SIZE;
            }
        };
    };
}