/*
This software is provided for student assignment use in the Department of
Electrical and Computer Engineering, Brigham Young University, Utah, USA.
Users agree to not re-host, or redistribute the software, in source or binary
form, to other persons or other institutions. Users may modify and use the
source code for personal or educational use.
For questions, contact Brad Hutchings or Jeff Goeders, https://ece.byu.edu/
*/

/*
The code in runningModes.c can be an example for implementing the game here.
*/

#include <stdio.h>

#include "hitLedTimer.h"
#include "interrupts.h"
#include "runningModes.h"
#include "filter.h"
#include "buttons.h"
#include "transmitter.h"
#include "trigger.h"
#include "histogram.h"
#include "lockoutTimer.h"
#include "switches.h"
#include "detector.h"
#include "intervalTimer.h"
#include "sound.h"
#include "utils.h"
#include "bluetooth.h"
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

#define TEAM_A_PLAYER 6
#define TEAM_B_PLAYER 9
#define ISR_CUMULATIVE_TIMER INTERVAL_TIMER_TIMER_0 // Used by the ISR.
#define TOTAL_RUNTIME_TIMER INTERVAL_TIMER_TIMER_1 // Used to compute total run-time.
#define MAIN_CUMULATIVE_TIMER INTERVAL_TIMER_TIMER_2 // Used to compute cumulative run-time in main.
#define INTERRUPTS_CURRENTLY_ENABLED true
#define INTERRUPTS_CURRENTLY_DISABLE false
#define DETECTOR_HIT_ARRAY_SIZE FILTER_FREQUENCY_COUNT // The array contains one location per user frequency.
#define CLIP_SIZE 10
#define DETERMINE_TEAM 2
#define ONE_SECOND_DELAY 1000
#define BLUETOOTH_SERVICE_INTERVAL 500
#define PLAYER 0
#define FROZEN_UNFROZEN 1
#define FROZEN 'f'
#define UNFROZEN 'u'
#define DATA_POSITION 0
#define ONE_READ 1
#define TWO_READS 2
#define PLAYER_ONE_FROZEN '1'
#define PLAYER_TWO_FROZEN '2'
#define PLAYER_THREE_FROZEN '3'
#define PLAYER_FOUR_FROZEN '4'
#define PLAYER_ONE_UNFROZEN 'q'
#define PLAYER_TWO_UNFROZEN 'w'
#define PLAYER_THREE_UNFROZEN 'e'
#define PLAYER_FOUR_UNFROZEN 'r'

