#include "tx_controller.h"

namespace esphome
{
    namespace nicode_smart_desk
    {
        bool TxController::enqueue(const uint8_t *command, int repeat)
        {
            if (count == TX_COMMAND_QUEUE_SIZE)
            {
                return false;
            }

            memcpy(tail->command, command, 5 * sizeof(uint8_t));
            tail->repeat = repeat;
            tail++;
            if (tail == &queue[TX_COMMAND_QUEUE_SIZE])
            {
                tail = queue; // 环绕
            }
            count++;
            return true;
        }

        const TxCommand *TxController::pop()
        {
            const TxCommand *result;
            if (count == 0)
            {
                // 队列为空
                return nullptr;
            }

            result = head;
            --head->repeat;

            if (head->repeat == 0)
            {
                head++;
                if (head == queue + TX_COMMAND_QUEUE_SIZE)
                {
                    head = queue; // 环绕
                }
                count--;
            }
            return result;
        }

    }
}