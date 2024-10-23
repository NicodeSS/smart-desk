#pragma once

#include "esphome/components/button/button.h"
#include "../smart_desk.h"

namespace esphome
{
    namespace nicode_smart_desk
    {
        class SmartDeskMoveToLevel2Button : public button::Button, public Parented<SmartDesk>
        {
        public:
            SmartDeskMoveToLevel2Button() = default;

        protected:
            void press_action() override;
        };
    }
}