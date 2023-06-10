#ifndef AGGOUT_PROCESSOR_CC
#define AGGOUT_PROCESSOR_CC

#include "AggOutProcessor.h"

namespace pdb {

// OutputClass must have getKey() and getValue() methods implemented
template <class OutputClass, class KeyType, class ValueType>
AggOutProcessor<OutputClass, KeyType, ValueType>::AggOutProcessor() {

    finalized = false;
    begin = nullptr;
    end = nullptr;
}

// initialize
template <class OutputClass, class KeyType, class ValueType>
void AggOutProcessor<OutputClass, KeyType, ValueType>::initialize() {
    finalized = false;
}

// loads up another input page to process
template <class OutputClass, class KeyType, class ValueType>
void AggOutProcessor<OutputClass, KeyType, ValueType>::loadInputPage(void* pageToProcess) {

    Record<Map<KeyType, ValueType>>* myRec = (Record<Map<KeyType, ValueType>>*)pageToProcess;
    inputData = myRec->getRootObject();
    if (begin != nullptr) {
        delete begin;
    }
    if (end != nullptr) {
        delete end;
    }
    begin = new PDBMapIterator<KeyType, ValueType>(inputData->getArray(), true);
    if (begin == nullptr) {
        std::cout << "AggOutProcessor.cc: Failed to allocate memory" << std::endl;
        exit(1);
    }
    end = new PDBMapIterator<KeyType, ValueType>(inputData->getArray());
    if (end == nullptr) {
        std::cout << "AggOutProcessor.cc: Failed to allocate memory" << std::endl;
        exit(1);
    }
}

// loads up another output page to write results to
template <class OutputClass, class KeyType, class ValueType>
void AggOutProcessor<OutputClass, KeyType, ValueType>::loadOutputPage(void* pageToWriteTo,
                                                                      size_t numBytesInPage) {

    blockPtr = nullptr;
    blockPtr = std::make_shared<UseTemporaryAllocationBlock>(pageToWriteTo, numBytesInPage);
    outputData = makeObject<Vector<Handle<OutputClass>>>();
    pos = 0;
}

template <class OutputClass, class KeyType, class ValueType>
bool AggOutProcessor<OutputClass, KeyType, ValueType>::fillNextOutputPage() {

    // if we are finalized, see if there are some left over records
    if (finalized) {
        getRecord(outputData);
        return false;
    }

    // we are not finalized, so process the page
    try {
        // see if there are any more items in current map to iterate over
        while (true) {

            if (!((*begin) != (*end))) {
                PDB_COUT << "AggOutProcessor: processed " << pos << " elements in the input page"
                         << std::endl;
                return false;
            }

            Handle<OutputClass> temp = makeObject<OutputClass>();
            temp->getKey() = (*(*begin)).key;
            temp->getValue() = (*(*begin)).value;
            outputData->push_back(temp);
            pos++;
            ++(*begin);
        }

    } catch (NotEnoughSpace& n) {
        getRecord(outputData);
        return true;
    }
}

template <class OutputClass, class KeyType, class ValueType>
void AggOutProcessor<OutputClass, KeyType, ValueType>::finalize() {
    finalized = true;
}

template <class OutputClass, class KeyType, class ValueType>
void AggOutProcessor<OutputClass, KeyType, ValueType>::clearOutputPage() {
    blockPtr = nullptr;
    outputData = nullptr;
}

template <class OutputClass, class KeyType, class ValueType>
void AggOutProcessor<OutputClass, KeyType, ValueType>::clearInputPage() {
    inputData = nullptr;
}
}


#endif