// This game supports two teams, Team-A and Team-B.
// Each team operates on its own configurable frequency.
// Each player has a fixed set of lives and once they
// have expended all lives, operation ceases and they are told
// to return to base to await the ultimate end of the game.
// The gun is clip-based and each clip contains a fixed number of shots
// that takes a short time to reload a new clip.
// The clips are automatically loaded.
// Runs until BTN3 is pressed.
void game_twoTeamTag(void) {
 uint16_t hitCount = 0;
 // runningModes_initAll();
 // trigger_enable();                                 // Makes the state machine responsive to the trigger.
 // bool silenceFlag = false;
 // // Configuration of the two teams
 // bool teamB = (runningModes_getFrequencySetting() % DETERMINE_TEAM);
 // bool ignoredFrequencies[FILTER_FREQUENCY_COUNT];
 // //set all frequencies to ignore
 // for (uint16_t i = 0; i < FILTER_FREQUENCY_COUNT; i++) {
 //   ignoredFrequencies[i] = true;
 // }
 // //sets 1 frequency to false based on which team it is. Note to self: rewrite this part to be more efficient in the future.
 // if(teamB) {
 //   transmitter_setFrequencyNumber(TEAM_A_PLAYER);
 //   ignoredFrequencies[TEAM_B_PLAYER] = false;

 // }
 // else {
 //   transmitter_setFrequencyNumber(TEAM_B_PLAYER);
 //   ignoredFrequencies[TEAM_A_PLAYER] = false;

 // }
 // detector_setIgnoredFrequencies(ignoredFrequencies);

 // interrupts_enableTimerGlobalInts();               // Allow timer interrupts.
 // interrupts_startArmPrivateTimer();                // Start the private ARM timer running.
 // intervalTimer_reset(ISR_CUMULATIVE_TIMER);        // Used to measure ISR execution time.
 // intervalTimer_reset(TOTAL_RUNTIME_TIMER);         // Used to measure total program execution time.
 // intervalTimer_reset(MAIN_CUMULATIVE_TIMER);       // Used to measure main-loop execution time.
 // intervalTimer_start(TOTAL_RUNTIME_TIMER);         // Start measuring total execution time.
 // interrupts_enableArmInts();                       // ARM will now see interrupts after this.
 // sound_setVolume(sound_mediumHighVolume_e);        // set volume
 // sound_setSound(sound_gameStart_e);                // set & play game start sound
 // sound_startSound();
 // utils_msDelay(ONE_SECOND_DELAY);
 // lockoutTimer_start();
 // utils_msDelay(ONE_SECOND_DELAY);
 // lockoutTimer_start();

 // // Implement game loop...
 // while ((!(buttons_read() & BUTTONS_BTN3_MASK)) && detector_getLives() > 0) { // Run until you detect BTN3 pressed.h
  //   intervalTimer_start(MAIN_CUMULATIVE_TIMER);     // Measure run-time when you are
 //                                                   // doing something.
 //   // Run filters, compute power, run hit-detection.
 //   detector(INTERRUPTS_CURRENTLY_ENABLED);         // Interrupts are currently enabled.
 //       if (detector_hitDetected()) {               // Hit detected
 //     hitCount++;                                   // increment the hit count.
 //     detector_clearHit();                          // Clear the hit.
 //     detector_hitCount_t
 //         hitCounts[DETECTOR_HIT_ARRAY_SIZE];       // Store the hit-counts here.
 //     detector_getHitCounts(hitCounts);             // Get the current hit counts.
 //     histogram_plotUserHits(hitCounts);            // Plot the hit counts on the TFT.
 //   }
 //   intervalTimer_stop(MAIN_CUMULATIVE_TIMER);      // All done with actual processing.
 // }

 // // Play the game over sound
 // trigger_disable();
 // sound_setSound(sound_gameOver_e);
 // sound_startSound();
 // utils_msDelay(ONE_SECOND_DELAY);

 // //sound loop for game over
 // while((!(buttons_read() & BUTTONS_BTN3_MASK))){
 //   //check for the sound being done
 //   if(sound_isSoundComplete()){
 //     //check to see if it's time to be silent or not, what part of the loop we're in
 //     if(silenceFlag){
 //       sound_setSound(sound_oneSecondSilence_e);
 //     } else {
 //       sound_setSound(sound_returnToBase_e);
 //     }
 //     sound_startSound();
 //     silenceFlag = !silenceFlag;
 //   }
 // }

 // interrupts_disableArmInts();                      // Done with game loop, disable the interrupts.
 // hitLedTimer_turnLedOff();                         // Save power
 // runningModes_printRunTimeStatistics();            // Print the run-time statistics.
}


   uint8_t rawIncomingData[1];
   uint8_t incomingData[1];
   uint8_t outgoingData[1];
   uint8_t playerTag;
   uint16_t readCounter = 0;
   bool playerOneFrozen, playerTwoFrozen, playerFourFrozen, playerThreeFrozen, gameOver, myPlayerFrozen;
static uint16_t tickCount = 0;

