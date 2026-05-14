#include "smart_desk_button.h"

namespace esphome
{
    namespace nicode_smart_desk
    {
        void SmartDeskButton::press_action()
        {
            if (command_.empty())
            {
                return;
            }

            if (repeat_ > 0)
            {
                this->parent_->add_command(command_, repeat_);
            }
            else
            {
                this->parent_->add_command(command_);
            }
        }
    }
}
