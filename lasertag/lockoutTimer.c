
#include "lockoutTimer.h"
#include "intervalTimer.h"
#include "utils.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

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

#define TWENTY_MS_DELAY 20
#define INIT_ZERO 0

// The lockoutTimer is active for 1/2 second once it is started.
// It is used to lock-out the detector once a hit has been detected.
// This ensures that only one hit is detected per 1/2-second interval.

enum lockoutTimer_st_t {
    INIT_ST,
    IDLE_ST,
    LOCKOUT_ST
};

volatile static enum lockoutTimer_st_t currentState;
volatile static uint32_t lockoutCount;
volatile static bool startTimer, timerRunning;

// Perform any necessary inits for the lockout timer.
void lockoutTimer_init() {
    currentState = INIT_ST;
    lockoutCount = INIT_ZERO;
    startTimer = false;
    timerRunning = false;
}

// Standard tick function.
void lockoutTimer_tick() {
    
    // Perform state update first
    switch (currentState) {
        case INIT_ST:
            currentState = IDLE_ST;
            break;
        case IDLE_ST:
            // The timer starts when the player has been shot
            if(startTimer) {
                currentState = LOCKOUT_ST;
                timerRunning = true;
                startTimer = false;
            }
            break;
        case LOCKOUT_ST:
            // Only changes if it has been long enough since the player has been shot
            if(lockoutCount >= LOCKOUT_TIMER_EXPIRE_VALUE) {
                currentState = IDLE_ST;
                timerRunning = false;
                lockoutCount = INIT_ZERO;  
            }
            break;
    }

    // Perform the actions for each of the states
    switch (currentState) {
        case INIT_ST:
            break;
        case IDLE_ST:
            break;
        case LOCKOUT_ST:
            lockoutCount++;
            break;
    }
}

// Calling this starts the timer.
void lockoutTimer_start() {
    startTimer = true;
}

// Returns true if the timer is running.
bool lockoutTimer_running() {
    return timerRunning;
}

// Test function assumes interrupts have been completely enabled and
// lockoutTimer_tick() function is invoked by isr_function().
// Prints out pass/fail status and other info to console.
// Returns true if passes, false otherwise.
// This test uses the interval timer to determine correct delay for
// the interval timer.
bool lockoutTimer_runTest() {
    printf("Running lockout timer test\n");
    intervalTimer_init(INTERVAL_TIMER_TIMER_1);
    intervalTimer_start(INTERVAL_TIMER_TIMER_1);
    lockoutTimer_start();
    utils_msDelay(TWENTY_MS_DELAY);
    //keep the test going until we decide to stop it 
    while (lockoutTimer_running()) {
    }
    intervalTimer_stop(INTERVAL_TIMER_TIMER_1);
    double duration = intervalTimer_getTotalDurationInSeconds(INTERVAL_TIMER_TIMER_1);
    printf("Timer ran for %f seconds\n", duration);
    return true;
}