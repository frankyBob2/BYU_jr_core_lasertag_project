
#include "transmitter.h"
#include "filter.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "mio.h"
#include "utils.h"
#include "buttons.h"
#include "switches.h"
#include "sound.h"

#define TRANSMITTER_OUTPUT_PIN 13
#define TRANSMITTER_HIGH_VALUE 1
#define TRANSMITTER_LOW_VALUE 0
#define PULSE_TICKS 20000
#define BUTTON3_MASK 8
#define HALF_VALUE 2
#define SHORT_DELAY 400
#define LONG_DELAY 2000
#define INIT_ZERO 0

// SM States
typedef enum
{
    TRANSMITTER_INIT_ST,
    TRANSMITTER_IDLE_ST,
    TRANSMITTER_HIGH_ST,
    TRANSMITTER_LOW_ST
} transmitter_state_t;

volatile static transmitter_state_t transmitterState;
volatile static bool continuousMode, runFlag;
volatile static uint32_t timeOnTicks, timeOnCount, pulseCount;
volatile static uint16_t frequencyNum;

// The transmitter state machine generates a square wave output at the chosen
// frequency as set by transmitter_setFrequencyNumber(). The step counts for the
// frequencies are provided in filter.h

// Write a 1 or 0 to JF-1.
void transmitter_set_jf1(uint8_t val)
{
    mio_writePin(TRANSMITTER_OUTPUT_PIN, val);
}

// Standard init function.
void transmitter_init()
{
    // false disables any debug printing if there is a system failure during init
    mio_init(false);
    mio_setPinAsOutput(TRANSMITTER_OUTPUT_PIN); // Configure the signal direction of the pin to be an output
    transmitterState = TRANSMITTER_INIT_ST;
    continuousMode = false;
    runFlag = false;
    frequencyNum = INIT_ZERO;
    timeOnTicks = filter_frequencyTickTable[frequencyNum] / HALF_VALUE;
    timeOnCount = INIT_ZERO;
    pulseCount = INIT_ZERO;
}

// Standard tick function.
void transmitter_tick()
{
    // SM Transitions
    switch (transmitterState)
    {
    case TRANSMITTER_INIT_ST:
        transmitterState = TRANSMITTER_IDLE_ST;
        break;
    case TRANSMITTER_IDLE_ST:
        // The transmitter stays in this state until runFlag is high
        if (runFlag)
        {
            transmitterState = TRANSMITTER_HIGH_ST;
            transmitter_set_jf1(TRANSMITTER_HIGH_VALUE);
            runFlag = false;
        }
        break;
    case TRANSMITTER_HIGH_ST:
        // changes state based on time count
        if (timeOnCount >= timeOnTicks)
        {
            transmitter_set_jf1(TRANSMITTER_LOW_VALUE);
            transmitterState = TRANSMITTER_LOW_ST;
            timeOnCount = 0;
        }
        break;
    case TRANSMITTER_LOW_ST:
        // If pulse count is high enough it will enter this if-statement
        if (pulseCount >= PULSE_TICKS)
        {
            // If timeOnCount is high enough it will run continuous mode
            if (timeOnCount >= timeOnTicks)
                // Executes continuous mode state changes
                if (continuousMode)
                {
                    timeOnCount = 0;
                    pulseCount = 0;
                    transmitter_set_jf1(TRANSMITTER_HIGH_VALUE);
                    transmitterState = TRANSMITTER_HIGH_ST;
                    timeOnTicks = filter_frequencyTickTable[frequencyNum] / HALF_VALUE;
                }
                // Executes non-continuous mode state changes
                else
                {
                    timeOnCount = 0;
                    pulseCount = 0;
                    transmitter_set_jf1(TRANSMITTER_LOW_VALUE);
                    transmitterState = TRANSMITTER_IDLE_ST;
                    timeOnTicks = filter_frequencyTickTable[frequencyNum] / HALF_VALUE;
                }
        }
        // else default case
        else
        {
            // Default case will go back to high state
            if (timeOnCount >= timeOnTicks)
            {
                transmitter_set_jf1(TRANSMITTER_HIGH_VALUE);
                transmitterState = TRANSMITTER_HIGH_ST;
                timeOnCount = 0;
            }
        }
        break;
    }

    // SM Actions
    switch (transmitterState)
    {
    case TRANSMITTER_INIT_ST:
    //printf("Current State: Init\n");
        break;
    case TRANSMITTER_IDLE_ST:
        timeOnTicks = filter_frequencyTickTable[frequencyNum] / HALF_VALUE;
    //printf("Current State: Idle\n");
        break;
    case TRANSMITTER_HIGH_ST:
    //printf("Current State: High\n");
        timeOnCount++;
        pulseCount++;
        break;
    case TRANSMITTER_LOW_ST:
    //printf("Current State: Low\n");
        timeOnCount++;
        pulseCount++;
        break;
    }
}

// Activate the transmitter.
void transmitter_run()
{
    runFlag = true;
}

// Returns true if the transmitter is still running.
bool transmitter_running()
{
    if (transmitterState == TRANSMITTER_IDLE_ST)
        return false;
    else
        return true;
}

