#pragma once

#include "esphome/components/cover/cover.h"
#include "../smart_desk.h"

namespace esphome
{
    namespace nicode_smart_desk
    {
        class SmartDeskCover : public cover::Cover, public Component, public Parented<SmartDesk>
        {
        public:
            void setup() override;
            void loop() override;
            void dump_config() override;
            cover::CoverTraits get_traits() override;

        protected:
            void control(const cover::CoverCall &call) override;
            float last_published_position = NAN;
            cover::CoverOperation last_published_operation = cover::COVER_OPERATION_IDLE;
        };
    }
}
