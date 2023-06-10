
#ifndef PAGECIRCULARBUFFERITERATOR_CC
#define PAGECIRCULARBUFFERITERATOR_CC

#include "PDBDebug.h"
#include "PageCircularBufferIterator.h"
#include "PageCircularBuffer.h"
#include <iostream>
using namespace std;
PageCircularBufferIterator::PageCircularBufferIterator(unsigned int id,
                                                       PageCircularBufferPtr buffer,
                                                       pdb::PDBLoggerPtr logger) {
    this->id = id;
    this->buffer = buffer;
    this->logger = logger;
}

PageCircularBufferIterator::~PageCircularBufferIterator() {}

// will block when queue is empty and will remove the page from queue before return;
PDBPagePtr PageCircularBufferIterator::next() {
    return this->buffer->popPageFromHead();
}

/* Potential Bug: in certain cases, two iterators may simultaneously find that the buffer is closed,
 * but there are still data in the buffer. In this case, one iterator will not be able
 * to get a page, although it says hasNext() == true.
 *
 * Solution 1: wait for all elements in the queue to be fetched, then close the queue;
 * Solution 2: allow that you may get a nullptr by calling next(), and it is your responsibility to
 * check that!
 *
 * Now, we go solution 2, please let me know if this is a problem!
 */
bool PageCircularBufferIterator::hasNext() {
    bool ret = ((!this->buffer->isClosed()) || (!this->buffer->isEmpty()));
    PDB_COUT << "iter Id=" << this->id << ",ret=" << ret << "\n";
    return ret;
}

unsigned int PageCircularBufferIterator::getId() {
    return this->id;
}


#endif
