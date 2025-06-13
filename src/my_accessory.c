#include <homekit/homekit.h>
#include <homekit/characteristics.h>
#include <math.h>

static void hsi2rgb(float h, float s, float i, uint8_t *led_r, uint8_t *led_g, uint8_t *led_b);

extern void ws2812b_fx_set(bool, uint8_t, uint8_t, uint8_t, uint8_t);

homekit_server_config_t config;

struct
{
    bool on;

    uint8_t brightness;
    float hue;
    float saturation;
} device_state;

void device_identify(homekit_value_t value)
{
    printf("device_identify: %d\n", value.bool_value);
}

homekit_value_t device_on_get()
{
    printf("device_on_get: %d\n", device_state.on);

    return HOMEKIT_BOOL(device_state.on);
}

void device_on_set(homekit_value_t value)
{
    device_state.on = value.bool_value;

    printf("device_on_set: %d\n", device_state.on);

    uint8_t led_r, led_g, led_b;

    hsi2rgb(device_state.hue, device_state.saturation, device_state.brightness, &led_r, &led_g, &led_b);

    ws2812b_fx_set(device_state.on, device_state.brightness, led_r, led_g, led_b);
}

homekit_value_t device_brightness_get()
{
    printf("device_brightness_get: %d\n", device_state.brightness);

    return HOMEKIT_INT(device_state.brightness);
}

void device_brightness_set(homekit_value_t value)
{
    device_state.brightness = value.int_value;

    printf("device_brightness_set: %d\n", device_state.brightness);

    uint8_t led_r, led_g, led_b;

    hsi2rgb(device_state.hue, device_state.saturation, device_state.brightness, &led_r, &led_g, &led_b);

    ws2812b_fx_set(device_state.on, device_state.brightness, led_r, led_g, led_b);
}

homekit_value_t device_hue_get()
{
    printf("device_hue_get: %f\n", device_state.hue);

    return HOMEKIT_FLOAT(device_state.hue);
}

void device_hue_set(homekit_value_t value)
{
    device_state.hue = value.float_value;

    printf("device_hue_set: %f\n", device_state.hue);

    uint8_t led_r, led_g, led_b;

    hsi2rgb(device_state.hue, device_state.saturation, device_state.brightness, &led_r, &led_g, &led_b);

    ws2812b_fx_set(device_state.on, device_state.brightness, led_r, led_g, led_b);
}

homekit_value_t device_saturation_get()
{
    printf("device_saturation_get: %f\n", device_state.saturation);

    return HOMEKIT_FLOAT(device_state.saturation);
}

void device_saturation_set(homekit_value_t value)
{
    device_state.saturation = value.float_value;

    printf("device_saturation_set: %f\n", device_state.saturation);

    uint8_t led_r, led_g, led_b;

    hsi2rgb(device_state.hue, device_state.saturation, device_state.brightness, &led_r, &led_g, &led_b);

    ws2812b_fx_set(device_state.on, device_state.brightness, led_r, led_g, led_b);
}

#define LED_RGB_SCALE 255

//http://blog.saikoled.com/post/44677718712/how-to-convert-from-hsi-to-rgb-white
static void hsi2rgb(float h, float s, float i, uint8_t *led_r, uint8_t *led_g, uint8_t *led_b)
{
    int r, g, b;

    // cycle h around to 0-360 degrees
    while (h < 0)
    {
        h += 360.0F;
    };

    while (h >= 360)
    {
        h -= 360.0F;
    };

    // convert to radians
    h = 3.14159F*h / 180.0F;

    // from percentage to ratio
    s /= 100.0F;

    // from percentage to ratio
    i /= 100.0F;

    // clamp s and i to interval [0,1]
    s = s > 0 ? (s < 1 ? s : 1) : 0;

    // clamp s and i to interval [0,1]
    i = i > 0 ? (i < 1 ? i : 1) : 0;

    // shape intensity to have finer granularity near 0
    i = i * sqrt(i);

    if (h < 2.09439)
    {
        r = LED_RGB_SCALE * i / 3 * (1 + s * cos(h) / cos(1.047196667 - h));
        g = LED_RGB_SCALE * i / 3 * (1 + s * (1 - cos(h) / cos(1.047196667 - h)));
        b = LED_RGB_SCALE * i / 3 * (1 - s);
    }
    else if (h < 4.188787)
    {
        h = h - 2.09439;
        g = LED_RGB_SCALE * i / 3 * (1 + s * cos(h) / cos(1.047196667 - h));
        b = LED_RGB_SCALE * i / 3 * (1 + s * (1 - cos(h) / cos(1.047196667 - h)));
        r = LED_RGB_SCALE * i / 3 * (1 - s);
    }
    else
    {
        h = h - 4.188787;
        b = LED_RGB_SCALE * i / 3 * (1 + s * cos(h) / cos(1.047196667 - h));
        r = LED_RGB_SCALE * i / 3 * (1 + s * (1 - cos(h) / cos(1.047196667 - h)));
        g = LED_RGB_SCALE * i / 3 * (1 - s);
    }

    *led_r = (uint8_t) r;
    *led_g = (uint8_t) g;
    *led_b = (uint8_t) b;
}

homekit_server_config_t config =
{
    .accessories = (homekit_accessory_t *[]){
        HOMEKIT_ACCESSORY(
            .id = 1,
            .category = homekit_accessory_category_lightbulb,
            .services = (homekit_service_t*[])
            {
                HOMEKIT_SERVICE(
                    ACCESSORY_INFORMATION,
                    .characteristics = (homekit_characteristic_t*[])
                    {
                        HOMEKIT_CHARACTERISTIC(NAME, "HomeKitWS2812b"),
                        HOMEKIT_CHARACTERISTIC(MANUFACTURER, "Andrei Gavrila"),
                        HOMEKIT_CHARACTERISTIC(SERIAL_NUMBER, "200000000000"),
                        HOMEKIT_CHARACTERISTIC(MODEL, "HomeKit WS2812b"),
                        HOMEKIT_CHARACTERISTIC(FIRMWARE_REVISION, "0.1"),
                        HOMEKIT_CHARACTERISTIC(IDENTIFY, device_identify),
                        NULL
                    }
                ),
                HOMEKIT_SERVICE(
                    LIGHTBULB,
                    .primary = true,
                    .characteristics = (homekit_characteristic_t*[])
                    {
                        HOMEKIT_CHARACTERISTIC(NAME, "WS2812b"),
                        HOMEKIT_CHARACTERISTIC(ON, true, .getter = device_on_get, .setter = device_on_set),
                        HOMEKIT_CHARACTERISTIC(BRIGHTNESS, 5, .getter = device_brightness_get, .setter = device_brightness_set),
                        HOMEKIT_CHARACTERISTIC(HUE, 0, .getter = device_hue_get, .setter = device_hue_set),
                        HOMEKIT_CHARACTERISTIC(SATURATION, 0, .getter = device_saturation_get, .setter = device_saturation_set),
                        NULL
                    }
                ),
                NULL
            }
        ),
        NULL
    },
    .password = "111-11-111"
};

void my_acessory_init(void)
{
    device_state.on = false;

    device_state.brightness = 10;     // 10%
    device_state.hue = 0.0f;          // Red (0 degrees)
    device_state.saturation = 100.0f; // Red (100% distance)

    uint8_t led_r, led_g, led_b;

    hsi2rgb(device_state.hue, device_state.saturation, device_state.brightness, &led_r, &led_g, &led_b);

    ws2812b_fx_set(device_state.on, device_state.brightness, led_r, led_g, led_b);
}
