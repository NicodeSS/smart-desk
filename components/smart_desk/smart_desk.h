#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"

#include "rx_decoder.h"
#include "tx_verifier.h"
#include "tx_controller.h"

namespace esphome
{
    namespace nicode_smart_desk
    {
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
        class SmartDesk : public Component
        {
        public:
            float get_setup_priority() const override { return setup_priority::LATE; }
            void setup() override;
            void loop() override;
            void dump_config() override;
            void set_uart_control(uart::UARTComponent *uart) { this->uart_control = uart; }
            void set_uart_handset(uart::UARTComponent *uart) { this->uart_handset = uart; }

            void write_initial_command();

            bool add_command(const char *button_chars, size_t len, int repeat);
            bool add_command(const char *button_chars, size_t len)
            {
                return add_command(button_chars, len, TX_COMMAND_DEFAULT_REPEAT);
            }

        protected:
            RxDecoder *rx_decoder;
            TxVerifier *tx_verifier;
            TxController *tx_controller;

            uart::UARTComponent *uart_control;
            uart::UARTComponent *uart_handset;

            bool initial_command_sent = false;
        };

    }
}