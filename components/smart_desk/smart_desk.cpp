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
            if (binary_sensor_handset_online != nullptr)
            {
                binary_sensor_handset_online->publish_initial_state(false);
            }
        }

        void SmartDesk::loop()
        {
            uint8_t uart_control_c, uart_handset_c;
            // ESP_LOGD(TAG, "Available Bytes: Handset: %d, Controller: %d", uart_handset->available(), uart_control->available());

            if (!is_initial_command_sent)
            {
                add_command("P", 1);
                is_initial_command_sent = true;
            }

            if (uart_handset->available() > 0)
            {
                handset_timeout_count = 0;
                if (is_handset_online == false)
                {
                    is_handset_online = true;
                    if (binary_sensor_handset_online != nullptr)
                    {
                        binary_sensor_handset_online->publish_state(true);
                    }
                    ESP_LOGW(TAG, "Handset goes online");
                }
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
            else if (!is_handset_online)
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
            else
            {
                if (handset_timeout_count < max_handset_timeout_count)
                {
                    handset_timeout_count++;
                }
                else if (is_handset_online)
                {
                    is_handset_online = false;
                    ESP_LOGW(TAG, "Handset goes offline.");
                    if (binary_sensor_handset_online != nullptr)
                    {
                        binary_sensor_handset_online->publish_state(false);
                    }
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
                    uart_handset->write_array(buf, 5);
                    rx_decoder->update_state();
                    if (!rx_decoder->is_updated())
                    {
                        if (sensor_height != nullptr)
                            sensor_height->publish_state(rx_decoder->get_desk_height());
                        if (text_sensor_status != nullptr)
                            text_sensor_status->publish_state(rx_decoder->get_desk_state());
                        if (text_sensor_display != nullptr)
                            text_sensor_display->publish_state(rx_decoder->get_desk_display());
                        rx_decoder->set_updated();
                    }
                    break;
                }
            }
        }

        void SmartDesk::dump_config()
        {
            ESP_LOGCONFIG(TAG, "dump_config");
            ESP_LOGCONFIG(TAG, "Max handset timeout count: %d", max_handset_timeout_count);
            ESP_LOGCONFIG(TAG, "Default command repeat: %d", default_tx_command_repeat);
        }

        bool SmartDesk::add_command(std::string button_chars, int repeat)
        {
            if (tx_controller->is_full())
            {
                return false;
            }

            uint8_t pressed = 0;

            for (auto button_char : button_chars)
            {
                switch (button_char)
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