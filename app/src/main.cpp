#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <inttypes.h>

#define DT_DRV_COMPAT custom_led_sensor

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

int main(void)
{
    const struct device *dev = DEVICE_DT_GET_ANY(custom_led_sensor);
    struct sensor_value value;
    int ret;

    if (dev == NULL || !device_is_ready(dev))
    {
        LOG_ERR("LED sensor device not found or not ready");
        return -ENODEV;
    }

    while (1)
    {
        ret = sensor_sample_fetch(dev);
        if (ret < 0)
        {
            LOG_ERR("sample_fetch failed: %d", ret);
            return ret;
        }

        LOG_INF("LED ON");
        k_msleep(1000);

        ret = sensor_channel_get(dev, SENSOR_CHAN_PROX, &value);
        if (ret < 0)
        {
            LOG_ERR("sensor_channel_get failed: %d", ret);
            return ret;
        }
        LOG_INF("LED OFF (prev value: %d)", value.val1);
        k_msleep(1000);
    }
    return 0;
}
