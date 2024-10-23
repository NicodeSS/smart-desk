#include "move_to_level_2.h"

namespace esphome
{
    namespace nicode_smart_desk
    {
        void SmartDeskMoveToLevel2Button::press_action()
        {
            this->parent_->add_command("2");
        }
    }
}