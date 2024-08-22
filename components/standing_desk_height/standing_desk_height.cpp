#include "standing_desk_height.h"
namespace esphome
{
    namespace standing_desk_height
    {
        static std::string TAG = "standing_desk_height";

        void StandingDeskHeightSensor::setup()
        {
            ESP_LOGVV(TAG, "Sensor Setup");
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
                    std::string content = this->decoder->decode();
                    ESP_LOGVV(TAG, "Got desk display: %s", content);
                    float height = this->decoder->get_height();
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
}