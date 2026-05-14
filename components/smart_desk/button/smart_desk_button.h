#pragma once

#include "esphome/components/button/button.h"
#include "../smart_desk.h"
#include <string>

namespace esphome
{
    namespace nicode_smart_desk
    {
        class SmartDeskButton : public button::Button, public Parented<SmartDesk>
        {
        public:
            SmartDeskButton() = default;
            void set_command(const std::string &command) { command_ = command; }
            void set_repeat(int repeat) { repeat_ = repeat; }
            void set_reset(bool reset) { reset_ = reset; }

        protected:
            void press_action() override;

            std::string command_;
            int repeat_ = 0;
            bool reset_ = false;
        };
    }
}
