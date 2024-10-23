#include "move_to_level_1.h"

namespace esphome
{
    namespace nicode_smart_desk
    {
        void SmartDeskMoveToLevel1Button::press_action()
        {
            this->parent_->add_command("1");
        }
    }
}