#pragma once

#include "esphome/components/button/button.h"
#include "../smart_desk.h"

namespace esphome
{
    namespace nicode_smart_desk
    {
        class SmartDeskMoveUpButton : public button::Button, public Parented<SmartDesk>
        {
        public:
            SmartDeskMoveUpButton() = default;

        protected:
            void press_action() override;
        };
    }
}