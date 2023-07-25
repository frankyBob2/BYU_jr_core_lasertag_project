
#include "hitLedTimer.h"
#include <stdbool.h>
#include "mio.h"
#include "leds.h"
#include "utils.h"
#include "buttons.h"
#define LED_OUTPUT_PIN 11
#define LED_HIGH_VALUE 1
#define LED_LOW_VALUE 0
#define ON_TICKS 50000
#define LED_ON 1
#define LED_OFF 0
#define INFINITE_LOOP 1
#define TEST_DELAY 2000
#define BOUNCE_DELAY 5

// Uncomment for debug prints
#define DEBUG
 
#if defined(DEBUG)
#include <stdio.h>
#include "xil_printf.h"
#define DPRINTF(...) printf(__VA_ARGS__)
#define DPCHAR(ch) outbyte(ch)
#else
#define DPRINTF(...)
#define DPCHAR(ch)
#endif

// SM States
typedef enum
{
    LED_INIT_ST,
    LED_IDLE_ST,
    LED_ON_ST
} hitLed_state_t;

volatile static hitLed_state_t hitLedState;
volatile static uint32_t onCount;
volatile static bool disabled, startTimer;

// The hitLedTimer is active for 1/2 second once it is started.
// While active, it turns on the LED connected to MIO pin 11
// and also LED LD0 on the ZYBO board.
void led_set_jf3(uint8_t val)
{
    mio_writePin(LED_OUTPUT_PIN, val);
}

// Need to init things.
void hitLedTimer_init()
{
    mio_init(false);
    mio_setPinAsOutput(LED_OUTPUT_PIN);
    leds_init(true);
    hitLedState = LED_INIT_ST;
    onCount = 0;
    disabled = false;
    startTimer = false;
}

// Standard tick function.
void hitLedTimer_tick()
{
    // SM Transitions
    switch (hitLedState)
    {
    case LED_INIT_ST:
        hitLedState = LED_IDLE_ST;
        break;
    case LED_IDLE_ST:
        // Executes if the timer is started and hitledtimer is high
        if (startTimer && !disabled) {
            hitLedState = LED_ON_ST;
            led_set_jf3(LED_ON);
            hitLedTimer_turnLedOn();
            startTimer = false;
        }
        break;
    case LED_ON_ST:
        // Once the count is equal to half a second this will execute
        if (onCount >= ON_TICKS)
        {
            hitLedState = LED_IDLE_ST;
            led_set_jf3(LED_OFF);
            hitLedTimer_turnLedOff();
            onCount = 0;
        }
        break;
    }

    // SM Actions
    switch (hitLedState)
    {
    case LED_INIT_ST:
        break;
    case LED_IDLE_ST:
        break;
    case LED_ON_ST:
        onCount++;
        break;
    }
}

// Calling this starts the timer.
void hitLedTimer_start()
{
    if (!disabled)
        startTimer = true;
}

// Returns true if the timer is currently running.
bool hitLedTimer_running()
{
    return (hitLedState == LED_ON_ST);
}

// Turns the gun's hit-LED on.
void hitLedTimer_turnLedOn()
{
    leds_write(LED_ON);
}

// Turns the gun's hit-LED off.
void hitLedTimer_turnLedOff()
{
    leds_write(LED_OFF);
}

// Disables the hitLedTimer.
void hitLedTimer_disable()
{
    disabled = true;
}

// Enables the hitLedTimer.
void hitLedTimer_enable()
{
    disabled = false;
}

// Runs a visual test of the hit LED until BTN3 is pressed.
// The test continuously blinks the hit-led on and off.
// Depends on the interrupt handler to call tick function.
void hitLedTimer_runTest() {
    printf("running hitLedTimer test\n");
    hitLedTimer_init();
    // When button 3 isn't pressed this will run the test
    while(!(buttons_read() & BUTTONS_BTN3_MASK)) {
        hitLedTimer_start();
        while (hitLedTimer_running()) {}
        utils_msDelay(TEST_DELAY);
    }
    do
    {
        utils_msDelay(BOUNCE_DELAY);
    } while (buttons_read());
}