// Sets the frequency number. If this function is called while the
// transmitter is running, the frequency will not be updated until the
// transmitter stops and transmitter_run() is called again.
void transmitter_setFrequencyNumber(uint16_t frequencyNumber)
{
    frequencyNum = frequencyNumber;
}

// Returns the current frequency setting
uint16_t transmitter_getFrequencyNumber()
{
    return frequencyNum;
}

// Runs the transmitter continuously.
// if continuousModeFlag == true, transmitter runs continuously, otherwise, it
// transmits one burst and stops. To set continuous mode, you must invoke
// this function prior to calling transmitter_run(). If the transmitter is
// currently in continuous mode, it will stop running if this function is
// invoked with continuousModeFlag == false. It can stop immediately or wait
// until a 200 ms burst is complete. NOTE: while running continuously,
// the transmitter will only change frequencies in between 200 ms bursts.
void transmitter_setContinuousMode(bool continuousModeFlag)
{
    continuousMode = continuousModeFlag;
}

/******************************************************************************
***** Test Functions
******************************************************************************/

// Prints out the clock waveform to stdio. Terminates when BTN3 is pressed.
// Does not use interrupts, but calls the tick function in a loop.

#define TRANSMITTER_TEST_TICK_PERIOD_IN_MS 1
#define BOUNCE_DELAY 5

// This function is used to make sure the transmitter functions properly
void transmitter_runTest()
{
    printf("starting transmitter_runTest()\n");
    transmitter_init(); // init the transmitter.
    transmitter_tick();
    // If button 3 isn't pressed then this will execute
    while (!(buttons_read() & BUTTON3_MASK))
    {
        uint16_t switchValue = switches_read() % FILTER_FREQUENCY_COUNT; // Compute a safe number from the switches.
        transmitter_setFrequencyNumber(switchValue);                     // set the frequency number based upon switch value.
        transmitter_run();                                            // Start the transmitter.
        transmitter_tick();   
        while (transmitter_running())
        {                                                      // Keep ticking until it is done.
            transmitter_tick();                                // tick.
            utils_msDelay(TRANSMITTER_TEST_TICK_PERIOD_IN_MS); // short delay between ticks.
        }
        transmitter_tick(); 
        transmitter_tick(); 
        transmitter_tick(); 
        transmitter_tick(); 
        transmitter_tick(); 
        transmitter_tick(); 
        transmitter_tick(); 
        printf("frequency ticks: %d\n", filter_frequencyTickTable[switchValue]);
        utils_msDelay(LONG_DELAY);
    }

    // Debounce the button
    do
    {
        utils_msDelay(BOUNCE_DELAY);
    } while (buttons_read());
    printf("exiting transmitter_runTest()\n");
}

// Tests the transmitter in non-continuous mode.
// The test runs until BTN3 is pressed.
// To perform the test, connect the oscilloscope probe
// to the transmitter and ground probes on the development board
// prior to running this test. You should see about a 300 ms dead
// spot between 200 ms pulses.
// Should change frequency in response to the slide switches.
// Depends on the interrupt handler to call tick function.
void transmitter_runTestNoncontinuous()
{

    printf("starting runTestNoncontinuous\n");
    buttons_init();
    switches_init();
    transmitter_init(); // init the transmitter.
    // This will only execute if button 3 is not pressed
    while (!(buttons_read() & BUTTON3_MASK))
    {

        uint16_t switchValue = switches_read() % FILTER_FREQUENCY_COUNT; // Compute a safe number from the switches.
        transmitter_setFrequencyNumber(switchValue);                     // set the frequency number based upon switch value.
        transmitter_run();                                               // Start the transmitter.
        while (transmitter_running())
        { // Keep ticking until it is done.
        }
        utils_msDelay(SHORT_DELAY);
    }
    // Used to debounce the buttons
    do
    {
        utils_msDelay(BOUNCE_DELAY);
    } while (buttons_read());
    transmitter_setContinuousMode(false);
    transmitter_run();
    while(transmitter_running()){}
}

// Tests the transmitter in continuous mode.
// To perform the test, connect the oscilloscope probe
// to the transmitter and ground probes on the development board
// prior to running this test.
// Transmitter should continuously generate the proper waveform
// at the transmitter-probe pin and change frequencies
// in response to changes in the slide switches.
// Test runs until BTN3 is pressed.
// Depends on the interrupt handler to call tick function.
void transmitter_runTestContinuous()
{

    printf("starting runTestContinuous\n");
    buttons_init();
    switches_init();
    transmitter_init();
    transmitter_setContinuousMode(true);
    transmitter_run();
    // Executed if button 3 isn't pressed
    while (!(buttons_read() & BUTTON3_MASK))
    {
        uint16_t switchValue = switches_read() % FILTER_FREQUENCY_COUNT;
        transmitter_setFrequencyNumber(switchValue);  
    }

    // Debounce the button
    do
    {
        utils_msDelay(BOUNCE_DELAY);
    } while (buttons_read());
}
