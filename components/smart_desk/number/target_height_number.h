#pragma once

#include "esphome/components/number/number.h"
#include "../smart_desk.h"

namespace esphome
{
    namespace nicode_smart_desk
    {
        class SmartDeskTargetHeightNumber : public number::Number, public Component, public Parented<SmartDesk>
        {
        public:
            void setup() override;
            void loop() override;
            void dump_config() override;

        protected:
            void control(float value) override;
            float last_published_height = NAN;
        };
    }
}
