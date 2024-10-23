#pragma once

#include "esphome/components/button/button.h"
#include "../smart_desk.h"

namespace esphome
{
    namespace nicode_smart_desk
    {
        class SmartDeskAwakeHandsetButton : public button::Button, public Parented<SmartDesk>
        {
        public:
            SmartDeskAwakeHandsetButton() = default;

        protected:
            void press_action() override;
        };
    }
}