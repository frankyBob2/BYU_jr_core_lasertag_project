#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <interrupts.h>
#include "detector.h"
#include "buffer.h"
#include "filter.h"
#include "lockoutTimer.h"
#include "hitLedTimer.h"
#include "utils.h"
#include "sound.h"
#include "trigger.h"
#include "runningModes.h"

#define SCALED_ADC_FACTOR 2047.5
#define SCALED_ADC_RANGE 1
#define INPUT_BEFORE_CALCULATION 10
#define NUM_PLAYERS 10
#define SET_TO_ZERO 0
#define MAX_ARRAY_INDEX 9
#define MEDIAN_INDEX 4
#define FUDGE_FACTOR 0
#define INCREMENT 1
#define THRESHOLD_FACTOR .1
#define FUDGE_FACTOR_ARRAY_SIZE 5
#define TOTAL_LIVES 1
#define HITS_PER_LIFE 1
#define FIVE_SECOND_DELAY 5000

typedef uint16_t detector_hitCount_t;
static uint32_t elementCount;
static uint8_t sampleCnt;
static bool hitDetected;
static uint32_t frequencyDetected;
static double powers[NUM_PLAYERS];
static double fudgeFactors[FUDGE_FACTOR_ARRAY_SIZE] = {100, 450, 600, 800, 1000};
static uint32_t fudgeFactorIndex;
static bool ignoredSignals[NUM_PLAYERS];
static detector_hitCount_t hitCounts[NUM_PLAYERS];
static uint32_t invocationCount;
static uint16_t lives;
static uint16_t ownFrequency;
static bool frozen;

void hit_detect();
void detector_makeSounds();

// Initialize the detector module.
// By default, all frequencies are considered for hits.
// Assumes the filter module is initialized previously.
void detector_init(void) {
    printf("initializing detector\n");
    // Initialize arrays to all 0s
    for (uint16_t i = 0; i < NUM_PLAYERS; i++) {
        ignoredSignals[i] = false;
        hitCounts[i] = SET_TO_ZERO;
    }
    // Initialize variables to zero
    invocationCount = SET_TO_ZERO;
    elementCount = SET_TO_ZERO;
    sampleCnt = SET_TO_ZERO;
    hitDetected = false;
    fudgeFactorIndex = FUDGE_FACTOR;
    lives = TOTAL_LIVES;
    frozen = false;

    bool teamB = (runningModes_getFrequencySetting() % 2);
    // set transmitter frequency
    if(teamB) {
        ownFrequency = 8;
    }
    else {
        ownFrequency = 4;
    }
}

// freqArray is indexed by frequency number. If an element is set to true,
// the frequency will be ignored. Multiple frequencies can be ignored.
// Your shot frequency (based on the switches) is a good choice to ignore.
void detector_setIgnoredFrequencies(bool freqArray[]) {
    // Iterate through the array to see what frequencies are true or false
    for (uint16_t i = 0; i < NUM_PLAYERS; i++) {
        ignoredSignals[i] = freqArray[i];
    }
}

