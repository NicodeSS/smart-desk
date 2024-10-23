#pragma once

#include "esphome/components/button/button.h"
#include "../smart_desk.h"

namespace esphome
{
    namespace nicode_smart_desk
    {
        class SmartDeskSetMemoryButton : public button::Button, public Parented<SmartDesk>
        {
        public:
            SmartDeskSetMemoryButton() = default;

        protected:
            void press_action() override;
        };
    }
}