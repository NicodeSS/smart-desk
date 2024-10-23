#pragma once

#include "esphome/components/button/button.h"
#include "../smart_desk.h"

namespace esphome
{
    namespace nicode_smart_desk
    {
        class SmartDeskMoveToLevel1Button : public button::Button, public Parented<SmartDesk>
        {
        public:
            SmartDeskMoveToLevel1Button() = default;

        protected:
            void press_action() override;
        };
    }
}