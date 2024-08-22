#include "standing_desk_height.h"
namespace esphome
{
    namespace standing_desk_height
    {
        static const char *const TAG = "standing_desk_height";

        void StandingDeskHeightSensor::setup()
        {
            if (this->decoder == nullptr)
            {
                this->decoder = new Decoder();
            }
        }

        void StandingDeskHeightSensor::loop()
        {
            while (this->available() > 0)
            {
                uint8_t byte;
                this->read_byte(&byte);

                ESP_LOGVV(TAG, "Reading byte: %d", byte);

                if (this->decoder != nullptr && this->decoder->put(byte))
                {
                    state_t state = this->decoder->get_state();
                    float height = this->decoder->decode();
                    ESP_LOGVV(TAG, "Got desk height: %f", height);
                    this->last_read = height;
                }
            }
        }

        void StandingDeskHeightSensor::update()
        {
            if (this->last_read > 0 && this->last_read != this->last_published)
            {
                publish_state(this->last_read);
                this->last_published = this->last_read;
            }
        }

        float StandingDeskHeightSensor::get_last_read()
        {
            return this->last_read;
        }
    }