// Runs the entire detector: decimating FIR-filter, IIR-filters,
// power-computation, hit-detection. If interruptsCurrentlyEnabled = true,
// interrupts are running. If interruptsCurrentlyEnabled = false you can pop
// values from the ADC buffer without disabling interrupts. If
// interruptsCurrentlyEnabled = true, do the following:
// 1. disable interrupts.
// 2. pop the value from the ADC buffer.
// 3. re-enable interrupts.
// Ignore hits on frequencies specified with detector_setIgnoredFrequencies().
// Assumption: draining the ADC buffer occurs faster than it can fill.
void detector(bool interruptsCurrentlyEnabled) {
    invocationCount++;
    elementCount = buffer_elements();
    uint32_t rawAdcValue;
    // Iterate through each item in the buffer
    for (uint32_t i = 0; i < elementCount; i++) {
        // Get the rawAdcValue if interrupts are enabled
        if (interruptsCurrentlyEnabled) {
            interrupts_disableArmInts();
            rawAdcValue = buffer_pop();
            interrupts_enableArmInts();
            // printf("raw value: %d\n", rawAdcValue);
        }
        // Get adcvalue withoug diabling interrupts
        else {
            rawAdcValue = buffer_pop();
            // printf("raw value: %d\n", rawAdcValue);
        }
        double scaledAdcValue = (((double)rawAdcValue / SCALED_ADC_FACTOR) - SCALED_ADC_RANGE);
        // printf("scaled value: %f\n", scaledAdcValue);
        filter_addNewInput(scaledAdcValue);
        sampleCnt++;
        // This is where the decimation happens, every tenth sample
        if (sampleCnt == INPUT_BEFORE_CALCULATION) {
            uint16_t filterNumber;
            sampleCnt = SET_TO_ZERO;                // Reset the sample count.
            filter_firFilter();                     // Runs the FIR filter, output goes in the y-queue.
            // Run all the IIR filters and compute power in each of the output queues.
            for (filterNumber = 0; filterNumber < FILTER_FREQUENCY_COUNT; filterNumber++) {
                filter_iirFilter(filterNumber);     // Run each of the IIR filters.
                // Compute the power for each of the filters, at lowest computational cost.
                // 1st false means do not compute from scratch.
                // 2nd false means no debug prints.
                filter_computePower(filterNumber, false, false);
            }
            // This determines if it has been enough time since the last hit we detected
            if (!lockoutTimer_running()) {
                hit_detect();
            }
        }
    }
}

// Helpter function that implements the algorithm to detect a hit
void hit_detect() {
    // printf("detecting hit\n");
    double tempArrayValues[NUM_PLAYERS];            // Create a temporary array of power values
    uint16_t tempArrayIndeces[NUM_PLAYERS];         // Create a temporary array of indeces to keep track of the player with highest power
    // Initialize the index array with index values
    for (uint8_t i = 0; i < NUM_PLAYERS; i++) {    
        tempArrayIndeces[i] = i;
    }

    uint16_t minIndex = SET_TO_ZERO;
    uint16_t lowestValue = SET_TO_ZERO;

    // This returns the power values from the array
    for(uint16_t i = 0; i < NUM_PLAYERS; i++) {
        tempArrayValues[i] = filter_getCurrentPowerValue(i);
        // printf("%f\n", tempArrayValues[i]);
    }

    // This is the algorithm to get the array in order from lowest power to highest power
    for (uint16_t i = 0; i < MAX_ARRAY_INDEX; i++) {
        minIndex = i;
        // Iterate through array to see what needs to be swapped
        for (uint16_t j = i + INCREMENT; j < NUM_PLAYERS; j++) {
            // Update min index if it is smaller
            if (tempArrayValues[j] < tempArrayValues[minIndex]) {
                minIndex = j;
            }
        }
        double temp1 = tempArrayValues[i];
        tempArrayValues[i] = tempArrayValues[minIndex];
        tempArrayValues[minIndex] = temp1;
        
        uint16_t temp2 = tempArrayIndeces[i];
        tempArrayIndeces[i] = tempArrayIndeces[minIndex];
        tempArrayIndeces[minIndex] = temp2;
    }

    // Calculate the median power value and threshold power
    frequencyDetected = tempArrayIndeces[MAX_ARRAY_INDEX];
    double median = tempArrayValues[MEDIAN_INDEX];
    double threshold = median*fudgeFactors[fudgeFactorIndex] + THRESHOLD_FACTOR;
   

    // Determine whether a player hit us or not and what player it was
    if((tempArrayValues[MAX_ARRAY_INDEX] > threshold) && !ignoredSignals[frequencyDetected] && !lockoutTimer_running()) {
        if(frozen) {
            if(frequencyDetected == ownFrequency) {
                lives = TOTAL_LIVES;
                hitDetected = true;
                trigger_enable();
                hitLedTimer_start();
                lockoutTimer_start();
                sound_setSound(sound_gameStart_e);
                sound_startSound();
                hitCounts[frequencyDetected]++;
                frozen = false;
            }
        }
        else {
            if(frequencyDetected != ownFrequency) {
                lives = 0;
                hitDetected = true;
                frozen = true;
                lockoutTimer_start();
                hitLedTimer_start();
                sound_setSound(sound_loseLife_e);
                sound_startSound();
                trigger_disable();
                hitCounts[frequencyDetected]++;
            }
        }
        
    }
    //Otherwise hitDetected is false
    else {
        hitDetected = false;
    }
}

