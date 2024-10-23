#include "move_to_level_4.h"

namespace esphome
{
    namespace nicode_smart_desk
    {
        void SmartDeskMoveToLevel4Button::press_action()
        {
            this->parent_->add_command("4");
        }
    }
}