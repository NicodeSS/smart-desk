#include "target_height_number.h"
#include "esphome/core/log.h"
#include <cmath>

namespace esphome
{
    namespace nicode_smart_desk
    {
        static const char *const TAG = "smart_desk.number";

        void SmartDeskTargetHeightNumber::setup()
        {
            const float height = this->parent_->get_current_height();
            if (!std::isnan(height))
            {
                this->publish_state(height);
                last_published_height = height;
            }
        }

        void SmartDeskTargetHeightNumber::loop()
        {
            if (this->parent_->is_moving())
            {
                return;
            }

            const float height = this->parent_->get_current_height();
            if (!std::isnan(height) && (std::isnan(last_published_height) || std::fabs(height - last_published_height) >= 0.1f))
            {
                this->publish_state(height);
                last_published_height = height;
            }
        }

        void SmartDeskTargetHeightNumber::dump_config()
        {
            LOG_NUMBER("", "Smart Desk Target Height", this);
        }

        void SmartDeskTargetHeightNumber::control(float value)
        {
            if (this->parent_->start_move_to_height(value))
            {
                this->publish_state(value);
                last_published_height = value;
            }
        }
    }
}
