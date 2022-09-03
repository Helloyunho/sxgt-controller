// Slightly modified for multiple pins
#include "Adafruit_TinyUSB_Arduino/src/Adafruit_TinyUSB.h"
#include "TinyUSB_Mouse_and_Keyboard/TinyUSB_Mouse_and_Keyboard.h"
#include "hardware/gpio.h"
#include "includes/Adafruit_USBD_CDC-stub.h"
#include "includes/usb.h"
#include "pico/binary_info.h"
#include "pico/stdlib.h"

// GPIO pins the keyswitch is on
#define PINS \
    { 0, 1, 2, 3, 4, 5, 6, 7 }

// Debounce delay (ms)
#define DEBOUNCE_DELAY 5

// Adafruit TinyUSB instance
extern Adafruit_USBD_Device TinyUSBDevice;

uint8_t KEYS[8] = {'d', 'f', 'g', 'h', 'j', 'c', 'b', KEY_RETURN};

int main() {
    bi_decl(bi_program_description("Sixtar Gate: STARTRAIL Controller"));
    bi_decl(bi_program_feature("USB HID Device"));
    TinyUSBDevice.begin();  // Initialise Adafruit TinyUSB

    // Initialise a keyboard (code will wait here to be plugged in)
    Keyboard.begin();

    // Initise GPIO pin as input with pull-up
    for (int pin : PINS) {
        gpio_init(pin);
        gpio_set_dir(pin, GPIO_IN);
        gpio_pull_up(pin);
    }

    // Variables for detecting key press
    bool lastState[8] = {true, true, true, true,
                         true, true, true, true};  // pulled up by default
    uint32_t lastTime[8] = {to_ms_since_boot(get_absolute_time()),
                            to_ms_since_boot(get_absolute_time()),
                            to_ms_since_boot(get_absolute_time()),
                            to_ms_since_boot(get_absolute_time()),
                            to_ms_since_boot(get_absolute_time()),
                            to_ms_since_boot(get_absolute_time()),
                            to_ms_since_boot(get_absolute_time()),
                            to_ms_since_boot(get_absolute_time())};
    // i mean this works too

    // Main loop
    while (1) {
        // Check GPIO pin, and if more than DEBOUNCE_DELAY ms have passed since
        // the key changed press release key depending on value (delay is for
        // debounce, ie to avoid rapid changes to switch value)
        for (int pin : PINS) {
            bool state = gpio_get(pin);
            uint8_t key = KEYS[pin];
            uint32_t now = to_ms_since_boot(get_absolute_time());
            if ((now - lastTime[pin] > DEBOUNCE_DELAY) &&
                state != lastState[pin]) {
                if (state)  // The pin is pulled up by default, so the logic is
                            // backwards
                    Keyboard.release(key);  // and true is released
                else
                    Keyboard.press(key);
                lastTime[pin] = now;
                lastState[pin] = state;
            }
        }
    }
}
