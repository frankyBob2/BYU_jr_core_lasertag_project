/*
This software is provided for student assignment use in the Department of
Electrical and Computer Engineering, Brigham Young University, Utah, USA.
Users agree to not re-host, or redistribute the software, in source or binary
form, to other persons or other institutions. Users may modify and use the
source code for personal or educational use.
For questions, contact Brad Hutchings or Jeff Goeders, https://ece.byu.edu/
*/

#ifndef FILTER_H_
#define FILTER_H_

#include <stdint.h>

#include "queue.h"

#define FILTER_IIR_FILTER_COUNT 10
#define IIR_A_COEFFICIENT_COUNT 10
#define IIR_B_COEFFICIENT_COUNT 11
#define FILTER_SAMPLE_FREQUENCY_IN_KHZ 100
#define FILTER_FREQUENCY_COUNT 10
#define FIR_FILTER_TAP_COUNT 81
#define FILTER_FIR_DECIMATION_FACTOR 10 // FIR-filter needs this many new inputs to compute a new output.
#define FILTER_INPUT_PULSE_WIDTH 2000 // This is the width of the pulse you are looking for, in terms of
                                      // decimated sample count.
// These are the tick counts that are used to generate the user frequencies.
// Not used in filter.h but are used to TEST the filter code.
// Placed here for general access as they are essentially constant throughout
// the code. The transmitter will also use these.
static const uint16_t filter_frequencyTickTable[FILTER_FREQUENCY_COUNT] = {
    68, 58, 50, 44, 38, 34, 30, 28, 26, 24};

// Filtering routines for the laser-tag project.
// Filtering is performed by a two-stage filter, as described below.

// 1. First filter is a decimating FIR filter with a configurable number of taps
// and decimation factor.
// 2. The output from the decimating FIR filter is passed through a bank of 10
// IIR filters. The characteristics of the IIR filter are fixed.

/******************************************************************************
***** Main Filter Functions
******************************************************************************/

// Must call this prior to using any filter functions.
void filter_init();

// Use this to copy an input into the input queue of the FIR-filter (xQueue).
void filter_addNewInput(double x);

// Invokes the FIR-filter. Input is contents of xQueue.
// Output is returned and is also pushed on to yQueue.
double filter_firFilter();

// Use this to invoke a single iir filter. Input comes from yQueue.
// Output is returned and is also pushed onto zQueue[filterNumber].
double filter_iirFilter(uint16_t filterNumber);

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
double filter_computePower(uint16_t filterNumber, bool forceComputeFromScratch,
                           bool debugPrint);

// Returns the last-computed output power value for the IIR filter
// [filterNumber].
double filter_getCurrentPowerValue(uint16_t filterNumber);

// Sets a current power value for a specific filter number.
// Useful in testing the detector.
void filter_setCurrentPowerValue(uint16_t filterNumber, double value);

// Get a copy of the current power values.
// This function copies the already computed values into a previously-declared
// array so that they can be accessed from outside the filter software by the
// detector. Remember that when you pass an array into a C function, changes to
// the array within that function are reflected in the returned array.
void filter_getCurrentPowerValues(double powerValues[]);

// Using the previously-computed power values that are currently stored in
// currentPowerValue[] array, copy these values into the normalizedArray[]
// argument and then normalize them by dividing all of the values in
// normalizedArray by the maximum power value contained in currentPowerValue[].
// The pointer argument indexOfMaxValue is used to return the index of the
// maximum value. If the maximum power is zero, make sure to not divide by zero
// and that *indexOfMaxValue is initialized to a sane value (like zero).
void filter_getNormalizedPowerValues(double normalizedArray[],
                                     uint16_t *indexOfMaxValue);

/******************************************************************************
***** Verification-Assisting Functions
***** External test functions access the internal data structures of filter.c
***** via these functions. They are not used by the main filter functions.
******************************************************************************/

