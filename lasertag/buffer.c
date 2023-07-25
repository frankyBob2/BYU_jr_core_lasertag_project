#include "buffer.h"
#include <stdbool.h>

// This implements a dedicated circular buffer for storing values
// from the ADC until they are read and processed by the detector.
// The function of the buffer is similar to a buffer or FIFO.

#define BUFFER_SIZE 32768
#define INDEXING_OFFSET 1
#define INIT_ZERO 0
#define EMPTY 0
#define BUG 0

// Uncomment for debug prints
// #define DEBUG

#if defined(DEBUG)
#include "xil_printf.h" // outbyte
#include <stdio.h>
#define DPRINTF(...) printf(__VA_ARGS__)
#define BUG 1
#else
#define DPRINTF(...)
#endif


//creating struct for simplicity
typedef struct {
  uint32_t indexIn;                // Points to the next open slot.
  uint32_t indexOut;               // Points to the next element to be removed.
  uint32_t elementCount;           // Number of elements in the buffer.
  buffer_data_t data[BUFFER_SIZE]; // Values are stored here.
} buffer_t;

volatile static buffer_t buff;

// Initialize the buffer to empty.
void buffer_init(void) {
  // Always points to the next open slot.
  buff.indexIn = INIT_ZERO;
  // Always points to the next element to be removed
  // from the buffer (or "oldest" element).
  buff.indexOut = INIT_ZERO;
  // Keep track of the number of elements currently in buffer.s
  buff.elementCount = INIT_ZERO;
}

// Add an element to the buffer 
void buffer_push(buffer_data_t value) {
  // If the buffer is full pop before pushing
        if(buff.elementCount < BUFFER_SIZE) {
            buff.data[buff.indexIn] = value;
            buff.elementCount++;
            // Loop indexIn back to 0 if necessary
            if(buff.indexIn == BUFFER_SIZE-INDEXING_OFFSET)
                buff.indexIn = INIT_ZERO;
            else
                buff.indexIn++;
        }
        // If elementCount is equal to size, it cannot push
        else if(buff.elementCount == BUFFER_SIZE) {
            if(BUG)
              DPRINTF("ERROR: buffer IS FULL, CANNOT PUSH\n");
        }
}

// Remove a value from the buffer. Return zero if empty.
buffer_data_t buffer_pop(void) {
  buffer_data_t value;
  // If elementCount is greater then 0, pop a datum
  if (buff.elementCount > EMPTY) {
    value = buff.data[buff.indexOut];
    buff.elementCount--;
    // Loop indexIn back to 0 if necessary
    if (buff.indexOut == BUFFER_SIZE - INDEXING_OFFSET)
      buff.indexOut = INIT_ZERO;
    else
        buff.indexOut++;
  }
  // If elementCount equals 0, it cannot pop
  else {
    if(BUG)
      DPRINTF("ERROR: buffer IS EMPTY, CANNOT POP\n");
    return 0;
  }

  return value;
}

// Add a value to the buffer. Overwrite the oldest value if full.
void buffer_pushover(buffer_data_t value) {
  // If the buffer is full pop before pushing
  if(buff.elementCount == BUFFER_SIZE) {
      buffer_pop();
      buffer_push(value);
  }
  // If the buffer is not full push normally
  else
      buffer_push(value);
}

// Return the number of elements in the buffer.
uint32_t buffer_elements(void) {
    return buff.elementCount;
}

// Return the capacity of the buffer in elements.
uint32_t buffer_size(void) {
    return BUFFER_SIZE;
}