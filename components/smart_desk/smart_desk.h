#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/binary_sensor/binary_sensor.h"

#include "rx_decoder.h"
#include "tx_verifier.h"
#include "tx_controller.h"

namespace esphome
{
    namespace nicode_smart_desk
    {
        extern float max_desk_height;
        extern float min_desk_height;
        extern int default_tx_command_repeat;

        typedef enum
        {
            DESK_MEMSET = 0x01,
            DESK_MEM1 = 0x02,
            DESK_MEM2 = 0x04,
            DESK_MEM3 = 0x08,
            DESK_MEM4 = 0x10,
            DESK_UP = 0x20,
            DESK_DOWN = 0x40,
            DESK_AWAKE = 0x80,
        } desk_button;

        const uint8_t command_handset_normal[5] = {0xA5, 0x00, 0x00, 0xFF, 0xFF};
        class SmartDesk : public Component
        {

        public:
            float get_setup_priority() const override { return setup_priority::DATA; }
            void setup() override;
            void loop() override;
            void dump_config() override;

            void set_uart_control(uart::UARTComponent *uart) { this->uart_control = uart; }
            void set_uart_handset(uart::UARTComponent *uart) { this->uart_handset = uart; }
            void set_sensor_height(sensor::Sensor *sensor_)
            {
                this->sensor_height = sensor_;
            }
            void set_text_sensor_status(text_sensor::TextSensor *text_sensor_)
            {
                this->text_sensor_status = text_sensor_;
            }
            void set_text_sensor_display(text_sensor::TextSensor *text_sensor_)
            {
                this->text_sensor_display = text_sensor_;
            }
            void set_binary_sensor_handset_online(binary_sensor::BinarySensor *binary_sensor_)
            {
                this->binary_sensor_handset_online = binary_sensor_;
            }

            bool add_command(std::string button_chars, int repeat);
            bool add_command(std::string button_chars)
            {
                return add_command(button_chars, default_tx_command_repeat);
            }
            bool get_handset_state()
            {
                return is_handset_online;
            }

            void set_max_handset_timeout_count(int v)
            {
                max_handset_timeout_count = v;
            }

            void set_default_command_repeat(int v)
            {
                default_tx_command_repeat = v;
            }

            void set_min_height(float v)
            {
                min_desk_height = v;
            }

            void set_max_height(float v)
            {
                max_desk_height = v;
            }

        protected:
            sensor::Sensor *sensor_height{nullptr};
            text_sensor::TextSensor *text_sensor_status{nullptr};
            text_sensor::TextSensor *text_sensor_display{nullptr};
            binary_sensor::BinarySensor *binary_sensor_handset_online{nullptr};

            RxDecoder *rx_decoder{nullptr};
            TxVerifier *tx_verifier{nullptr};
            TxController *tx_controller{nullptr};

            uart::UARTComponent *uart_control{nullptr};
            uart::UARTComponent *uart_handset{nullptr};

            bool is_handset_online = false;
            bool is_initial_command_sent = false;
            int handset_timeout_count = 0;
            int max_handset_timeout_count = 5000;
        };

    }
}