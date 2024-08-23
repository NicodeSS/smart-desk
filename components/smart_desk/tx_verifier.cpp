#include "tx_verifier.h"
namespace esphome
{
    namespace nicode_smart_desk
    {
        bool TxVerifier::put(uint8_t b)
        {
            bool flag = false;
            switch (data_state)
            {
            case SYNC:
                if (b == 0xA5)
                {
                    buf[0] = b;
                    data_state = UNKNOWN_DATA;
                    break;
                }
                else
                {
                    data_state = SYNC;
                    break;
                }
            case UNKNOWN_DATA:
            {
                buf[1] = b;
                data_state = PUSHED;
                break;
            }
            case PUSHED:
            {
                buf[2] = b;
                data_state = RELEASED;
                break;
            }
            case RELEASED:
            {
                buf[3] = b;
                data_state = CHECKSUM;
                break;
            }
            case CHECKSUM:
            {
                buf[4] = b;
                uint8_t computed_checksum = (buf[1] + buf[2] + buf[3]);
                if (computed_checksum == b)
                {
                    flag = true;
                }
                // else
                // {
                //     ESP_LOGW("UART TX VERIFIER", "Headset --> Controlbox [Mismatched]: %X %X %X %X %Xvs%X", buf[0], buf[1], buf[2], buf[3], buf[4], computed_checksum);
                // }
                data_state = SYNC;
                break;
            }
            }
            return flag;
        }

        const uint8_t *TxVerifier::get_buffer() const
        {
            return buf;
        }
    }
}