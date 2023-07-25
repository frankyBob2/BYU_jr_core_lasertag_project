#include "isr.h"
#include "transmitter.h"
#include "trigger.h"
#include "hitLedTimer.h"
#include "lockoutTimer.h"
#include "buffer.h"
#include "interrupts.h"
#include "sound.h"
#include "game.h"

// The interrupt service routine (ISR) is implemented here.
// Add function calls for state machine tick functions and
// other interrupt related modules.

// Perform initialization for interrupt and timing related modules.
void isr_init()
{
    transmitter_init();
    trigger_init();
    hitLedTimer_init();
    lockoutTimer_init();
    buffer_init();
    sound_init();
}

// This function is invoked by the timer interrupt at 100 kHz.
void isr_function()
{
    transmitter_tick();
    trigger_tick();
    hitLedTimer_tick();
    lockoutTimer_tick();
    buffer_pushover(interrupts_getAdcData());
    sound_tick();
    bluetooth_isr_function();
}