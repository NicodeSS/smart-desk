#include "move_to_level_3.h"

namespace esphome
{
    namespace nicode_smart_desk
    {
        void SmartDeskMoveToLevel3Button::press_action()
        {
            this->parent_->add_command("3");
        }
    }
}