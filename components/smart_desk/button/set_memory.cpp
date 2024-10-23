#include "set_memory.h"

namespace esphome
{
    namespace nicode_smart_desk
    {
        void SmartDeskSetMemoryButton::press_action()
        {
            this->parent_->add_command("M");
        }
    }
}