// Returns true if a hit was detected.
bool detector_hitDetected(void) {
    return hitDetected;
}

// Returns the frequency number that caused the hit.
uint16_t detector_getFrequencyNumberOfLastHit(void) {
    return frequencyDetected;
}

// Clear the detected hit once you have accounted for it.
void detector_clearHit(void) {
    hitDetected = false;
}

// Ignore all hits. Used to provide some limited invincibility in some game
// modes. The detector will ignore all hits if the flag is true, otherwise will
// respond to hits normally.
void detector_ignoreAllHits(bool flagValue) {
    // Iterate throufh players to set if they are ignored or not
    for (uint16_t i = 0; i < NUM_PLAYERS; i++) {
        ignoredSignals[i] = flagValue;
    }
}

// Get the current hit counts.
// Copy the current hit counts into the user-provided hitArray
// using a for-loop.
void detector_getHitCounts(detector_hitCount_t hitArray[]) {
    // Iterate throughplayers and return hit counts
    for(uint16_t i = 0; i < NUM_PLAYERS; i++) {
        hitArray[i] = hitCounts[i];
    }
}

// Allows the fudge-factor index to be set externally from the detector.
// The actual values for fudge-factors is stored in an array found in detector.c
void detector_setFudgeFactorIndex(uint32_t factor) {
    fudgeFactorIndex = factor;
}

// Returns the detector invocation count.
// The count is incremented each time detector is called.
// Used for run-time statistics.
uint32_t detector_getInvocationCount(void) {
    return invocationCount;
}

uint16_t detector_getLives() {
    return lives;
}

void detector_setOwnFrequency(uint16_t playerNum) {
    ownFrequency = playerNum;
}


/******************************************************
******************** Test Routines ********************
******************************************************/

// Students implement this as part of Milestone 3, Task 3.
// Create two sets of power values and call your hit detection algorithm
// on each set. With the same fudge factor, your hit detect algorithm
// should detect a hit on the first set and not detect a hit on the second.
void detector_runTest(void) {

    // detector_init();
    // detector_setFudgeFactorIndex(SET_TO_ZERO);
    // //setting our first test double
    // double testData1[NUM_PLAYERS] = {150, 20, 40, 10, 15, 30, 35, 15, 25, 80};
    // // Set power values with test data 1
    // for (uint16_t i = 0; i < NUM_PLAYERS; i++) {
    //     filter_setCurrentPowerValue(i, testData1[i]);
    // }

    // hit_detect();
    // bool result1 = detector_hitDetected();
    // //setting our second test double
    // double testData2[NUM_PLAYERS] = {10, 20, 15, 10, 15, 10, 20, 15, 10, 15};
    // // Set power values with test data 2
    // for (int i = 0; i < NUM_PLAYERS; i++) {
    //     filter_setCurrentPowerValue(i, testData2[i]);
    // }

    // hit_detect();
    // bool result2 = detector_hitDetected();

    // printf("Result 1: %d\n", result1);
    // printf("Result 2: %d\n", result2);
}   

// This function was made to play all of the sounds associated with detecting shots
// i.e. hit sound and lose a life sound
void detector_makeSounds()
{
    // As long as we have not lost a life play the hit sound
    if((lives % HITS_PER_LIFE)!=0) {
        sound_setSound(sound_hit_e);
        sound_startSound();
    }
    else {
        // If we have not lost all of our lives play the lost a life sound when 
        if(lives != 0) {
            sound_setSound(sound_loseLife_e);
            sound_startSound();
            trigger_disable();
            // When you lose a life you cannot be shot for 5 seconds
            bool ignoredSignalsCopy[NUM_PLAYERS];
            //setting what ignored signals are based off input array
            for(uint16_t i = 0; i < NUM_PLAYERS; i++) {
                ignoredSignalsCopy[i] = ignoredSignals[i];
                if(ignoredSignals[i] == false) 
                    ignoredSignals[i] = true;
            }
            // 5 second delay before you can shoot again
            utils_msDelay(FIVE_SECOND_DELAY);
            trigger_enable();
            detector_setIgnoredFrequencies(ignoredSignalsCopy);
        }
    }
}

//coef.txt currently does not have the FIR filter for the brain ball project currently attached to it.