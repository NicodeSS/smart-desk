#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include <cmath>
#include <stdint.h>
#include <string>

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
            void set_sensor_target_height(sensor::Sensor *sensor_)
            {
                this->sensor_target_height = sensor_;
            }
            void set_sensor_learned_idle_interval(sensor::Sensor *sensor_)
            {
                this->sensor_learned_idle_interval = sensor_;
            }
            void set_text_sensor_status(text_sensor::TextSensor *text_sensor_)
            {
                this->text_sensor_status = text_sensor_;
            }
            void set_text_sensor_display(text_sensor::TextSensor *text_sensor_)
            {
                this->text_sensor_display = text_sensor_;
            }
            void set_text_sensor_movement(text_sensor::TextSensor *text_sensor_)
            {
                this->text_sensor_movement = text_sensor_;
            }
            void set_text_sensor_last_move_result(text_sensor::TextSensor *text_sensor_)
            {
                this->text_sensor_last_move_result = text_sensor_;
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
            float get_current_height() const
            {
                return current_height;
            }
            float get_min_height() const
            {
                return min_desk_height;
            }
            float get_max_height() const
            {
                return max_desk_height;
            }
            float height_to_position(float height) const
            {
                const float range = max_desk_height - min_desk_height;
                if (std::isnan(height) || range <= 0.0f)
                    return NAN;
                float position = (height - min_desk_height) / range;
                if (position < 0.0f)
                    return 0.0f;
                if (position > 1.0f)
                    return 1.0f;
                return position;
            }
            float position_to_height(float position) const
            {
                if (position < 0.0f)
                    position = 0.0f;
                if (position > 1.0f)
                    position = 1.0f;
                return min_desk_height + position * (max_desk_height - min_desk_height);
            }
            void clear_commands()
            {
                if (tx_controller != nullptr)
                    tx_controller->clear();
            }
            bool start_move_to_height(float target_height);
            bool start_move_to_position(float position)
            {
                return start_move_to_height(position_to_height(position));
            }
            void stop_moving();
            bool is_moving() const
            {
                return move_state != MOVE_IDLE;
            }
            int get_move_direction() const
            {
                if (move_state == MOVE_UP)
                    return 1;
                if (move_state == MOVE_DOWN)
                    return -1;
                return 0;
            }
            float get_target_height() const
            {
                return target_height;
            }
            uint32_t get_learned_idle_interval_ms() const
            {
                return learned_handset_idle_interval_ms;
            }
            std::string get_movement_state() const;
            std::string get_last_move_result() const
            {
                return last_move_result;
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
                this->sync_decoder_height_range_();
            }

            void set_max_height(float v)
            {
                max_desk_height = v;
                this->sync_decoder_height_range_();
            }

            void set_offline_tx_interval_ms(uint32_t v)
            {
                offline_tx_interval_ms = v < MIN_OFFLINE_TX_INTERVAL_MS ? MIN_OFFLINE_TX_INTERVAL_MS : v;
            }
            void set_move_tolerance(float v)
            {
                move_tolerance = v;
            }
            void set_move_command_repeat(int v)
            {
                move_command_repeat = v;
            }
            void set_move_command_interval_ms(uint32_t v)
            {
                move_command_interval_ms = v;
            }
            void set_move_timeout_ms(uint32_t v)
            {
                move_timeout_ms = v;
            }
            void set_move_stall_timeout_ms(uint32_t v)
            {
                move_stall_timeout_ms = v;
            }
            void set_move_stall_tolerance(float v)
            {
                move_stall_tolerance = v;
            }

        protected:
            typedef enum
            {
                MOVE_IDLE,
                MOVE_UP,
                MOVE_DOWN,
            } move_state_t;

            sensor::Sensor *sensor_height{nullptr};
            sensor::Sensor *sensor_target_height{nullptr};
            sensor::Sensor *sensor_learned_idle_interval{nullptr};
            text_sensor::TextSensor *text_sensor_status{nullptr};
            text_sensor::TextSensor *text_sensor_display{nullptr};
            text_sensor::TextSensor *text_sensor_movement{nullptr};
            text_sensor::TextSensor *text_sensor_last_move_result{nullptr};
            binary_sensor::BinarySensor *binary_sensor_handset_online{nullptr};

            RxDecoder *rx_decoder{nullptr};
            TxVerifier *tx_verifier{nullptr};
            TxController *tx_controller{nullptr};

            uart::UARTComponent *uart_control{nullptr};
            uart::UARTComponent *uart_handset{nullptr};

            float current_height = NAN;
            float target_height = NAN;
            float min_desk_height = 62.0f;
            float max_desk_height = 127.0f;
            int default_tx_command_repeat = 5;
            move_state_t move_state = MOVE_IDLE;
            float move_tolerance = 0.4f;
            int move_command_repeat = 4;
            uint32_t move_command_interval_ms = 80;
            uint32_t move_timeout_ms = 30000;
            uint32_t move_stall_timeout_ms = 3000;
            float move_stall_tolerance = 0.2f;
            uint32_t move_started_ms = 0;
            uint32_t last_move_command_ms = 0;
            uint32_t last_move_progress_ms = 0;
            float last_move_progress_height = NAN;
            std::string last_move_result = "idle";

            bool is_handset_online = false;
            bool is_initial_command_sent = false;
            int handset_timeout_count = 0;
            int max_handset_timeout_count = 5000;
            uint32_t last_offline_tx_ms = 0;
            uint32_t offline_tx_interval_ms = 20;
            uint32_t last_handset_idle_frame_ms = 0;
            uint32_t learned_handset_idle_interval_ms = 0;
            bool has_logged_learned_interval = false;
            static constexpr uint32_t MIN_OFFLINE_TX_INTERVAL_MS = 5;
            static constexpr uint32_t MIN_LEARNED_IDLE_INTERVAL_MS = 5;
            static constexpr uint32_t MAX_LEARNED_IDLE_INTERVAL_MS = 100;

            uint32_t get_offline_tx_interval_ms_() const;
            void observe_handset_frame_(const uint8_t *buf, uint32_t now);
            void process_move_();
            void finish_move_(const std::string &result);
            void publish_diagnostics_();
            void sync_decoder_height_range_();

        };

    }
}
