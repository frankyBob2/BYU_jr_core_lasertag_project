#include "filter.h"
#include <stdint.h>
#include "queue.h"

#define FIR_FILTER_TAP_COUNT 81
#define QUEUE_INIT_VALUE 0.0
#define X_QUEUE_SIZE 81
#define Y_QUEUE_SIZE 11
#define Z_QUEUE_SIZE 10
#define OUTPUT_QUEUE_SIZE 2000
#define INDEX_ONE 1

static queue_t xQueue;
static queue_t yQueue;
static queue_t zQueues[FILTER_IIR_FILTER_COUNT];
static queue_t outputQueues[FILTER_IIR_FILTER_COUNT];

// 1. First filter is a decimating FIR filter with a configurable number of taps
// and decimation factor.
// 2. The output from the decimating FIR filter is passed through a bank of 10
// IIR filters. The characteristics of the IIR filter are fixed.

// Initialize the X-Queue with all zeros
void initXQueue()
{
    queue_init(&xQueue, X_QUEUE_SIZE, "xQueue");
    //check for loop 
    for (uint32_t i = 0; i < X_QUEUE_SIZE; i++)
        queue_overwritePush(&xQueue, QUEUE_INIT_VALUE);
}

// Initialize the Y-Queue with all zeros
void initYQueue()
{
    queue_init(&yQueue, Y_QUEUE_SIZE, "yQueue");
    //check for loop
    for (uint32_t i = 0; i < Y_QUEUE_SIZE; i++)
        queue_overwritePush(&yQueue, QUEUE_INIT_VALUE);
}

// Initializes the Z-Queue with zeros
void initZQueues()
{
    // There are 10 values for each filter so we initialize each value with nested for-loops
    for (uint32_t i = 0; i < FILTER_IIR_FILTER_COUNT; i++)
    {
        queue_init(&(zQueues[i]), Z_QUEUE_SIZE, "zQueue");
        //check for loop
        for (uint32_t j = 0; j < Z_QUEUE_SIZE; j++)
            queue_overwritePush(&(zQueues[i]), QUEUE_INIT_VALUE);
    }
}

// Initializes the output queues to all zeros
void initOutputQueues()
{
    // There are 2000 values for each of the 10 filters so we use nested for-loops to initialize all values
    for (uint32_t i = 0; i < FILTER_IIR_FILTER_COUNT; i++)
    {
        queue_init(&(outputQueues[i]), OUTPUT_QUEUE_SIZE, "outputQueue");
        //check for loop
        for (uint32_t j = 0; j < OUTPUT_QUEUE_SIZE; j++)
            queue_overwritePush(&(outputQueues[i]), QUEUE_INIT_VALUE);
    }
}

/******************************************************************************
***** Main Filter Functions
******************************************************************************/
// Must call this prior to using any filteIR coefficients were declared, for example: const static double iir_a_coeffs[FREQUENCY_COUNT][IIR_A_COEFF_COUr functions.
void filter_init()
{
    initXQueue();       // Call queue_init() on xQueue and fill it with zeros.
    initYQueue();       // Call queue_init() on yQueue and fill it with zeros.
    initZQueues();      // Call queue_init() on all of the zQueues and fill each z queue with zeros.
    initOutputQueues(); // Call queue_init() on all of the outputQueues and fill each outputQueue with zeros.
}

// Use this to copy an input into the input queue of the FIR-filter (xQueue).
void filter_addNewInput(double x)
{
    queue_overwritePush(&xQueue, x);
}

// Invokes the FIR-filter. Input is contents of xQueue.
// Output is returned and is also pushed on to yQueue.
double filter_firFilter()
{
    double y = 0.0;
    // For loop to iterate through each element and multiply it by a filter coefficent
    for (uint32_t i = 0; i < FIR_FILTER_TAP_COUNT; i++)
    {
        y += queue_readElementAt(&xQueue, i) * firCoefficients[FIR_FILTER_TAP_COUNT - i - INDEX_ONE];
    }
    queue_overwritePush(&yQueue, y);
    return y;
}

// Use this to invoke a single iir filter. Input comes from yQueue.
// Output is returned and is also pushed onto zQueue[filterNumber].
double filter_iirFilter(uint16_t filterNumber)
{
    double z = 0;
    double yTotal = 0;
    double zTotal = 0;
    // This is the calculation to populate the zQueue and output queue with updated values and is then pushed to each queue
    for (uint32_t j = 0; j < IIR_B_COEFFICIENT_COUNT; j++)
    {
        yTotal += iirBCoefficientConstants[filterNumber][j] * queue_readElementAt(&yQueue, IIR_B_COEFFICIENT_COUNT - j - INDEX_ONE);
        //if the right conditions, add to the power
        if (j != IIR_B_COEFFICIENT_COUNT - INDEX_ONE)
            zTotal += iirACoefficientConstants[filterNumber][j] * queue_readElementAt(&(zQueues[filterNumber]), IIR_A_COEFFICIENT_COUNT - j - INDEX_ONE);
    }
    z = yTotal - zTotal;
    queue_overwritePush(&(zQueues[filterNumber]), z);
    queue_overwritePush(&(outputQueues[filterNumber]), z);
    return z;
}

