#include "awake_handset.h"

namespace esphome
{
    namespace nicode_smart_desk
    {
        void SmartDeskAwakeHandsetButton::press_action()
        {
            this->parent_->add_command("P", 1);
        }
    }
}