// Returns the array of FIR coefficients.
const double *filter_getFirCoefficientArray();

// Returns the number of FIR coefficients.
uint32_t filter_getFirCoefficientCount();

// Returns the array of coefficients for a particular filter number.
const double *filter_getIirACoefficientArray(uint16_t filterNumber);

// Returns the number of A coefficients.
uint32_t filter_getIirACoefficientCount();

// Returns the array of coefficients for a particular filter number.
const double *filter_getIirBCoefficientArray(uint16_t filterNumber);

// Returns the number of B coefficients.
uint32_t filter_getIirBCoefficientCount();

// Returns the size of the yQueue.
uint32_t filter_getYQueueSize();

// Returns the decimation value.
uint16_t filter_getDecimationValue();

// Returns the address of xQueue.
queue_t *filter_getXQueue();

// Returns the address of yQueue.
queue_t *filter_getYQueue();

// Returns the address of zQueue for a specific filter number.
queue_t *filter_getZQueue(uint16_t filterNumber);

// Returns the address of the IIR output-queue for a specific filter-number.
queue_t *filter_getIirOutputQueue(uint16_t filterNumber);

// This array contains our FIR filter coefficients computed from matlab
const static double firCoefficients[FIR_FILTER_TAP_COUNT] = {
    6.0546138291252597e-04,
    5.2507143315267811e-04,
    3.8449091272701525e-04,
    1.7398667197948182e-04,
    -1.1360489934931548e-04,
    -4.7488111478632532e-04,
    -8.8813878356223768e-04,
    -1.3082618178394971e-03,
    -1.6663618496969908e-03,
    -1.8755700366336781e-03,
    -1.8432363328817916e-03,
    -1.4884258721727399e-03,
    -7.6225514924622853e-04,
    3.3245249132384837e-04,
    1.7262548802593762e-03,
    3.2768418720744217e-03,
    4.7744814146589041e-03,
    5.9606317814670249e-03,
    6.5591485566565593e-03,
    6.3172870282586493e-03,
    5.0516421324586546e-03,
    2.6926388909554420e-03,
    -6.7950808883015244e-04,
    -4.8141100026888716e-03,
    -9.2899200683230643e-03,
    -1.3538595939086505e-02,
    -1.6891587875325020e-02,
    -1.8646984919441702e-02,
    -1.8149697899123560e-02,
    -1.4875876924586697e-02,
    -8.5110608557150517e-03,
    9.8848931927316319e-04,
    1.3360421141947857e-02,
    2.8033301291042201e-02,
    4.4158668590312596e-02,
    6.0676486642862550e-02,
    7.6408062643700314e-02,
    9.0166807112971648e-02,
    1.0087463525509034e-01,
    1.0767073207825099e-01,
    1.1000000000000000e-01,
    1.0767073207825099e-01,
    1.0087463525509034e-01,
    9.0166807112971648e-02,
    7.6408062643700314e-02,
    6.0676486642862550e-02,
    4.4158668590312596e-02,
    2.8033301291042201e-02,
    1.3360421141947857e-02,
    9.8848931927316319e-04,
    -8.5110608557150517e-03,
    -1.4875876924586697e-02,
    -1.8149697899123560e-02,
    -1.8646984919441702e-02,
    -1.6891587875325020e-02,
    -1.3538595939086505e-02,
    -9.2899200683230643e-03,
    -4.8141100026888716e-03,
    -6.7950808883015244e-04,
    2.6926388909554420e-03,
    5.0516421324586546e-03,
    6.3172870282586493e-03,
    6.5591485566565593e-03,
    5.9606317814670249e-03,
    4.7744814146589041e-03,
    3.2768418720744217e-03,
    1.7262548802593762e-03,
    3.3245249132384837e-04,
    -7.6225514924622853e-04,
    -1.4884258721727399e-03,
    -1.8432363328817916e-03,
    -1.8755700366336781e-03,
    -1.6663618496969908e-03,
    -1.3082618178394971e-03,
    -8.8813878356223768e-04,
    -4.7488111478632532e-04,
    -1.1360489934931548e-04,
    1.7398667197948182e-04,
    3.8449091272701525e-04,
    5.2507143315267811e-04,
    6.0546138291252597e-04};

