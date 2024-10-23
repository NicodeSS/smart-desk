#include "move_down.h"

namespace esphome
{
    namespace nicode_smart_desk
    {
        void SmartDeskMoveDownButton::press_action()
        {
            this->parent_->add_command("D");
        }
    }
}