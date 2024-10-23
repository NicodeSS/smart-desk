#include "move_up.h"

namespace esphome
{
    namespace nicode_smart_desk
    {
        void SmartDeskMoveUpButton::press_action()
        {
            this->parent_->add_command("U");
        }
    }
}