// This 2 dimensional array contains our IIR-A coefficients for all 10 filters
const static double iirACoefficientConstants[FILTER_IIR_FILTER_COUNT][IIR_A_COEFFICIENT_COUNT] = {
    {-5.9637727070164033e+00, 1.9125339333078262e+01, -4.0341474540744223e+01, 6.1537466875368928e+01, -7.0019717951472359e+01, 6.0298814235239057e+01, -3.8733792862566446e+01, 1.7993533279581140e+01, -5.4979061224867953e+00, 9.0332828533800158e-01},
    {-4.6377947119071443e+00, 1.3502215749461563e+01, -2.6155952405269730e+01, 3.8589668330738292e+01, -4.3038990303252554e+01, 3.7812927599537034e+01, -2.5113598088113712e+01, 1.2703182701888043e+01, -4.2755083391143316e+00, 9.0332828533799781e-01},
    {-3.0591317915750951e+00, 8.6417489609637563e+00, -1.4278790253808854e+01, 2.1302268283304326e+01, -2.2193853972079253e+01, 2.0873499791105470e+01, -1.3709764520609415e+01, 8.1303553577931851e+00, -2.8201643879900580e+00, 9.0332828533800236e-01},
    {-1.4071749185996771e+00, 5.6904141470697578e+00, -5.7374718273676413e+00, 1.1958028362868918e+01, -8.5435280598354826e+00, 1.1717345583835980e+01, -5.5088290876998780e+00, 5.3536787286077736e+00, -1.2972519209655626e+00, 9.0332828533800136e-01},
    {8.2010906117760329e-01, 5.1673756579268595e+00, 3.2580350909220921e+00, 1.0392903763919190e+01, 4.8101776408669066e+00, 1.0183724507092503e+01, 3.1282000712126736e+00, 4.8615933365571946e+00, 7.5604535083144853e-01, 9.0332828533799947e-01},
    {2.7080869856154464e+00, 7.8319071217995475e+00, 1.2201607990980694e+01, 1.8651500443681531e+01, 1.8758157568004435e+01, 1.8276088095998901e+01, 1.1715361303018808e+01, 7.3684394621252913e+00, 2.4965418284511678e+00, 9.0332828533799581e-01},
    {4.9479835250075874e+00, 1.4691607003177591e+01, 2.9082414772101028e+01, 4.3179839108869274e+01, 4.8440791644688801e+01, 4.2310703962394257e+01, 2.7923434247706368e+01, 1.3822186510470974e+01, 4.5614664160654215e+00, 9.0332828533799658e-01},
    {6.1701893352279829e+00, 2.0127225876810321e+01, 4.2974193398071641e+01, 6.5958045321253366e+01, 7.5230437667866497e+01, 6.4630411355739767e+01, 4.1261591079244056e+01, 1.8936128791950505e+01, 5.6881982915180203e+00, 9.0332828533799647e-01},
    {7.4092912870072354e+00, 2.6857944460290113e+01, 6.1578787811202183e+01, 9.8258255839887198e+01, 1.1359460153696280e+02, 9.6280452143025911e+01, 5.9124742025776264e+01, 2.5268527576524143e+01, 6.8305064480742885e+00, 9.0332828533799747e-01},
    {8.5743055776347745e+00, 3.4306584753117939e+01, 8.4035290411037266e+01, 1.3928510844056862e+02, 1.6305115418161688e+02, 1.3648147221895857e+02, 8.0686288623300214e+01, 3.2276361903872321e+01, 7.9045143816245282e+00, 9.0332828533800358e-01}};

