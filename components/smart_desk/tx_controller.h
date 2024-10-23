#pragma once

#include <stdint.h>
#include <cstring>
#include "esphome/core/log.h"

namespace esphome
{
    namespace nicode_smart_desk
    {
        extern int default_tx_command_repeat;
        constexpr size_t TX_COMMAND_QUEUE_SIZE = 16;
        struct TxCommand
        {
            int repeat;
            uint8_t command[5];
        };
        class TxController
        {
        protected:
            TxCommand queue[TX_COMMAND_QUEUE_SIZE];
            TxCommand *head = queue, *tail = queue;
            int count = 0;

        public:
            TxController() {};
            ~TxController() {};

            bool enqueue(const uint8_t *command, int repeat);
            bool enqueue(const uint8_t *command)
            {
                return enqueue(command, default_tx_command_repeat);
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