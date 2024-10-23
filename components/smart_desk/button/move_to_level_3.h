#pragma once

#include "esphome/components/button/button.h"
#include "../smart_desk.h"

namespace esphome
{
    namespace nicode_smart_desk
    {
        class SmartDeskMoveToLevel3Button : public button::Button, public Parented<SmartDesk>
        {
        public:
            SmartDeskMoveToLevel3Button() = default;

        protected:
            void press_action() override;
        };
    }
}