// This 2 dimensional array contains the IIR-B coefficients for all 10 filters
const static double iirBCoefficientConstants[FILTER_IIR_FILTER_COUNT][IIR_B_COEFFICIENT_COUNT] = {
    {9.0928661148194273e-10, 0.0000000000000000e+00, -4.5464330574097132e-09, 0.0000000000000000e+00, 9.0928661148194265e-09, 0.0000000000000000e+00, -9.0928661148194265e-09, 0.0000000000000000e+00, 4.5464330574097132e-09, 0.0000000000000000e+00, -9.0928661148194273e-10},
    {9.0928661148200467e-10, 0.0000000000000000e+00, -4.5464330574100234e-09, 0.0000000000000000e+00, 9.0928661148200469e-09, 0.0000000000000000e+00, -9.0928661148200469e-09, 0.0000000000000000e+00, 4.5464330574100234e-09, 0.0000000000000000e+00, -9.0928661148200467e-10},
    {9.0928661148188338e-10, 0.0000000000000000e+00, -4.5464330574094171e-09, 0.0000000000000000e+00, 9.0928661148188342e-09, 0.0000000000000000e+00, -9.0928661148188342e-09, 0.0000000000000000e+00, 4.5464330574094171e-09, 0.0000000000000000e+00, -9.0928661148188338e-10},
    {9.0928661148192443e-10, 0.0000000000000000e+00, -4.5464330574096223e-09, 0.0000000000000000e+00, 9.0928661148192445e-09, 0.0000000000000000e+00, -9.0928661148192445e-09, 0.0000000000000000e+00, 4.5464330574096223e-09, 0.0000000000000000e+00, -9.0928661148192443e-10},
    {9.0928661148202390e-10, 0.0000000000000000e+00, -4.5464330574101194e-09, 0.0000000000000000e+00, 9.0928661148202388e-09, 0.0000000000000000e+00, -9.0928661148202388e-09, 0.0000000000000000e+00, 4.5464330574101194e-09, 0.0000000000000000e+00, -9.0928661148202390e-10},
    {9.0928661148205316e-10, 0.0000000000000000e+00, -4.5464330574102658e-09, 0.0000000000000000e+00, 9.0928661148205316e-09, 0.0000000000000000e+00, -9.0928661148205316e-09, 0.0000000000000000e+00, 4.5464330574102658e-09, 0.0000000000000000e+00, -9.0928661148205316e-10},
    {9.0928661148200353e-10, 0.0000000000000000e+00, -4.5464330574100176e-09, 0.0000000000000000e+00, 9.0928661148200353e-09, 0.0000000000000000e+00, -9.0928661148200353e-09, 0.0000000000000000e+00, 4.5464330574100176e-09, 0.0000000000000000e+00, -9.0928661148200353e-10},
    {9.0928661148210507e-10, 0.0000000000000000e+00, -4.5464330574105255e-09, 0.0000000000000000e+00, 9.0928661148210511e-09, 0.0000000000000000e+00, -9.0928661148210511e-09, 0.0000000000000000e+00, 4.5464330574105255e-09, 0.0000000000000000e+00, -9.0928661148210507e-10},
    {9.0928661148201087e-10, 0.0000000000000000e+00, -4.5464330574100540e-09, 0.0000000000000000e+00, 9.0928661148201081e-09, 0.0000000000000000e+00, -9.0928661148201081e-09, 0.0000000000000000e+00, 4.5464330574100540e-09, 0.0000000000000000e+00, -9.0928661148201087e-10},
    {9.0928661148192753e-10, 0.0000000000000000e+00, -4.5464330574096380e-09, 0.0000000000000000e+00, 9.0928661148192759e-09, 0.0000000000000000e+00, -9.0928661148192759e-09, 0.0000000000000000e+00, 4.5464330574096380e-09, 0.0000000000000000e+00, -9.0928661148192753e-10}};


#endif /* FILTER_H_ */