void game_freezeTag(void) {
 gameOver = false;
 myPlayerFrozen = false;
 playerOneFrozen = false;
 playerTwoFrozen = false;
 playerThreeFrozen = false;
 playerFourFrozen = false;
 uint16_t hitCount = 0;
 runningModes_initAll();
 trigger_enable();                                 // Makes the state machine responsive to the trigger.
 bool silenceFlag = false;
 // Configuration of the two teams
 playerTag = runningModes_getFrequencySetting()+1;
 bool teamB = ((playerTag-1) % DETERMINE_TEAM);
 bool ignoredFrequencies[FILTER_FREQUENCY_COUNT];
 for (uint16_t i = 0; i < FILTER_FREQUENCY_COUNT; i++) {
   ignoredFrequencies[i] = false;
 }
 ignoredFrequencies[9] = true;
 detector_setIgnoredFrequencies(ignoredFrequencies);

 // set transmitter frequency
 if(teamB) {
   transmitter_setFrequencyNumber(8);
 }
 else {
   transmitter_setFrequencyNumber(4);
 }
 
 interrupts_enableTimerGlobalInts(); // Allows the timer to generate
                                     // interrupts.
 interrupts_startArmPrivateTimer();  // Start the private ARM timer running.
 bluetooth_init();


 intervalTimer_reset(ISR_CUMULATIVE_TIMER);        // Used to measure ISR execution time.
 intervalTimer_reset(TOTAL_RUNTIME_TIMER);         // Used to measure total program execution time.
 intervalTimer_reset(MAIN_CUMULATIVE_TIMER);       // Used to measure main-loop execution time.
 intervalTimer_start(TOTAL_RUNTIME_TIMER);         // Start measuring total execution time.
 interrupts_enableArmInts();                       // ARM will now see interrupts after this.
 sound_setVolume(sound_mediumHighVolume_e);        // set volume
 sound_setSound(sound_gameStart_e);                // set & play game start sound
 sound_startSound();
 utils_msDelay(ONE_SECOND_DELAY);
 lockoutTimer_start();
 utils_msDelay(ONE_SECOND_DELAY);
 lockoutTimer_start();

 // Implement game loop...
 while ((!(buttons_read() & BUTTONS_BTN3_MASK))&&(!gameOver)) { // Run until you detect BTN3 pressed.h
    intervalTimer_start(MAIN_CUMULATIVE_TIMER);     // Measure run-time when you are
                                                   // doing something.
   // Run filters, compute power, run hit-detection.
   detector(INTERRUPTS_CURRENTLY_ENABLED);         // Interrupts are currently enabled.
   if (detector_hitDetected()) {                   // Hit detected
     hitCount++;
     if(!myPlayerFrozen){
       switch (playerTag) {
         case 1:
             outgoingData[PLAYER] = PLAYER_ONE_FROZEN;
         break;
         case 2:
             outgoingData[PLAYER] = PLAYER_TWO_FROZEN;
         break;
         case 3:
             outgoingData[PLAYER] = PLAYER_THREE_FROZEN;
         break;
         case 4:
             outgoingData[PLAYER] = PLAYER_FOUR_FROZEN;
         break;
         default:
             //WE NEED AN ERROR THING TO GO HERE, IF WE ARE HERE SOMETHING IS WRONG
         break;
       }
       bluetooth_transmitQueueWrite(outgoingData, 1);
     }else {
       switch (playerTag) {
         case 1:
             outgoingData[PLAYER] = PLAYER_ONE_UNFROZEN;
         break;
         case 2:
             outgoingData[PLAYER] = PLAYER_TWO_UNFROZEN;
         break;
         case 3:
             outgoingData[PLAYER] = PLAYER_THREE_UNFROZEN;
         break;
         case 4:
             outgoingData[PLAYER] = PLAYER_FOUR_UNFROZEN;
         break;
         default:
             //WE NEED AN ERROR THING TO GO HERE, IF WE ARE HERE SOMETHING IS WRONG
         break;
       }
       bluetooth_transmitQueueWrite(outgoingData, 1);
     }
     myPlayerFrozen = !myPlayerFrozen;
     if (detector_getLives() == 0) {
       utils_msDelay(ONE_SECOND_DELAY);
       lockoutTimer_start();
       utils_msDelay(ONE_SECOND_DELAY);
       lockoutTimer_start();
     }                                 // increment the hit count.
     detector_clearHit();                          // Clear the hit.
     detector_hitCount_t
         hitCounts[DETECTOR_HIT_ARRAY_SIZE];       // Store the hit-counts here.
     detector_getHitCounts(hitCounts);             // Get the current hit counts.
     histogram_plotUserHits(hitCounts);            // Plot the hit counts on the TFT.
    
   }
   interrupts_disableArmInts();
   if (tickCount < (5)) {
       readCounter = readCounter + bluetooth_receiveQueueRead(incomingData, 1);
       if(readCounter == ONE_READ) {
           switch (incomingData[PLAYER]) {
               case PLAYER_ONE_FROZEN:
                   playerOneFrozen = true;
                   sound_setSound(sound_p1Frozen);
                   sound_startSound();
               break;
               case PLAYER_ONE_UNFROZEN:
                   playerOneFrozen = false;
                   sound_setSound(sound_p1Unfrozen);
                   sound_startSound();
               break;
               case PLAYER_TWO_FROZEN:
                   playerTwoFrozen = true;
                   sound_setSound(sound_p2Frozen);
                   sound_startSound();
               break;
               case PLAYER_TWO_UNFROZEN:
                   playerTwoFrozen = false;
                   sound_setSound(sound_p2Unfrozen);
                   sound_startSound();
               break;
               case PLAYER_THREE_FROZEN:
                   playerThreeFrozen = true;
                   sound_setSound(sound_p3Frozen);
                   sound_startSound();
               break;
               case PLAYER_THREE_UNFROZEN:
                   playerThreeFrozen = false;
                   sound_setSound(sound_p3Unfrozen);
                   sound_startSound();
               break;
               case PLAYER_FOUR_FROZEN:
                   playerFourFrozen = true;
                   sound_setSound(sound_p4Frozen);
                   sound_startSound();
               break;
               case PLAYER_FOUR_UNFROZEN:
                   playerFourFrozen = false;
                   sound_setSound(sound_p4Unfrozen);
                   sound_startSound();
               break;
               default:
                   //WE NEED AN ERROR THING TO GO HERE, IF WE ARE HERE SOMETHING IS WRONG
               break;
           }
           outgoingData[0] = 'R';
           bluetooth_transmitQueueWrite(outgoingData, 1);
           readCounter = 0;


       }
   }
   if((playerOneFrozen&&playerThreeFrozen)||(playerTwoFrozen&&playerFourFrozen)){gameOver=true;}
   interrupts_enableArmInts();
   intervalTimer_stop(MAIN_CUMULATIVE_TIMER);      // All done with actual processing.
 }


 // Play the game over sound
 trigger_disable();
 sound_setSound(sound_gameOver_e);
 sound_startSound();
 utils_msDelay(ONE_SECOND_DELAY);


 //sound loop for game over
 while((!(buttons_read() & BUTTONS_BTN3_MASK))){
   //check for the sound being done
   if(sound_isSoundComplete()){
     //check to see if it's time to be silent or not, what part of the loop we're in
     if(silenceFlag){
       sound_setSound(sound_oneSecondSilence_e);
     } else {
       sound_setSound(sound_returnToBase_e);
     }
     sound_startSound();
     silenceFlag = !silenceFlag;
   }
 }

 interrupts_disableArmInts();                      // Done with game loop, disable the interrupts.
 hitLedTimer_turnLedOff();                         // Save power
 runningModes_printRunTimeStatistics();            // Print the run-time statistics.
}


void bluetooth_isr_function() {
 tickCount++;
 if (tickCount > BLUETOOTH_SERVICE_INTERVAL) {
   bluetooth_poll();
   tickCount = 0;
 }
 bluetooth_poll();
}
