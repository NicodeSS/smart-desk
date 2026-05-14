#include "smart_desk_button.h"

namespace esphome
{
    namespace nicode_smart_desk
    {
        void SmartDeskButton::press_action()
        {
            if (reset_)
            {
                this->parent_->start_reset();
                return;
            }

            if (command_.empty())
            {
                return;
            }

            if (repeat_ > 0)
            {
                this->parent_->press_button_command(command_, repeat_);
            }
            else
            {
                this->parent_->press_button_command(command_);
            }
        }
    }
}
