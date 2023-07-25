#include "trigger.h"
#include "buttons.h"
#include "transmitter.h"
#include <stdint.h>
#include "mio.h"
#include "utils.h"
#include "sound.c"
#define TRIGGER_INPUT_PIN 10
#define DEBOUNCE_TICKS 5000
#define RELOAD_TICKS 300000
#define GUN_TRIGGER_PRESSED 1
#define BOUNCE_DELAY 5
#define SHOTS_PER_CLIP 10

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

// The trigger state machine debounces both the press and release of gun
// trigger. Ultimately, it will activate the transmitter when a debounced press
// is detected.

// SM Statesf
typedef enum
{
    TRIGGER_INIT_ST,
    TRIGGER_DISABLED_ST,
    TRIGGER_IDLE_ST,
    TRIGGER_COUNT_1_ST,
    TRIGGER_PRESSED_ST,
    TRIGGER_COUNT_2_ST,
    TRIGGER_RELOAD_ST
} trigger_state_t;

volatile static trigger_state_t triggerState;
volatile static uint32_t waitCount;
volatile static uint32_t reloadCount;
volatile static bool disabled;
volatile static bool reloadFlag;
volatile static trigger_shotsRemaining_t remainingShots;


// Returns true if the button is pressed
bool triggerPressed()
{
    return ((!disabled & (mio_readPin(TRIGGER_INPUT_PIN) == GUN_TRIGGER_PRESSED)) ||
            (buttons_read() & BUTTONS_BTN0_MASK));
}

// Init trigger data-structures.
// Initializes the mio subsystem.
// Determines whether the trigger switch of the gun is connected
// (see discussion in lab web pages).
void trigger_init()
{
    mio_setPinAsInput(TRIGGER_INPUT_PIN);
    // If the trigger is pressed when trigger_init() is called, assume that the gun is not connected and ignore it.
    if (triggerPressed())
    {
        disabled = true;
    }
    disabled = true;
    reloadFlag = false;
    waitCount = 0;
    reloadCount = 0;
    remainingShots = SHOTS_PER_CLIP;
}

// Standard tick function.
void trigger_tick()
{
    // SM Transitions
    switch (triggerState)
    {
        case TRIGGER_INIT_ST:
            triggerState = TRIGGER_DISABLED_ST;
            break;
        case TRIGGER_DISABLED_ST:
            // Conditions for leaving disabled state
            if (!disabled && (remainingShots > 0))
            {
                triggerState = TRIGGER_IDLE_ST;
            }
            break;
        case TRIGGER_IDLE_ST:
            // This is the state the gun remains in when no actions are actively taken
            if (disabled)
                triggerState = TRIGGER_DISABLED_ST;
            else if (remainingShots == 0) {
                triggerState = TRIGGER_RELOAD_ST;
                reloadCount = 0;
            }
            // If the trigger is pressed then start debouncing
            else if (triggerPressed())
            {
                triggerState = TRIGGER_COUNT_1_ST;
            }
            break;
        case TRIGGER_COUNT_1_ST:
            // If the trigger is released go back to idle
            if (!triggerPressed())
            {
                triggerState = TRIGGER_IDLE_ST;
            }
            // After waiting long enough, move to pressed state
            else if (waitCount >= DEBOUNCE_TICKS) {
                triggerState = TRIGGER_PRESSED_ST;
                waitCount = 0;
                // Decrement remaining shots and play the gunFire sound when the trigger is pulled
                if(remainingShots) {
                    remainingShots--;
                    sound_setSound(sound_gunFire_e);
                    transmitter_run();
                }
                else {
                    sound_setSound(sound_gunClick_e);
                }
                waitCount = 0;
                sound_startSound();
                DPCHAR('U');
                DPCHAR('\n');
                DPCHAR('D');
                DPCHAR('\n');
            }
            break;
        case TRIGGER_PRESSED_ST:
            // When trigger is released go to next state
            if(reloadCount > RELOAD_TICKS) {
                triggerState = TRIGGER_COUNT_2_ST;
                sound_setSound(sound_gunReload_e);
                // Reload the gun with a new clip when 10 shots have been shot
                if(sound_isSoundComplete()){
                  sound_setSound(sound_gunReload_e);
                  sound_startSound();
                  }
                remainingShots = SHOTS_PER_CLIP;
                reloadCount = 0;
            }
            //if trigger's still not pressed, don't reload
            if (!triggerPressed())
            {
                triggerState = TRIGGER_COUNT_2_ST;
                reloadCount = 0;
            }
            break;
        case TRIGGER_COUNT_2_ST:
            // If trigger gets re pressed go back
            if (triggerPressed())
            {
                triggerState = TRIGGER_PRESSED_ST;
            }
            // Move back to idle if you've waited long enough
            else if (waitCount >= DEBOUNCE_TICKS)
            {
                triggerState = TRIGGER_IDLE_ST;
            }
            break;
        case TRIGGER_RELOAD_ST:
            // When you have shot 10 times reload the gun and play the reload sound
            if(reloadCount > RELOAD_TICKS){
                sound_setSound(sound_gunReload_e);
                reloadCount = 0;
                sound_startSound();
                triggerState = TRIGGER_DISABLED_ST;
                remainingShots = SHOTS_PER_CLIP;
            // If the trigger is pressed while reloading play the click sound
            } else if(triggerPressed()){
                sound_setSound(sound_gunClick_e);
                sound_startSound();
            }

    }

    // SM Actions
    switch (triggerState)
    {
        case TRIGGER_INIT_ST:
            break;
        case TRIGGER_DISABLED_ST:
            break;
        case TRIGGER_IDLE_ST:
            break;
        case TRIGGER_COUNT_1_ST:
            waitCount++;
            break;
        case TRIGGER_PRESSED_ST:
            reloadCount++; 
            break;
        case TRIGGER_COUNT_2_ST:
            waitCount++;
            break;
        case TRIGGER_RELOAD_ST:
            reloadCount++;
            break;
    }
}

// Enable the trigger state machine. The trigger state-machine is inactive until
// this function is called. This allows you to ignore the trigger when helpful
// (mostly useful for testing).
void trigger_enable()
{
    disabled = false;
}

// Disable the trigger state machine so that trigger presses are ignored.
void trigger_disable()
{
    disabled = true;
}

// Returns the number of remaining shots.
trigger_shotsRemaining_t trigger_getRemainingShotCount()
{
    return remainingShots;
}

// Sets the number of remaining shots.
void trigger_setRemainingShotCount(trigger_shotsRemaining_t count)
{
    remainingShots = count;
}

// Runs the test continuously until BTN3 is pressed.
// The test just prints out a 'D' when the trigger or BTN0
// is pressed, and a 'U' when the trigger or BTN0 is released.
// Depends on the interrupt handler to call tick function.
void trigger_runTest()
{
    printf("running trigger test\n");
    trigger_init();
    buttons_init();
    //sends the trigger running continuously while in game mode
    while(!(buttons_read() & BUTTONS_BTN3_MASK)) {trigger_enable();}
    trigger_disable();
    // Debouce button release
    do
    {
        utils_msDelay(BOUNCE_DELAY);
    } while (buttons_read());
}