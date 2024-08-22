#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/uart/uart.h"

namespace esphome
{
    namespace standing_desk_height
    {

        class StandingDeskHeightSensor : public PollingComponent, public uart::UARTDevice, public sensor::Sensor
        {
        public:
            void setup() override;
            void loop() override;
            void update() override;
            void dump_config() override;
            float get_last_read();

        protected:
            Decoder *decoder;

            float last_read = -1;
            float last_published = -1;

            bool is_detecting = false;
            uint32_t started_detecting_at = 0;

            void try_next_decoder();
        };

    }
}
