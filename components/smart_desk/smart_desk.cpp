#include "smart_desk.h"
#include "esphome/core/hal.h"
#include "esphome/core/log.h"
#include <inttypes.h>
#include <cstring>

namespace esphome
{
    namespace nicode_smart_desk
    {

        static const char *const TAG = "smart_desk";

        void SmartDesk::setup()
        {
            ESP_LOGW(TAG, "Smart Desk setup");
            if (uart_control == nullptr || uart_handset == nullptr)
            {
                ESP_LOGE(TAG, "Both control and handset UARTs must be configured");
                this->mark_failed();
                return;
            }
            if (rx_decoder == nullptr)
            {
                rx_decoder = new RxDecoder();
            }
            this->sync_decoder_height_range_();
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
            if (uart_control == nullptr || uart_handset == nullptr || tx_controller == nullptr ||
                tx_verifier == nullptr || rx_decoder == nullptr)
            {
                return;
            }

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
                        const uint32_t now = millis();
                        observe_handset_frame_(tx_verifier->get_buffer(), now);
                        observe_manual_handset_frame_(tx_verifier->get_buffer(), now);
                        const bool reset_active = this->is_reset_active_();
                        if (reset_active)
                            process_reset_(true);
                        else
                            process_move_(true);

                        if (!tx_controller->is_empty())
                        {
                            const TxCommand *tx_c = tx_controller->pop();
                            // ESP_LOGD(TAG, "Replaced command: %X %X %X %X %X", tx_c->command[0], tx_c->command[1], tx_c->command[2], tx_c->command[3], tx_c->command[4]);
                            write_control_frame_(tx_c->command, now, true);
                        }
                        else
                        {
                            const uint8_t *buf = tx_verifier->get_buffer();
                            if (reset_active)
                            {
                                write_control_frame_(command_handset_normal, now, true);
                            }
                            else
                            {
                                // ESP_LOGD(TAG, "Handset --> ControlBox : %X %X %X %X %X", buf[0], buf[1], buf[2], buf[3], buf[4]);
                                write_control_frame_(buf, now, false);
                            }
                        }

                        break;
                    }
                }
            }
            else if (!is_handset_online)
            {
                const uint32_t now = millis();
                if (now - last_offline_tx_ms >= get_offline_tx_interval_ms_())
                {
                    last_offline_tx_ms = now;
                    if (this->is_reset_active_())
                        process_reset_(true);
                    else
                        process_move_(true);

                    if (!tx_controller->is_empty())
                    {
                        const TxCommand *tx_c = tx_controller->pop();
                        if (tx_c != nullptr)
                        {
                            write_control_frame_(tx_c->command, now, true);
                        }
                    }
                    else
                    {
                        write_control_frame_(command_handset_normal, now, false);
                    }
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
                    if (!rx_decoder->is_updated())
                    {
                        current_height = rx_decoder->get_desk_height();
                        const uint32_t now = millis();
                        process_button_action_(now);
                        if (sensor_height != nullptr)
                            sensor_height->publish_state(current_height);
                        if (text_sensor_status != nullptr)
                            text_sensor_status->publish_state(rx_decoder->get_desk_state());
                        if (text_sensor_display != nullptr)
                            text_sensor_display->publish_state(rx_decoder->get_desk_display());
                        rx_decoder->set_updated();
                        publish_diagnostics_();
                        observe_manual_rx_update_(buf, now);
                        observe_target_rx_update_(buf, now);
                    }
                    break;
                }
            }
            if (this->is_reset_active_())
                process_reset_();
            else
                process_move_();
            maybe_finish_manual_move_(millis());
            maybe_log_target_move_final_sample_(millis());
            process_button_action_(millis());
        }

        void SmartDesk::dump_config()
        {
            ESP_LOGCONFIG(TAG, "dump_config");
            ESP_LOGCONFIG(TAG, "Max handset timeout count: %d", max_handset_timeout_count);
            ESP_LOGCONFIG(TAG, "Default command repeat: %d", default_tx_command_repeat);
            ESP_LOGCONFIG(TAG, "Height range: %.1f-%.1f cm", min_desk_height, max_desk_height);
            ESP_LOGCONFIG(TAG, "Offline TX interval: %" PRIu32 " ms", offline_tx_interval_ms);
            ESP_LOGCONFIG(TAG, "Learned handset idle interval: %" PRIu32 " ms", learned_handset_idle_interval_ms);
            ESP_LOGCONFIG(TAG, "Move tolerance: %.1f cm", move_tolerance);
            ESP_LOGCONFIG(TAG, "Move stop margin: base %.2f cm + %.2f * traveled cm, clamped %.2f-%.2f cm",
                          move_stop_margin_base, move_stop_margin_per_cm,
                          move_stop_margin_min, move_stop_margin_max);
            ESP_LOGCONFIG(TAG, "Move command repeat: %d", move_command_repeat);
            ESP_LOGCONFIG(TAG, "Move command interval: %" PRIu32 " ms", move_command_interval_ms);
            ESP_LOGCONFIG(TAG, "Move timeout: %" PRIu32 " ms", move_timeout_ms);
            ESP_LOGCONFIG(TAG, "Move stall timeout: %" PRIu32 " ms", move_stall_timeout_ms);
            ESP_LOGCONFIG(TAG, "Move stall tolerance: %.1f cm", move_stall_tolerance);
        }

        std::string SmartDesk::get_movement_state() const
        {
            if (reset_state != RESET_IDLE)
            {
                return reset_state_to_string_(reset_state);
            }
            if (button_action_active)
            {
                if (button_action_direction > 0)
                    return "up";
                if (button_action_direction < 0)
                    return "down";
                return "button";
            }
            switch (move_state)
            {
            case MOVE_UP:
                return "up";
            case MOVE_DOWN:
                return "down";
            case MOVE_IDLE:
            default:
                return "idle";
            }
        }

        uint32_t SmartDesk::get_offline_tx_interval_ms_() const
        {
            return learned_handset_idle_interval_ms != 0 ? learned_handset_idle_interval_ms : offline_tx_interval_ms;
        }

        bool SmartDesk::is_reset_active_() const
        {
            return reset_state != RESET_IDLE;
        }

        const char *SmartDesk::reset_state_to_string_(reset_state_t state) const
        {
            switch (state)
            {
            case RESET_MOVING_TO_MIN:
                return "reset_down_to_min";
            case RESET_RELEASE_BEFORE_HOLD:
                return "reset_release";
            case RESET_HOLDING_FOR_RESET:
                return "reset_holding";
            case RESET_IDLE:
            default:
                return "idle";
            }
        }

        bool SmartDesk::start_reset()
        {
            if (this->is_reset_active_())
            {
                ESP_LOGW(TAG, "Reset is already running");
                return false;
            }

            this->clear_commands();
            if (move_state != MOVE_IDLE)
            {
                finish_target_move_debug_(millis(), "reset_started");
            }
            move_state = MOVE_IDLE;
            target_height = NAN;
            move_started_ms = 0;
            move_start_height = NAN;
            last_move_stop_margin = NAN;
            last_move_command_ms = 0;
            last_move_progress_ms = 0;
            last_move_progress_height = NAN;

            const uint32_t now = millis();
            reset_state = RESET_MOVING_TO_MIN;
            reset_started_ms = now;
            reset_phase_started_ms = now;
            reset_last_command_ms = 0;
            reset_seen_display = false;
            last_move_result = "resetting";
            publish_diagnostics_();

            ESP_LOGW(TAG, "Starting desk reset: drive down to %.1f cm, release for %" PRIu32 "ms, then hold down until reset completes",
                     min_desk_height, reset_release_ms);
            return true;
        }

        void SmartDesk::finish_reset_(const std::string &result)
        {
            if (!this->is_reset_active_())
            {
                return;
            }

            ESP_LOGW(TAG, "Desk reset finished: result=%s state=%s height=%.1f seen_rst=%s",
                     result.c_str(), reset_state_to_string_(reset_state), current_height,
                     reset_seen_display ? "true" : "false");
            reset_state = RESET_IDLE;
            reset_started_ms = 0;
            reset_phase_started_ms = 0;
            reset_last_command_ms = 0;
            reset_seen_display = false;
            last_move_result = result;
            this->clear_commands();
            publish_diagnostics_();
        }

        bool SmartDesk::should_track_button_action_(const std::string &button_chars) const
        {
            for (char button_char : button_chars)
            {
                switch (button_char)
                {
                case '1':
                case '2':
                case '3':
                case '4':
                case 'U':
                case 'D':
                    return true;
                default:
                    break;
                }
            }
            return false;
        }

        void SmartDesk::start_button_action_(const std::string &button_chars)
        {
            const uint32_t now = millis();
            button_action_active = true;
            button_action_command = button_chars;
            button_action_started_ms = now;
            button_action_last_progress_ms = now;
            button_action_last_height = current_height;
            button_action_direction = 0;
            if (button_chars.find('U') != std::string::npos)
                button_action_direction = 1;
            else if (button_chars.find('D') != std::string::npos)
                button_action_direction = -1;
            last_move_result = "button_moving";
            publish_diagnostics_();
        }

        void SmartDesk::process_button_action_(uint32_t now)
        {
            if (!button_action_active)
            {
                return;
            }
            if (this->is_reset_active_() || move_state != MOVE_IDLE)
            {
                this->finish_button_action_("button_cancelled");
                return;
            }

            if (!std::isnan(current_height) && !std::isnan(button_action_last_height))
            {
                const float delta = current_height - button_action_last_height;
                if (std::fabs(delta) >= move_stall_tolerance)
                {
                    button_action_direction = delta > 0.0f ? 1 : -1;
                    button_action_last_height = current_height;
                    button_action_last_progress_ms = now;
                    last_move_result = "button_moving";
                    publish_diagnostics_();
                    return;
                }
            }
            else if (!std::isnan(current_height))
            {
                button_action_last_height = current_height;
                button_action_last_progress_ms = now;
            }

            if (now - button_action_started_ms > button_action_timeout_ms)
            {
                this->finish_button_action_("button_timeout");
                return;
            }
            if (now - button_action_last_progress_ms > button_action_idle_timeout_ms)
            {
                this->finish_button_action_("button_done");
            }
        }

        void SmartDesk::finish_button_action_(const std::string &result)
        {
            if (!button_action_active)
            {
                return;
            }
            button_action_active = false;
            button_action_command.clear();
            button_action_started_ms = 0;
            button_action_last_progress_ms = 0;
            button_action_last_height = NAN;
            button_action_direction = 0;
            last_move_result = result;
            publish_diagnostics_();
        }

        void SmartDesk::process_reset_(bool force_command_refill)
        {
            if (!this->is_reset_active_())
            {
                return;
            }

            const uint32_t now = millis();
            const std::string desk_state = rx_decoder != nullptr ? rx_decoder->get_desk_state() : "";
            const bool height_is_min = !std::isnan(current_height) &&
                                       current_height <= min_desk_height + reset_completion_tolerance;

            if (reset_state == RESET_MOVING_TO_MIN)
            {
                if (now - reset_phase_started_ms > move_timeout_ms)
                {
                    ESP_LOGW(TAG, "Desk reset did not reach minimum height within %" PRIu32 "ms", move_timeout_ms);
                    this->finish_reset_("reset_descent_timeout");
                    return;
                }

                if (height_is_min)
                {
                    reset_state = RESET_RELEASE_BEFORE_HOLD;
                    reset_phase_started_ms = now;
                    reset_last_command_ms = 0;
                    last_move_result = "reset_release";
                    this->clear_commands();
                    publish_diagnostics_();
                    ESP_LOGW(TAG, "Desk reset reached minimum height %.1f cm; releasing down button for %" PRIu32 "ms",
                             current_height, reset_release_ms);
                    return;
                }
            }
            else if (reset_state == RESET_RELEASE_BEFORE_HOLD)
            {
                this->clear_commands();
                if (now - reset_phase_started_ms >= reset_release_ms)
                {
                    reset_state = RESET_HOLDING_FOR_RESET;
                    reset_phase_started_ms = now;
                    reset_last_command_ms = 0;
                    last_move_result = "reset_holding";
                    publish_diagnostics_();
                    ESP_LOGW(TAG, "Desk reset release window complete; holding down until RST completes");
                }
                return;
            }
            else if (reset_state == RESET_HOLDING_FOR_RESET)
            {
                if (now - reset_phase_started_ms > reset_hold_timeout_ms)
                {
                    ESP_LOGW(TAG, "Desk reset did not complete within %" PRIu32 "ms after hold started", reset_hold_timeout_ms);
                    this->finish_reset_("reset_timeout");
                    return;
                }

                if (desk_state == "复位中")
                {
                    reset_seen_display = true;
                }
                if (reset_seen_display && desk_state == "正常" && height_is_min)
                {
                    this->finish_reset_("reset_done");
                    return;
                }
            }

            if (!tx_controller->is_empty() ||
                (!force_command_refill && reset_last_command_ms != 0 && now - reset_last_command_ms < move_command_interval_ms))
            {
                return;
            }

            if (enqueue_command_("D", move_command_repeat))
            {
                reset_last_command_ms = now;
            }
        }

        void SmartDesk::observe_handset_frame_(const uint8_t *buf, uint32_t now)
        {
            if (buf == nullptr || memcmp(buf, command_handset_normal, 5) != 0)
            {
                return;
            }

            if (last_handset_idle_frame_ms != 0)
            {
                const uint32_t interval = now - last_handset_idle_frame_ms;
                if (interval >= MIN_LEARNED_IDLE_INTERVAL_MS && interval <= MAX_LEARNED_IDLE_INTERVAL_MS)
                {
                    if (learned_handset_idle_interval_ms == 0)
                    {
                        learned_handset_idle_interval_ms = interval;
                    }
                    else
                    {
                        learned_handset_idle_interval_ms = (learned_handset_idle_interval_ms * 3 + interval) / 4;
                    }

                    if (!has_logged_learned_interval)
                    {
                        ESP_LOGW(TAG, "Learned handset idle interval: %" PRIu32 " ms", learned_handset_idle_interval_ms);
                        has_logged_learned_interval = true;
                    }
                }
            }
            last_handset_idle_frame_ms = now;
        }

        void SmartDesk::observe_manual_handset_frame_(const uint8_t *buf, uint32_t now)
        {
            if (!manual_move_debug || buf == nullptr)
            {
                return;
            }

            const manual_move_direction_t direction = get_handset_direction_(buf);
            if (!manual_move_active)
            {
                if (direction != MANUAL_MOVE_UP && direction != MANUAL_MOVE_DOWN && direction != MANUAL_MOVE_MIXED)
                {
                    return;
                }
                reset_manual_move_(now, direction);
            }

            store_debug_frame_(manual_move_recent_handset_frames, manual_move_recent_handset_next,
                               manual_move_recent_handset_count, buf, now, manual_move_started_ms);

            if (manual_move_last_handset_frame_ms != 0)
            {
                const uint32_t interval = now - manual_move_last_handset_frame_ms;
                if (interval < manual_move_handset_interval_min_ms)
                    manual_move_handset_interval_min_ms = interval;
                if (interval > manual_move_handset_interval_max_ms)
                    manual_move_handset_interval_max_ms = interval;
                manual_move_handset_interval_sum_ms += interval;
                manual_move_handset_interval_count++;
            }
            manual_move_last_handset_frame_ms = now;
            manual_move_handset_frames++;

            const uint8_t pressed = buf[2];
            if (direction == MANUAL_MOVE_UP)
            {
                manual_move_up_frames++;
                manual_move_last_direction_ms = now;
                if (manual_move_direction == MANUAL_MOVE_DOWN)
                    manual_move_direction = MANUAL_MOVE_MIXED;
            }
            else if (direction == MANUAL_MOVE_DOWN)
            {
                manual_move_down_frames++;
                manual_move_last_direction_ms = now;
                if (manual_move_direction == MANUAL_MOVE_UP)
                    manual_move_direction = MANUAL_MOVE_MIXED;
            }
            else if (direction == MANUAL_MOVE_MIXED)
            {
                manual_move_other_frames++;
                manual_move_last_direction_ms = now;
                manual_move_direction = MANUAL_MOVE_MIXED;
            }
            else if (pressed == 0x00)
            {
                manual_move_normal_frames++;
            }
            else
            {
                manual_move_other_frames++;
            }
        }

        void SmartDesk::observe_manual_rx_update_(const uint8_t *buf, uint32_t now)
        {
            if (!manual_move_debug || !manual_move_active || rx_decoder == nullptr || buf == nullptr)
            {
                return;
            }

            store_debug_frame_(manual_move_recent_rx_frames, manual_move_recent_rx_next,
                               manual_move_recent_rx_count, buf, now, manual_move_started_ms);

            if (manual_move_last_rx_update_ms != 0)
            {
                const uint32_t interval = now - manual_move_last_rx_update_ms;
                if (interval < manual_move_rx_interval_min_ms)
                    manual_move_rx_interval_min_ms = interval;
                if (interval > manual_move_rx_interval_max_ms)
                    manual_move_rx_interval_max_ms = interval;
                manual_move_rx_interval_sum_ms += interval;
                manual_move_rx_interval_count++;
            }
            manual_move_last_rx_update_ms = now;
            manual_move_rx_updates++;

            manual_move_end_height = current_height;
            manual_move_last_display = rx_decoder->get_desk_display();
            manual_move_last_state = rx_decoder->get_desk_state();
            if (manual_move_last_state == "报错" ||
                (!manual_move_last_display.empty() && manual_move_last_display[0] == 'E') ||
                manual_move_last_display == "H0t")
            {
                manual_move_error_seen = true;
            }
        }

        void SmartDesk::maybe_finish_manual_move_(uint32_t now)
        {
            if (!manual_move_debug || !manual_move_active || manual_move_last_direction_ms == 0)
            {
                return;
            }
            if (now - manual_move_last_direction_ms >= MANUAL_MOVE_END_TIMEOUT_MS)
            {
                finish_manual_move_(now, "release_timeout");
            }
        }

        void SmartDesk::finish_manual_move_(uint32_t now, const char *reason)
        {
            const uint32_t duration = now - manual_move_started_ms;
            const uint32_t handset_avg = manual_move_handset_interval_count == 0 ? 0 :
                                                                                   manual_move_handset_interval_sum_ms / manual_move_handset_interval_count;
            const uint32_t rx_avg = manual_move_rx_interval_count == 0 ? 0 :
                                                                         manual_move_rx_interval_sum_ms / manual_move_rx_interval_count;
            const uint32_t handset_min = manual_move_handset_interval_count == 0 ? 0 : manual_move_handset_interval_min_ms;
            const uint32_t rx_min = manual_move_rx_interval_count == 0 ? 0 : manual_move_rx_interval_min_ms;
            const float delta = (!std::isnan(manual_move_start_height) && !std::isnan(manual_move_end_height)) ?
                                    manual_move_end_height - manual_move_start_height :
                                    NAN;

            ESP_LOGW(TAG, "Manual move ended: reason=%s direction=%s duration=%" PRIu32 "ms",
                     reason, manual_move_direction_to_string_(manual_move_direction), duration);
            ESP_LOGW(TAG, "  handset_frames=%" PRIu32 " up=%" PRIu32 " down=%" PRIu32 " normal=%" PRIu32 " other=%" PRIu32,
                     manual_move_handset_frames, manual_move_up_frames, manual_move_down_frames,
                     manual_move_normal_frames, manual_move_other_frames);
            ESP_LOGW(TAG, "  handset_interval_ms min=%" PRIu32 " avg=%" PRIu32 " max=%" PRIu32,
                     handset_min, handset_avg, manual_move_handset_interval_max_ms);
            ESP_LOGW(TAG, "  height_start=%.1f height_end=%.1f delta=%.1f",
                     manual_move_start_height, manual_move_end_height, delta);
            ESP_LOGW(TAG, "  rx_updates=%" PRIu32 " rx_interval_ms min=%" PRIu32 " avg=%" PRIu32 " max=%" PRIu32,
                     manual_move_rx_updates, rx_min, rx_avg, manual_move_rx_interval_max_ms);
            ESP_LOGW(TAG, "  display_last=%s state=%s error_seen=%s",
                     manual_move_last_display.c_str(), manual_move_last_state.c_str(),
                     manual_move_error_seen ? "true" : "false");

            if (manual_move_debug_dump_frames)
            {
                log_recent_debug_frames_("recent handset frames", manual_move_recent_handset_frames,
                                         manual_move_recent_handset_next, manual_move_recent_handset_count);
                log_recent_debug_frames_("recent controlbox frames", manual_move_recent_rx_frames,
                                         manual_move_recent_rx_next, manual_move_recent_rx_count);
            }

            manual_move_active = false;
        }

        void SmartDesk::reset_manual_move_(uint32_t now, manual_move_direction_t direction)
        {
            manual_move_active = true;
            manual_move_direction = direction;
            manual_move_started_ms = now;
            manual_move_last_direction_ms = now;
            manual_move_last_handset_frame_ms = 0;
            manual_move_handset_frames = 0;
            manual_move_up_frames = 0;
            manual_move_down_frames = 0;
            manual_move_normal_frames = 0;
            manual_move_other_frames = 0;
            manual_move_handset_interval_min_ms = UINT32_MAX;
            manual_move_handset_interval_max_ms = 0;
            manual_move_handset_interval_sum_ms = 0;
            manual_move_handset_interval_count = 0;
            manual_move_last_rx_update_ms = 0;
            manual_move_rx_updates = 0;
            manual_move_rx_interval_min_ms = UINT32_MAX;
            manual_move_rx_interval_max_ms = 0;
            manual_move_rx_interval_sum_ms = 0;
            manual_move_rx_interval_count = 0;
            manual_move_start_height = current_height;
            manual_move_end_height = current_height;
            manual_move_error_seen = false;
            manual_move_last_display = rx_decoder != nullptr ? rx_decoder->get_desk_display() : "";
            manual_move_last_state = rx_decoder != nullptr ? rx_decoder->get_desk_state() : "";
            manual_move_recent_handset_next = 0;
            manual_move_recent_handset_count = 0;
            manual_move_recent_rx_next = 0;
            manual_move_recent_rx_count = 0;
        }

        void SmartDesk::reset_target_move_debug_(uint32_t now)
        {
            if (!manual_move_debug)
            {
                return;
            }
            target_move_debug_active = true;
            target_move_direction = move_state == MOVE_UP ? MANUAL_MOVE_UP :
                                    move_state == MOVE_DOWN ? MANUAL_MOVE_DOWN :
                                                              MANUAL_MOVE_NONE;
            target_move_started_ms = now;
            target_move_last_tx_frame_ms = 0;
            target_move_sent_frames = 0;
            target_move_up_frames = 0;
            target_move_down_frames = 0;
            target_move_normal_frames = 0;
            target_move_other_frames = 0;
            target_move_injected_frames = 0;
            target_move_passthrough_frames = 0;
            target_move_tx_interval_min_ms = UINT32_MAX;
            target_move_tx_interval_max_ms = 0;
            target_move_tx_interval_sum_ms = 0;
            target_move_tx_interval_count = 0;
            target_move_last_rx_update_ms = 0;
            target_move_rx_updates = 0;
            target_move_rx_interval_min_ms = UINT32_MAX;
            target_move_rx_interval_max_ms = 0;
            target_move_rx_interval_sum_ms = 0;
            target_move_rx_interval_count = 0;
            target_move_start_height = current_height;
            target_move_end_height = current_height;
            target_move_target_height = target_height;
            target_move_instant_overshoot = NAN;
            target_move_result = "";
            target_move_error_seen = false;
            target_move_last_display = rx_decoder != nullptr ? rx_decoder->get_desk_display() : "";
            target_move_last_state = rx_decoder != nullptr ? rx_decoder->get_desk_state() : "";
            target_move_recent_tx_next = 0;
            target_move_recent_tx_count = 0;
            target_move_recent_rx_next = 0;
            target_move_recent_rx_count = 0;
        }

        void SmartDesk::observe_target_tx_frame_(const uint8_t *buf, uint32_t now, bool injected)
        {
            if (!manual_move_debug || !target_move_debug_active || buf == nullptr)
            {
                return;
            }

            store_debug_frame_(target_move_recent_tx_frames, target_move_recent_tx_next,
                               target_move_recent_tx_count, buf, now, target_move_started_ms);

            if (target_move_last_tx_frame_ms != 0)
            {
                const uint32_t interval = now - target_move_last_tx_frame_ms;
                if (interval < target_move_tx_interval_min_ms)
                    target_move_tx_interval_min_ms = interval;
                if (interval > target_move_tx_interval_max_ms)
                    target_move_tx_interval_max_ms = interval;
                target_move_tx_interval_sum_ms += interval;
                target_move_tx_interval_count++;
            }
            target_move_last_tx_frame_ms = now;
            target_move_sent_frames++;
            if (injected)
                target_move_injected_frames++;
            else
                target_move_passthrough_frames++;

            const manual_move_direction_t direction = get_handset_direction_(buf);
            if (direction == MANUAL_MOVE_UP)
            {
                target_move_up_frames++;
                if (target_move_direction == MANUAL_MOVE_DOWN)
                    target_move_direction = MANUAL_MOVE_MIXED;
            }
            else if (direction == MANUAL_MOVE_DOWN)
            {
                target_move_down_frames++;
                if (target_move_direction == MANUAL_MOVE_UP)
                    target_move_direction = MANUAL_MOVE_MIXED;
            }
            else if (direction == MANUAL_MOVE_MIXED)
            {
                target_move_other_frames++;
                target_move_direction = MANUAL_MOVE_MIXED;
            }
            else if (buf[2] == 0x00)
            {
                target_move_normal_frames++;
            }
            else
            {
                target_move_other_frames++;
            }
        }

        void SmartDesk::observe_target_rx_update_(const uint8_t *buf, uint32_t now)
        {
            if (!manual_move_debug || !target_move_debug_active || rx_decoder == nullptr || buf == nullptr)
            {
                return;
            }

            store_debug_frame_(target_move_recent_rx_frames, target_move_recent_rx_next,
                               target_move_recent_rx_count, buf, now, target_move_started_ms);

            if (target_move_last_rx_update_ms != 0)
            {
                const uint32_t interval = now - target_move_last_rx_update_ms;
                if (interval < target_move_rx_interval_min_ms)
                    target_move_rx_interval_min_ms = interval;
                if (interval > target_move_rx_interval_max_ms)
                    target_move_rx_interval_max_ms = interval;
                target_move_rx_interval_sum_ms += interval;
                target_move_rx_interval_count++;
            }
            target_move_last_rx_update_ms = now;
            target_move_rx_updates++;
            target_move_end_height = current_height;
            target_move_last_display = rx_decoder->get_desk_display();
            target_move_last_state = rx_decoder->get_desk_state();
            if (target_move_last_state == "报错" ||
                (!target_move_last_display.empty() && target_move_last_display[0] == 'E') ||
                target_move_last_display == "H0t")
            {
                target_move_error_seen = true;
            }
        }

        void SmartDesk::finish_target_move_debug_(uint32_t now, const std::string &result)
        {
            if (!manual_move_debug || !target_move_debug_active)
            {
                return;
            }

            const uint32_t duration = now - target_move_started_ms;
            const uint32_t tx_avg = target_move_tx_interval_count == 0 ? 0 :
                                                                             target_move_tx_interval_sum_ms / target_move_tx_interval_count;
            const uint32_t rx_avg = target_move_rx_interval_count == 0 ? 0 :
                                                                             target_move_rx_interval_sum_ms / target_move_rx_interval_count;
            const uint32_t tx_min = target_move_tx_interval_count == 0 ? 0 : target_move_tx_interval_min_ms;
            const uint32_t rx_min = target_move_rx_interval_count == 0 ? 0 : target_move_rx_interval_min_ms;
            const float delta = (!std::isnan(target_move_start_height) && !std::isnan(target_move_end_height)) ?
                                    target_move_end_height - target_move_start_height :
                                    NAN;
            const float requested = (!std::isnan(target_move_start_height) && !std::isnan(target_move_target_height)) ?
                                        target_move_target_height - target_move_start_height :
                                        NAN;
            const float avg_speed = duration == 0 || std::isnan(delta) ? NAN :
                                                                        std::fabs(delta) * 1000.0f / duration;
            target_move_instant_overshoot = (!std::isnan(target_move_end_height) && !std::isnan(target_move_target_height)) ?
                                                (target_move_end_height - target_move_target_height) :
                                                NAN;
            target_move_result = result;

            ESP_LOGW(TAG, "Target move ended: result=%s direction=%s duration=%" PRIu32 "ms target=%.1f",
                     result.c_str(), manual_move_direction_to_string_(target_move_direction), duration, target_move_target_height);
            ESP_LOGW(TAG, "  requested_delta=%.1f traveled_delta=%.1f avg_speed=%.2fcm/s stop_margin=%.2f instant_overshoot=%.1f",
                     requested, delta, avg_speed, last_move_stop_margin, target_move_instant_overshoot);
            ESP_LOGW(TAG, "  tx_frames=%" PRIu32 " up=%" PRIu32 " down=%" PRIu32 " normal=%" PRIu32 " other=%" PRIu32,
                     target_move_sent_frames, target_move_up_frames, target_move_down_frames,
                     target_move_normal_frames, target_move_other_frames);
            ESP_LOGW(TAG, "  tx_source injected=%" PRIu32 " passthrough=%" PRIu32,
                     target_move_injected_frames, target_move_passthrough_frames);
            ESP_LOGW(TAG, "  tx_interval_ms min=%" PRIu32 " avg=%" PRIu32 " max=%" PRIu32,
                     tx_min, tx_avg, target_move_tx_interval_max_ms);
            ESP_LOGW(TAG, "  height_start=%.1f height_end=%.1f delta=%.1f",
                     target_move_start_height, target_move_end_height, delta);
            ESP_LOGW(TAG, "  rx_updates=%" PRIu32 " rx_interval_ms min=%" PRIu32 " avg=%" PRIu32 " max=%" PRIu32,
                     target_move_rx_updates, rx_min, rx_avg, target_move_rx_interval_max_ms);
            ESP_LOGW(TAG, "  display_last=%s state=%s error_seen=%s",
                     target_move_last_display.c_str(), target_move_last_state.c_str(),
                     target_move_error_seen ? "true" : "false");

            if (manual_move_debug_dump_frames)
            {
                log_recent_debug_frames_("recent target tx frames", target_move_recent_tx_frames,
                                         target_move_recent_tx_next, target_move_recent_tx_count);
                log_recent_debug_frames_("recent target controlbox frames", target_move_recent_rx_frames,
                                         target_move_recent_rx_next, target_move_recent_rx_count);
            }

            target_move_debug_active = false;
            target_move_final_sample_pending = true;
            target_move_final_sample_due_ms = now + TARGET_MOVE_FINAL_SAMPLE_DELAY_MS;
            target_move_final_sample_target_height = target_move_target_height;
            target_move_final_sample_stop_height = target_move_end_height;
            target_move_final_sample_direction = target_move_direction;
            target_move_final_sample_result = result;
        }

        void SmartDesk::maybe_log_target_move_final_sample_(uint32_t now)
        {
            if (!manual_move_debug || !target_move_final_sample_pending)
            {
                return;
            }
            if (static_cast<int32_t>(now - target_move_final_sample_due_ms) < 0)
            {
                return;
            }

            target_move_final_sample_pending = false;
            const float final_height = current_height;
            const float final_overshoot = (!std::isnan(final_height) && !std::isnan(target_move_final_sample_target_height)) ?
                                              final_height - target_move_final_sample_target_height :
                                              NAN;
            const float post_stop_drift = (!std::isnan(final_height) && !std::isnan(target_move_final_sample_stop_height)) ?
                                              final_height - target_move_final_sample_stop_height :
                                              NAN;
            ESP_LOGW(TAG, "Target move final sample: result=%s direction=%s final_height=%.1f target=%.1f",
                     target_move_final_sample_result.c_str(),
                     manual_move_direction_to_string_(target_move_final_sample_direction),
                     final_height, target_move_final_sample_target_height);
            ESP_LOGW(TAG, "  final_overshoot=%.1f post_stop_drift=%.1f sample_delay=%" PRIu32 "ms",
                     final_overshoot, post_stop_drift, TARGET_MOVE_FINAL_SAMPLE_DELAY_MS);
        }

        SmartDesk::manual_move_direction_t SmartDesk::get_handset_direction_(const uint8_t *buf) const
        {
            if (buf == nullptr)
            {
                return MANUAL_MOVE_NONE;
            }
            const uint8_t pressed = buf[2];
            const bool up = (pressed & DESK_UP) != 0;
            const bool down = (pressed & DESK_DOWN) != 0;
            if (up && down)
                return MANUAL_MOVE_MIXED;
            if (up)
                return MANUAL_MOVE_UP;
            if (down)
                return MANUAL_MOVE_DOWN;
            return MANUAL_MOVE_NONE;
        }

        const char *SmartDesk::manual_move_direction_to_string_(manual_move_direction_t direction) const
        {
            switch (direction)
            {
            case MANUAL_MOVE_UP:
                return "up";
            case MANUAL_MOVE_DOWN:
                return "down";
            case MANUAL_MOVE_MIXED:
                return "mixed";
            case MANUAL_MOVE_NONE:
            default:
                return "none";
            }
        }

        void SmartDesk::store_debug_frame_(DebugFrame *frames, size_t &next, size_t &count, const uint8_t *buf, uint32_t now, uint32_t started_ms)
        {
            if (frames == nullptr || buf == nullptr)
            {
                return;
            }
            DebugFrame &frame = frames[next];
            frame.offset_ms = now - started_ms;
            memcpy(frame.bytes, buf, sizeof(frame.bytes));
            next = (next + 1) % MANUAL_DEBUG_RECENT_FRAME_COUNT;
            if (count < MANUAL_DEBUG_RECENT_FRAME_COUNT)
            {
                count++;
            }
        }

        void SmartDesk::log_recent_debug_frames_(const char *label, const DebugFrame *frames, size_t next, size_t count) const
        {
            if (frames == nullptr || count == 0)
            {
                return;
            }
            ESP_LOGW(TAG, "  %s:", label);
            const size_t start = count == MANUAL_DEBUG_RECENT_FRAME_COUNT ? next : 0;
            for (size_t i = 0; i < count; i++)
            {
                const DebugFrame &frame = frames[(start + i) % MANUAL_DEBUG_RECENT_FRAME_COUNT];
                ESP_LOGW(TAG, "    +%" PRIu32 "ms %02X %02X %02X %02X %02X",
                         frame.offset_ms, frame.bytes[0], frame.bytes[1], frame.bytes[2],
                         frame.bytes[3], frame.bytes[4]);
            }
        }

        void SmartDesk::write_control_frame_(const uint8_t *buf, uint32_t now, bool injected)
        {
            if (buf == nullptr || uart_control == nullptr)
            {
                return;
            }
            observe_target_tx_frame_(buf, now, injected);
            uart_control->write_array(buf, 5);
        }

        bool SmartDesk::start_move_to_height(float target_height)
        {
            if (this->is_reset_active_())
            {
                ESP_LOGW(TAG, "Cannot move desk while reset is running");
                return false;
            }
            if (button_action_active)
            {
                this->finish_button_action_("button_cancelled");
            }
            if (std::isnan(current_height))
            {
                ESP_LOGW(TAG, "Cannot move desk: current height is unknown");
                return false;
            }
            if (target_height < min_desk_height || target_height > max_desk_height)
            {
                ESP_LOGW(TAG, "Target height %.1f cm is outside %.1f-%.1f cm",
                         target_height, min_desk_height, max_desk_height);
                return false;
            }

            this->target_height = target_height;
            this->clear_commands();

            if (std::fabs(current_height - this->target_height) <= move_tolerance)
            {
                this->finish_move_("reached");
                return true;
            }

            move_state = current_height < this->target_height ? MOVE_UP : MOVE_DOWN;
            move_started_ms = millis();
            move_start_height = current_height;
            last_move_stop_margin = this->get_move_stop_margin_();
            last_move_command_ms = 0;
            last_move_progress_ms = move_started_ms;
            last_move_progress_height = current_height;
            last_move_result = "moving";
            reset_target_move_debug_(move_started_ms);
            publish_diagnostics_();
            ESP_LOGI(TAG, "Moving desk from %.1f cm to %.1f cm", current_height, this->target_height);
            return true;
        }

        void SmartDesk::stop_moving()
        {
            if (this->is_reset_active_())
            {
                this->finish_reset_("reset_cancelled");
                return;
            }
            if (button_action_active)
            {
                this->finish_button_action_("button_stopped");
                return;
            }
            this->finish_move_("stopped");
        }

        void SmartDesk::finish_move_(const std::string &result)
        {
            if (move_state != MOVE_IDLE)
            {
                ESP_LOGI(TAG, "Stopping desk move at %.1f cm", current_height);
            }
            finish_target_move_debug_(millis(), result);
            last_move_result = result;
            move_state = MOVE_IDLE;
            target_height = NAN;
            move_started_ms = 0;
            move_start_height = NAN;
            last_move_stop_margin = NAN;
            last_move_command_ms = 0;
            last_move_progress_ms = 0;
            last_move_progress_height = NAN;
            this->clear_commands();
            publish_diagnostics_();
        }

        void SmartDesk::process_move_(bool force_command_refill)
        {
            if (move_state == MOVE_IDLE)
            {
                return;
            }

            const uint32_t now = millis();
            if (now - move_started_ms > move_timeout_ms)
            {
                ESP_LOGW(TAG, "Timed out moving desk toward %.1f cm", target_height);
                this->finish_move_("timeout");
                return;
            }

            if (std::isnan(current_height))
            {
                return;
            }

            const float error = target_height - current_height;
            const float stop_margin = this->get_move_stop_margin_();
            last_move_stop_margin = stop_margin;
            if (std::fabs(error) <= stop_margin)
            {
                ESP_LOGI(TAG, "Reached desk target %.1f cm (current %.1f cm, stop margin %.2f cm)",
                         target_height, current_height, stop_margin);
                this->finish_move_("reached");
                return;
            }

            if (std::isnan(last_move_progress_height) ||
                std::fabs(current_height - last_move_progress_height) >= move_stall_tolerance)
            {
                last_move_progress_height = current_height;
                last_move_progress_ms = now;
            }
            else if (last_move_progress_ms != 0 && now - last_move_progress_ms > move_stall_timeout_ms)
            {
                ESP_LOGW(TAG, "Desk move stalled at %.1f cm while targeting %.1f cm", current_height, target_height);
                this->finish_move_("stalled");
                return;
            }

            const move_state_t next_state = error > 0.0f ? MOVE_UP : MOVE_DOWN;
            if (next_state != move_state)
            {
                this->clear_commands();
                move_state = next_state;
                last_move_command_ms = 0;
            }

            if (!tx_controller->is_empty() ||
                (!force_command_refill && last_move_command_ms != 0 && now - last_move_command_ms < move_command_interval_ms))
            {
                return;
            }

            if (add_command(move_state == MOVE_UP ? "U" : "D", move_command_repeat))
            {
                last_move_command_ms = now;
            }
        }

        void SmartDesk::publish_diagnostics_()
        {
            if (sensor_target_height != nullptr)
            {
                sensor_target_height->publish_state(target_height);
            }
            if (sensor_learned_idle_interval != nullptr)
            {
                sensor_learned_idle_interval->publish_state(learned_handset_idle_interval_ms);
            }
            if (text_sensor_movement != nullptr)
            {
                text_sensor_movement->publish_state(get_movement_state());
            }
            if (text_sensor_last_move_result != nullptr)
            {
                text_sensor_last_move_result->publish_state(last_move_result);
            }
        }

        float SmartDesk::get_move_stop_margin_() const
        {
            if (std::isnan(move_start_height) || std::isnan(current_height) || std::isnan(target_height))
            {
                return move_tolerance;
            }

            const float traveled = std::fabs(current_height - move_start_height);
            float margin = move_stop_margin_base + traveled * move_stop_margin_per_cm;
            const float min_margin = move_stop_margin_min > move_tolerance ? move_stop_margin_min : move_tolerance;
            const float max_margin = move_stop_margin_max > min_margin ? move_stop_margin_max : min_margin;

            if (margin < min_margin)
            {
                margin = min_margin;
            }
            if (margin > max_margin)
            {
                margin = max_margin;
            }
            return margin;
        }

        void SmartDesk::sync_decoder_height_range_()
        {
            if (rx_decoder != nullptr)
            {
                rx_decoder->set_height_range(min_desk_height, max_desk_height);
            }
        }

        bool SmartDesk::add_command(std::string button_chars, int repeat)
        {
            if (this->is_reset_active_())
            {
                ESP_LOGW(TAG, "Ignoring command '%s' while reset is running", button_chars.c_str());
                return false;
            }
            return enqueue_command_(button_chars, repeat);
        }

        bool SmartDesk::press_button_command(std::string button_chars, int repeat)
        {
            if (this->is_reset_active_())
            {
                ESP_LOGW(TAG, "Ignoring button command '%s' while reset is running", button_chars.c_str());
                return false;
            }

            const bool queued = enqueue_command_(button_chars, repeat);
            if (queued && should_track_button_action_(button_chars))
            {
                start_button_action_(button_chars);
            }
            return queued;
        }

        bool SmartDesk::enqueue_command_(std::string button_chars, int repeat)
        {
            if (tx_controller == nullptr || repeat <= 0 || tx_controller->is_full())
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
