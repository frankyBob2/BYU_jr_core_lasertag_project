    #include <stdio.h>  // printf
    #include <stdlib.h> // malloc, free, abort
    #include <string.h> // strncpy
    #include "queue.h"

    #define INIT_ZERO 0
    #define EMPTY 0
    #define INDEXING_OFFSET 1

    // Allocates memory for the queue (the data* pointer) and initializes all
    // parts of the data structure. Prints out an error message if malloc() fails
    // and calls assert(false) to print-out line-number information and die.
    // The queue is empty after initialization. To fill the queue with known
    // values (e.g. zeros), call queue_overwritePush() up to queue_size() times.
    void queue_init(queue_t *q, queue_size_t size, const char *name)
    {
    	// Always points to the next open slot.
    	q->indexIn = INIT_ZERO;
    	// Always points to the next element to be removed
    	// from the queue (or "oldest" element).
    	q->indexOut = INIT_ZERO;
    	// Keep track of the number of elements currently in queue.s
    	q->elementCount = INIT_ZERO;
    	// Queue capacity.
    	q->size = size;
    	// Points to a dynamically-allocated array.
    	q->data = malloc(size * sizeof(queue_data_t));
    	if (q->data == NULL) abort();
    	// True if queue_pop() is called on an empty queue. Reset
    	// to false after queue_push() is called.
    	q->underflowFlag = false;
    	// True if queue_push() is called on a full queue. Reset to
    	// false once queue_pop() is called.
    	q->overflowFlag = false;
    	// Name for debugging purposes.
    	strncpy(q->name, name, QUEUE_MAX_NAME_SIZE);
    	q->name[QUEUE_MAX_NAME_SIZE-INDEXING_OFFSET] = '\0';
    }
    
    // Get the user-assigned name for the queue.
    const char *queue_name(queue_t *q)
    {
    	return q->name;
    }

    // Returns the capacity of the queue.
    queue_size_t queue_size(queue_t *q)
    {
    	return q->size;
    }

    // Returns true if the queue is full.
    bool queue_full(queue_t *q)
    {
        return q->elementCount == q->size;
    }

    // Returns true if the queue is empty.
    bool queue_empty(queue_t *q)
    {
        return !q->elementCount;
    }

    // If the queue is not full, pushes a new element into the queue and clears the
    // underflowFlag. IF the queue is full, set the overflowFlag, print an error
    // message and DO NOT change the queue.
    void queue_push(queue_t *q, queue_data_t value)
    {
        // If elementCount is less than size, push the data
        if(q->elementCount < q->size) {
            q->data[q->indexIn] = value;
            // Loop indexIn back to 0 if necessary
            if(q->indexIn == q->size-INDEXING_OFFSET)
                q->indexIn = INIT_ZERO;
            else
                q->indexIn++;
            q->elementCount++;
            q->underflowFlag = false;
        }
        // If elementCount is equal to size, it cannot push
        else if(q->elementCount == q->size) {
            q->overflowFlag = true;
            printf("ERROR: QUEUE IS FULL, CANNOT PUSH\n");
        }
    }

    // If the queue is not empty, remove and return the oldest element in the queue.
    // If the queue is empty, set the underflowFlag, print an error message, and DO
    // NOT change the queue.
    queue_data_t queue_pop(queue_t *q)
    {
        queue_data_t value;
        // If elementCount is greater then 0, pop a datum
        if(q->elementCount > EMPTY) {
            value = q->data[q->indexOut];
            // Loop indexIn back to 0 if necessary
            if(q->indexOut == q->size-INDEXING_OFFSET)
                q->indexOut = INIT_ZERO;
            else 
                q->indexOut++;
            q->elementCount--;
            q->overflowFlag = false;
        }
        // If elementCount equals 0, it cannot pop
        else if(q->elementCount == EMPTY) {
            q->underflowFlag = true;
            printf("ERROR: QUEUE IS EMPTY, CANNOT POP\n");
            return 0;
        }

        return value;
    }

    // If the queue is full, call queue_pop() and then call queue_push().
    // If the queue is not full, just call queue_push().
    void queue_overwritePush(queue_t *q, queue_data_t value)
    {
        // If the queue is full pop before pushing
        if(queue_full(q)) {
            queue_pop(q);
            queue_push(q, value);
        }
        // If the queue is not full push normally
        else
            queue_push(q, value);
    }

    // Provides random-access read capability to the queue.
    // Low-valued indexes access older queue elements while higher-value indexes
    // access newer elements (according to the order that they were added). Print a
    // meaningful error message if an error condition is detected.
    queue_data_t queue_readElementAt(queue_t *q, queue_index_t index)
    {
        queue_data_t value = INIT_ZERO;
        // If the markers are in normal order, read element normally
        if(q->indexOut < q->indexIn) {
            // Check if the element to access is out of range
            if((q->indexOut + index) < q->indexIn)
                value = q->data[q->indexOut + index];
            else
                printf("ERROR: Index is out of range\n");
        }
        // If the markers have partially looped, use more checks
        else {
            // Check if the element to access isn't looped
            if((q->indexOut + index) < q->size)
                value = q->data[q->indexOut + index];
            // If the element to access is looped, subtract the size when acessing
            else {
                // Check if the element to access is out of range
                if((q->indexOut + index - q->size) < q->indexIn)
                    value = q->data[q->indexOut + index - q->size];
                else 
                    printf("ERROR: Index is out of range\n");
            }
        }

        return value;

    }

    // Returns a count of the elements currently contained in the queue.
    queue_size_t queue_elementCount(queue_t *q)
    {
    	return q->elementCount;
    }

    // Returns true if an underflow has occurred (queue_pop() called on an empty
    // queue).
    bool queue_underflow(queue_t *q)
    {
    	return q->underflowFlag;
    }

    // Returns true if an overflow has occurred (queue_push() called on a full
    // queue).
    bool queue_overflow(queue_t *q)
    {
    	return q->overflowFlag;
    }

    // Frees the storage that you malloc'd before.
    void queue_garbageCollect(queue_t *q)
    {
    	free(q->data);
    }

