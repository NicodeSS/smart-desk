#include "smart_desk.h"
#include "esphome/core/log.h"

namespace esphome
{
    namespace nicode_smart_desk
    {

        static const char *const TAG = "smart_desk";

        void SmartDesk::setup()
        {
            ESP_LOGW(TAG, "Smart Desk setup");
            if (rx_decoder == nullptr)
            {
                rx_decoder = new RxDecoder();
            }
            if (tx_verifier == nullptr)
            {
                tx_verifier = new TxVerifier();
            }
            if (tx_controller == nullptr)
            {
                tx_controller = new TxController();
            }
        }

        void SmartDesk::write_initial_command()
        {
            const uint8_t initial_command_control[5] = {0xA5, 0x00, 0x00, 0xFF, 0xFF};
            uart_control->write_array(initial_command_control, 5);
            const uint8_t initial_command_handset[5] = {0x5A, 0x3F, 0x3F, 0x3F, 0xBD};

            initial_command_sent = true;
            ESP_LOGW(TAG, "Initial command sent");
        }

        void SmartDesk::loop()
        {
            if (!initial_command_sent)
            {
                write_initial_command();
            }
            uint8_t uart_control_c, uart_handset_c;
            // ESP_LOGD(TAG, "Available Bytes: Handset: %d, Controller: %d", uart_handset->available(), uart_control->available());

            if (uart_handset->available() > 0)
            {
                while (uart_handset->available() > 0)
                {
                    uart_handset->read_byte(&uart_handset_c);
                    // ESP_LOGW(TAG, "Handset --> ControlBox: %X", uart_handset_c);

                    if (tx_verifier != nullptr && tx_verifier->put(uart_handset_c))
                    {
                        if (!tx_controller->is_empty())
                        {
                            const TxCommand *tx_c = tx_controller->pop();
                            // ESP_LOGD(TAG, "Replaced command: %X %X %X %X %X", tx_c->command[0], tx_c->command[1], tx_c->command[2], tx_c->command[3], tx_c->command[4]);
                            uart_control->write_array(tx_c->command, 5);
                        }
                        else
                        {
                            const uint8_t *buf = tx_verifier->get_buffer();
                            // ESP_LOGD(TAG, "Handset --> ControlBox : %X %X %X %X %X", buf[0], buf[1], buf[2], buf[3], buf[4]);
                            uart_control->write_array(buf, 5);
                        }

                        break;
                    }
                }
            }
            else
            {
                if (!tx_controller->is_empty())
                {
                    const TxCommand *tx_c = tx_controller->pop();
                    uart_control->write_array(tx_c->command, 5);
                }
                else
                {
                    uart_control->write_array(command_handset_normal, 5);
                }
            }

            while (uart_control->available() > 0)
            {
                uart_control->read_byte(&uart_control_c);
                // ESP_LOGW(TAG, "Controlbox --> Headset: %X", uart_control_c);
                if (rx_decoder != nullptr && rx_decoder->put(uart_control_c))
                {
                    const uint8_t *buf = rx_decoder->get_buffer();
                    // ESP_LOGD(TAG, "Controlbox --> Headset: %X %X %X %X %X", buf[0], buf[1], buf[2], buf[3], buf[4]);
                    const char *display = rx_decoder->decode();
                    uart_handset->write_array(buf, 5);
                    break;
                }
            }
        }

        void SmartDesk::dump_config() { ESP_LOGCONFIG(TAG, "smart_desk dump_config"); }

        bool SmartDesk::add_command(const char *button_chars, size_t len, int repeat)
        {
            if (tx_controller->is_full())
            {
                return false;
            }

            uint8_t pressed = 0;

            for (int i = 0; i < len; i++)
            {
                switch (button_chars[i])
                {
                case '1':
                {
                    pressed |= DESK_MEM1;
                    break;
                }
                case '2':
                {
                    pressed |= DESK_MEM2;
                    break;
                }
                case '3':
                {
                    pressed |= DESK_MEM3;
                    break;
                }
                case '4':
                {
                    pressed |= DESK_MEM4;
                    break;
                }
                case 'M':
                {
                    pressed |= DESK_MEMSET;
                    break;
                }
                case 'U':
                {
                    pressed |= DESK_UP;
                    break;
                }
                case 'D':
                {
                    pressed |= DESK_DOWN;
                    break;
                }
                case 'P':
                {
                    pressed |= DESK_AWAKE;
                    break;
                }
                }
            }

            const uint8_t command[5] = {
                0xA5,
                0x00,
                pressed,
                (uint8_t)~pressed,
                0xFF,
            };
            return tx_controller->enqueue(command, repeat);
        }
    }
}