#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include <cmath>
#include <stddef.h>
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
            void set_move_stop_margin_base(float v)
            {
                move_stop_margin_base = v;
            }
            void set_move_stop_margin_per_cm(float v)
            {
                move_stop_margin_per_cm = v;
            }
            void set_move_stop_margin_min(float v)
            {
                move_stop_margin_min = v;
            }
            void set_move_stop_margin_max(float v)
            {
                move_stop_margin_max = v;
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
            void set_manual_move_debug(bool v)
            {
                manual_move_debug = v;
            }
            void set_manual_move_debug_dump_frames(bool v)
            {
                manual_move_debug_dump_frames = v;
            }

        protected:
            typedef enum
            {
                MOVE_IDLE,
                MOVE_UP,
                MOVE_DOWN,
            } move_state_t;
            typedef enum
            {
                MANUAL_MOVE_NONE,
                MANUAL_MOVE_UP,
                MANUAL_MOVE_DOWN,
                MANUAL_MOVE_MIXED,
            } manual_move_direction_t;
            struct DebugFrame
            {
                uint32_t offset_ms;
                uint8_t bytes[5];
            };

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
            float move_stop_margin_base = 0.3f;
            float move_stop_margin_per_cm = 0.36f;
            float move_stop_margin_min = 0.45f;
            float move_stop_margin_max = 1.2f;
            int move_command_repeat = 4;
            uint32_t move_command_interval_ms = 80;
            uint32_t move_timeout_ms = 30000;
            uint32_t move_stall_timeout_ms = 3000;
            float move_stall_tolerance = 0.2f;
            uint32_t move_started_ms = 0;
            float move_start_height = NAN;
            float last_move_stop_margin = NAN;
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
            bool manual_move_debug = false;
            bool manual_move_debug_dump_frames = true;
            bool manual_move_active = false;
            manual_move_direction_t manual_move_direction = MANUAL_MOVE_NONE;
            uint32_t manual_move_started_ms = 0;
            uint32_t manual_move_last_direction_ms = 0;
            uint32_t manual_move_last_handset_frame_ms = 0;
            uint32_t manual_move_handset_frames = 0;
            uint32_t manual_move_up_frames = 0;
            uint32_t manual_move_down_frames = 0;
            uint32_t manual_move_normal_frames = 0;
            uint32_t manual_move_other_frames = 0;
            uint32_t manual_move_handset_interval_min_ms = UINT32_MAX;
            uint32_t manual_move_handset_interval_max_ms = 0;
            uint32_t manual_move_handset_interval_sum_ms = 0;
            uint32_t manual_move_handset_interval_count = 0;
            uint32_t manual_move_last_rx_update_ms = 0;
            uint32_t manual_move_rx_updates = 0;
            uint32_t manual_move_rx_interval_min_ms = UINT32_MAX;
            uint32_t manual_move_rx_interval_max_ms = 0;
            uint32_t manual_move_rx_interval_sum_ms = 0;
            uint32_t manual_move_rx_interval_count = 0;
            float manual_move_start_height = NAN;
            float manual_move_end_height = NAN;
            bool manual_move_error_seen = false;
            std::string manual_move_last_display;
            std::string manual_move_last_state;
            bool target_move_debug_active = false;
            manual_move_direction_t target_move_direction = MANUAL_MOVE_NONE;
            uint32_t target_move_started_ms = 0;
            uint32_t target_move_last_tx_frame_ms = 0;
            uint32_t target_move_sent_frames = 0;
            uint32_t target_move_up_frames = 0;
            uint32_t target_move_down_frames = 0;
            uint32_t target_move_normal_frames = 0;
            uint32_t target_move_other_frames = 0;
            uint32_t target_move_injected_frames = 0;
            uint32_t target_move_passthrough_frames = 0;
            uint32_t target_move_tx_interval_min_ms = UINT32_MAX;
            uint32_t target_move_tx_interval_max_ms = 0;
            uint32_t target_move_tx_interval_sum_ms = 0;
            uint32_t target_move_tx_interval_count = 0;
            uint32_t target_move_last_rx_update_ms = 0;
            uint32_t target_move_rx_updates = 0;
            uint32_t target_move_rx_interval_min_ms = UINT32_MAX;
            uint32_t target_move_rx_interval_max_ms = 0;
            uint32_t target_move_rx_interval_sum_ms = 0;
            uint32_t target_move_rx_interval_count = 0;
            float target_move_start_height = NAN;
            float target_move_end_height = NAN;
            float target_move_target_height = NAN;
            float target_move_instant_overshoot = NAN;
            std::string target_move_result;
            bool target_move_error_seen = false;
            std::string target_move_last_display;
            std::string target_move_last_state;
            bool target_move_final_sample_pending = false;
            uint32_t target_move_final_sample_due_ms = 0;
            float target_move_final_sample_target_height = NAN;
            float target_move_final_sample_stop_height = NAN;
            manual_move_direction_t target_move_final_sample_direction = MANUAL_MOVE_NONE;
            std::string target_move_final_sample_result;
            static constexpr size_t MANUAL_DEBUG_RECENT_FRAME_COUNT = 8;
            DebugFrame manual_move_recent_handset_frames[MANUAL_DEBUG_RECENT_FRAME_COUNT];
            DebugFrame manual_move_recent_rx_frames[MANUAL_DEBUG_RECENT_FRAME_COUNT];
            DebugFrame target_move_recent_tx_frames[MANUAL_DEBUG_RECENT_FRAME_COUNT];
            DebugFrame target_move_recent_rx_frames[MANUAL_DEBUG_RECENT_FRAME_COUNT];
            size_t manual_move_recent_handset_next = 0;
            size_t manual_move_recent_handset_count = 0;
            size_t manual_move_recent_rx_next = 0;
            size_t manual_move_recent_rx_count = 0;
            size_t target_move_recent_tx_next = 0;
            size_t target_move_recent_tx_count = 0;
            size_t target_move_recent_rx_next = 0;
            size_t target_move_recent_rx_count = 0;
            static constexpr uint32_t MIN_OFFLINE_TX_INTERVAL_MS = 5;
            static constexpr uint32_t MIN_LEARNED_IDLE_INTERVAL_MS = 5;
            static constexpr uint32_t MAX_LEARNED_IDLE_INTERVAL_MS = 100;
            static constexpr uint32_t MANUAL_MOVE_END_TIMEOUT_MS = 150;
            static constexpr uint32_t TARGET_MOVE_FINAL_SAMPLE_DELAY_MS = 800;

            uint32_t get_offline_tx_interval_ms_() const;
            void observe_handset_frame_(const uint8_t *buf, uint32_t now);
            void observe_manual_handset_frame_(const uint8_t *buf, uint32_t now);
            void observe_manual_rx_update_(const uint8_t *buf, uint32_t now);
            void maybe_finish_manual_move_(uint32_t now);
            void finish_manual_move_(uint32_t now, const char *reason);
            void reset_manual_move_(uint32_t now, manual_move_direction_t direction);
            void reset_target_move_debug_(uint32_t now);
            void observe_target_tx_frame_(const uint8_t *buf, uint32_t now, bool injected);
            void observe_target_rx_update_(const uint8_t *buf, uint32_t now);
            void finish_target_move_debug_(uint32_t now, const std::string &result);
            void maybe_log_target_move_final_sample_(uint32_t now);
            float get_move_stop_margin_() const;
            manual_move_direction_t get_handset_direction_(const uint8_t *buf) const;
            const char *manual_move_direction_to_string_(manual_move_direction_t direction) const;
            void store_debug_frame_(DebugFrame *frames, size_t &next, size_t &count, const uint8_t *buf, uint32_t now, uint32_t started_ms);
            void log_recent_debug_frames_(const char *label, const DebugFrame *frames, size_t next, size_t count) const;
            void write_control_frame_(const uint8_t *buf, uint32_t now, bool injected);
            void process_move_(bool force_command_refill = false);
            void finish_move_(const std::string &result);
            void publish_diagnostics_();
            void sync_decoder_height_range_();

        };

    }
}
