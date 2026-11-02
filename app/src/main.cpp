#include <errno.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <led_sensor/led_sensor.h>
#include <zephyr/shell/shell.h>

#define LED_SENSOR_NODE DT_COMPAT_GET_ANY_STATUS_OKAY(custom_led_sensor)
#define LED_SENSOR_DEV DEVICE_DT_GET(LED_SENSOR_NODE)

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

int main(void)
{
    const struct device *dev = DEVICE_DT_GET(LED_SENSOR_NODE);
    LOG_INF("LED sensor device: %s, ready: %s", dev->name, device_is_ready(dev) ? "yes" : "no");

    return 0;
}

static int get_led_sensor(const struct shell *sh, const struct device **dev)
{
    *dev = LED_SENSOR_DEV;
    if (!device_is_ready(*dev))
    {
        shell_error(sh, "LED sensor '%s' device not ready", (*dev)->name);
        return -ENODEV;
    }
    return 0;
}

static int cmd_sensor_fetch(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);
    const struct device *dev;
    int ret = get_led_sensor(sh, &dev);

    if (ret < 0)
    {
        return ret;
    }
    ret = sensor_sample_fetch(dev);
    if (ret < 0)
    {
        shell_error(sh, "sensor_sample_fetch failed: %d", ret);
        return ret;
    }
    shell_print(sh, "Sample fetched; LED ON");
    return 0;
}

static int cmd_sensor_read(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);
    const struct device *dev;
    struct sensor_value value;
    int ret = get_led_sensor(sh, &dev);

    if (ret < 0)
    {
        return ret;
    }
    ret = sensor_channel_get(dev, SENSOR_CHAN_PROX, &value);
    if (ret < 0)
    {
        shell_error(sh, "sensor_channel_get failed: %d", ret);
        return ret;
    }

    shell_print(sh, "Previous LED state: %d", value.val1);
    return 0;
}

static int cmd_sensor_info(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);
    const struct device *dev = LED_SENSOR_DEV;

    shell_print(sh, "LED sensor device: %s, ready: %s", dev->name, device_is_ready(dev) ? "yes" : "no");
    return 0;
}

static int cmd_sensor_set(const struct shell *sh, size_t argc, char **argv)
{
    const struct device *dev;
    char *end;
    unsigned long requested;
    int ret;

    if (argc != 2 || argv[1] == NULL || argv[1][0] == '\0')
    {
        shell_error(sh, "Usage: lsensor set <requested_state> (1..60000)");
        return -EINVAL;
    }

    for (const char *p = argv[1]; *p != '\0'; ++p)
    {
        if (*p < '0' || *p > '9')
        {
            shell_error(sh, "Invalid on-time '%s': use 1..60000 ms", argv[1]);
            return -EINVAL;
        }
    }

    errno = 0;
    requested = strtoul(argv[1], &end, 10);
    if (errno == ERANGE || *end != '\0' ||
        requested < 1UL || requested > 60000UL)
    {
        shell_error(sh, "On-time out of range: 1..60000 ms");
        return -EINVAL;
    }
    ret = get_led_sensor(sh, &dev);
    if (ret < 0)
    {
        return ret;
    }

    ret = led_sensor_set_on_time(dev, (uint32_t)requested);
    if (ret < 0)
    {
        shell_error(sh, "led_sensor_set_on_time failed: %d", ret);
        return ret;
    }

    shell_print(sh, "LED on-time set to %lu ms", requested);
    return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(sub_led_sensor,
                               SHELL_CMD(fetch, NULL, "Fetch sample (LED ON)", cmd_sensor_fetch),
                               SHELL_CMD(read, NULL, "Read state and turn LED OFF", cmd_sensor_read),
                               SHELL_CMD(info, NULL, "show LED sensor device name and readiness", cmd_sensor_info),
                               SHELL_CMD_ARG(set, NULL, "Set LED on-time in ms (1..60000 ms)", cmd_sensor_set, 2, 0),
                               SHELL_SUBCMD_SET_END);

SHELL_CMD_REGISTER(lsensor, &sub_led_sensor, "Custom LED sensor commands", NULL);