// Use this to compute the power for values contained in an outputQueue.
// If force == true, then recompute power by using all values in the
// outputQueue. This option is necessary so that you can correctly compute power
// values the first time. After that, you can incrementally compute power values
// by:
// 1. Keeping track of the power computed in a previous run, call this
// prev-power.
// 2. Keeping track of the oldest outputQueue value used in a previous run, call
// this oldest-value.
// 3. Get the newest value from the power queue, call this newest-value.
// 4. Compute new power as: prev-power - (oldest-value * oldest-value) +
// (newest-value * newest-value). Note that this function will probably need an
// array to keep track of these values for each of the 10 output queues.
double prevPower[FILTER_IIR_FILTER_COUNT];
double oldestValue[FILTER_IIR_FILTER_COUNT];

// Return the amount of power in the signal output by the corresponding IIR filter
double filter_computePower(uint16_t filterNumber, bool forceComputeFromScratch, bool debugPrint)
{
    double power = 0.0;
    // Computes the power using all values of the output queue starting from scratch
    if (forceComputeFromScratch)
    {
        // Iterates through output queue and sqaures each value before adding it to total
        for (uint32_t i = 0; i < OUTPUT_QUEUE_SIZE; i++)
        {
            power += queue_readElementAt(&(outputQueues[filterNumber]), i) * queue_readElementAt(&(outputQueues[filterNumber]), i);
        }
    }
    // Calculates the power based on the newest value of the output queue and previous power calculation
    else
    {
        double newestValue = queue_readElementAt(&(outputQueues[filterNumber]), OUTPUT_QUEUE_SIZE - INDEX_ONE);
        power = prevPower[filterNumber] - (oldestValue[filterNumber] * oldestValue[filterNumber]) + (newestValue * newestValue);
    }
    prevPower[filterNumber] = power;
    oldestValue[filterNumber] = queue_readElementAt(&(outputQueues[filterNumber]), 0);
    return power;
}

// Returns the last-computed output power value for the IIR filter
// [filterNumber].
double filter_getCurrentPowerValue(uint16_t filterNumber)
{
    return prevPower[filterNumber];
}

// Sets a current power value for a specific filter number.
// Useful in testing the detector.
void filter_setCurrentPowerValue(uint16_t filterNumber, double value)
{
    prevPower[filterNumber] = value;
}

// Get a copy of the current power values.
// This function copies the already computed values into a previously-declared
// array so that they can be accessed from outside the filter software by the
// detector. Remember that when you pass an array into a C function, changes to
// the array within that function are reflected in the returned array.
void filter_getCurrentPowerValues(double powerValues[])
{
    // Iterates through each filter to put power values into new array
    for (uint32_t i = 0; i < FILTER_IIR_FILTER_COUNT; i++)
    {
        powerValues[i] = prevPower[i];
    }
}

// Using the previously-computed power values that are currently stored in
// currentPowerValue[] array, copy these values into the normalizedArray[]
// argument and then normalize them by dividing all of the values in
// normalizedArray by the maximum power value contained in currentPowerValue[].
// The pointer argument indexOfMaxValue is used to return the index of the
// maximum value. If the maximum power is zero, make sure to not divide by zero
// and that *indexOfMaxValue is initialized to a sane value (like zero).
void filter_getNormalizedPowerValues(double normalizedArray[], uint16_t *indexOfMaxValue)
{
    double maxPower = 0;
    // Ensures that we don't divide by zero
    for (uint32_t i = 0; i < FILTER_IIR_FILTER_COUNT; i++)
    {   
        // If previous power is greater than mac power, update max power
        if (prevPower[i] > maxPower)
        {
            maxPower = prevPower[i];
            *indexOfMaxValue = i;
        }
    }
    // Copies then normalized power values into the normalized array
    for (uint32_t i = 0; i < FILTER_IIR_FILTER_COUNT; i++)
    {
        normalizedArray[i] = prevPower[i] / prevPower[*indexOfMaxValue];
    }
}

/******************************************************************************
***** Verification-Assisting Functions
***** External test functions access the internal data structures of filter.c
***** via these functions. They are not used by the main filter functions.
******************************************************************************/

// Returns the array of FIR coefficients.
const double *filter_getFirCoefficientArray()
{
    return firCoefficients;
}

// Returns the number of FIR coefficients.
uint32_t filter_getFirCoefficientCount()
{
    return FIR_FILTER_TAP_COUNT;
}

// Returns the array of coefficients for a particular filter number.
const double *filter_getIirACoefficientArray(uint16_t filterNumber)
{
    return iirACoefficientConstants[filterNumber];
}

// Returns the number of A coefficients.
uint32_t filter_getIirACoefficientCount()
{
    return IIR_A_COEFFICIENT_COUNT;
}

// Returns the array of coefficients for a particular filter number.
const double *filter_getIirBCoefficientArray(uint16_t filterNumber)
{
    return iirBCoefficientConstants[filterNumber];
}

// Returns the number of B coefficients.
uint32_t filter_getIirBCoefficientCount()
{
    return IIR_B_COEFFICIENT_COUNT;
}

// Returns the size of the yQueue.
uint32_t filter_getYQueueSize()
{
    queue_size(&yQueue);
}

// Returns the decimation value.
uint16_t filter_getDecimationValue()
{
    return FILTER_FIR_DECIMATION_FACTOR;
}

// Returns the address of xQueue.
queue_t *filter_getXQueue()
{
    return &xQueue;
}

// Returns the address of yQueue.
queue_t *filter_getYQueue()
{
    return &yQueue;
}

// Returns the address of zQueue for a specific filter number.
queue_t *filter_getZQueue(uint16_t filterNumber)
{
    return &(zQueues[filterNumber]);
}

// Returns the address of the IIR output-queue for a specific filter-number.
queue_t *filter_getIirOutputQueue(uint16_t filterNumber)
{
    return &(outputQueues[filterNumber]);
}