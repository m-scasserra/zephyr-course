#define DT_DRV_COMPAT custom_led_sensor

#include <errno.h>
#include <stdbool.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/sensor.h>

struct led_sensor_config
{
    struct gpio_dt_spec led;
};

struct led_sensor_data
{
    bool led_on;
};

static int led_sensor_sample_fetch(const struct device *dev,
                                   enum sensor_channel chan)
{
    const struct led_sensor_config *cfg = dev->config;
    struct led_sensor_data *data = dev->data;

    if (chan != SENSOR_CHAN_ALL && chan != SENSOR_CHAN_PROX)
    {
        return -ENOTSUP;
    }

    int ret = gpio_pin_set_dt(&cfg->led, 1);
    if (ret < 0)
    {
        return ret;
    }

    data->led_on = true;

    return 0;
}

static int led_sensor_channel_get(const struct device *dev,
                                  enum sensor_channel chan,
                                  struct sensor_value *val)
{
    const struct led_sensor_config *cfg = dev->config;
    struct led_sensor_data *data = dev->data;

    if (chan != SENSOR_CHAN_PROX)
    {
        return -ENOTSUP;
    }

    if (val == NULL)
    {
        return -EINVAL;
    }

    const int previous_state = data->led_on ? 1 : 0;
    int ret = gpio_pin_set_dt(&cfg->led, 0);
    if (ret < 0)
    {
        return ret;
    }
    data->led_on = false;
    val->val1 = previous_state;
    val->val2 = 0;
    return 0;
}

static int led_sensor_init(const struct device *dev)
{
    const struct led_sensor_config *cfg = dev->config;
    struct led_sensor_data *data = dev->data;

    if (!gpio_is_ready_dt(&cfg->led))
    {
        return -ENODEV;
    }

    int ret = gpio_pin_configure_dt(&cfg->led, GPIO_OUTPUT_INACTIVE);
    if (ret < 0)
    {
        return ret;
    }

    data->led_on = false;
    return 0;
}

static DEVICE_API(sensor, led_sensor_api) = {
    .sample_fetch = led_sensor_sample_fetch,
    .channel_get = led_sensor_channel_get,
};

#define LED_SENSOR_DEFINE(inst)                                        \
    static struct led_sensor_data led_sensor_data_##inst;              \
    static const struct led_sensor_config led_sensor_config_##inst = { \
        .led = GPIO_DT_SPEC_GET(DT_INST_PHANDLE(inst, led), gpios),    \
    };                                                                 \
    DEVICE_DT_INST_DEFINE(inst, led_sensor_init, NULL,                 \
                          &led_sensor_data_##inst,                     \
                          &led_sensor_config_##inst,                   \
                          POST_KERNEL, CONFIG_SENSOR_INIT_PRIORITY,    \
                          &led_sensor_api);

DT_INST_FOREACH_STATUS_OKAY(LED_SENSOR_DEFINE)