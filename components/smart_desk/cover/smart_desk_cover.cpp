#include "smart_desk_cover.h"
#include "esphome/core/log.h"
#include <cmath>

namespace esphome
{
    namespace nicode_smart_desk
    {
        static const char *const TAG = "smart_desk.cover";

        void SmartDeskCover::setup()
        {
            const float position = this->parent_->height_to_position(this->parent_->get_current_height());
            if (!std::isnan(position))
            {
                this->position = position;
                last_published_position = position;
            }
            this->current_operation = cover::COVER_OPERATION_IDLE;
        }

        void SmartDeskCover::loop()
        {
            bool changed = false;
            const float position = this->parent_->height_to_position(this->parent_->get_current_height());
            if (!std::isnan(position) && (std::isnan(last_published_position) || std::fabs(position - last_published_position) >= 0.005f))
            {
                this->position = position;
                last_published_position = position;
                changed = true;
            }

            cover::CoverOperation operation = cover::COVER_OPERATION_IDLE;
            const int direction = this->parent_->get_move_direction();
            if (direction > 0)
            {
                operation = cover::COVER_OPERATION_OPENING;
            }
            else if (direction < 0)
            {
                operation = cover::COVER_OPERATION_CLOSING;
            }

            if (operation != last_published_operation)
            {
                this->current_operation = operation;
                last_published_operation = operation;
                changed = true;
            }

            if (changed)
            {
                this->publish_state();
            }
        }

        void SmartDeskCover::dump_config()
        {
            LOG_COVER("", "Smart Desk Cover", this);
        }

        cover::CoverTraits SmartDeskCover::get_traits()
        {
            auto traits = cover::CoverTraits();
            traits.set_supports_position(true);
            traits.set_supports_stop(true);
            traits.set_supports_tilt(false);
            traits.set_is_assumed_state(false);
            return traits;
        }

        void SmartDeskCover::control(const cover::CoverCall &call)
        {
            if (call.get_stop())
            {
                this->parent_->stop_moving();
                this->current_operation = cover::COVER_OPERATION_IDLE;
                this->publish_state();
                return;
            }

            if (call.get_position().has_value())
            {
                const float position = *call.get_position();
                if (this->parent_->start_move_to_position(position))
                {
                    const int direction = this->parent_->get_move_direction();
                    if (direction > 0)
                    {
                        this->current_operation = cover::COVER_OPERATION_OPENING;
                    }
                    else if (direction < 0)
                    {
                        this->current_operation = cover::COVER_OPERATION_CLOSING;
                    }
                    else
                    {
                        this->current_operation = cover::COVER_OPERATION_IDLE;
                    }
                    last_published_operation = this->current_operation;
                    this->publish_state();
                }
            }
        }
    }
}
