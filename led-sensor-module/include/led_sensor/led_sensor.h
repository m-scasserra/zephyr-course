#ifndef LED_SENSOR_H
#define LED_SENSOR_H

#include <stdint.h>
#include <zephyr/device.h>

#ifdef __cplusplus
extern "C"
{
#endif

    int led_sensor_set_on_time(const struct device *dev, uint32_t on_time_ms);

#ifdef __cplusplus
}
#endif

#endif /* LED_SENSOR_H */