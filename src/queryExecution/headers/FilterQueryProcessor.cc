
#ifndef FILTER_QUERY_PROCESSOR_CC
#define FILTER_QUERY_PROCESSOR_CC

#include "InterfaceFunctions.h"
#include "FilterQueryProcessor.h"

namespace pdb {

template <class Output, class Input>
FilterQueryProcessor<Output, Input>::FilterQueryProcessor(Selection<Output, Input>& forMe) {

    // get a copy of the lambdas for query processing
    filterPred = forMe.getSelection(inputObject);
    finalized = false;
}

template <class Output, class Input>
FilterQueryProcessor<Output, Input>::FilterQueryProcessor(SimpleLambda<bool> filterPred) {

    // get a copy of the lambdas for filter processing
    this->filterPred = filterPred;
    finalized = false;
}

// no need to do anything
template <class Output, class Input>
void FilterQueryProcessor<Output, Input>::initialize() {
    //	filterFunc = filterPred.getFunc ();
    finalized = false;
}

// loads up another input page to process
template <class Output, class Input>
void FilterQueryProcessor<Output, Input>::loadInputPage(void* pageToProcess) {
    Record<Vector<Handle<Input>>>* myRec = (Record<Vector<Handle<Input>>>*)pageToProcess;
    inputVec = myRec->getRootObject();
    posInInput = 0;
}

// load up another output page to process
template <class Output, class Input>
void FilterQueryProcessor<Output, Input>::loadOutputPage(void* pageToWriteTo,
                                                         size_t numBytesInPage) {

    // kill the old allocation block
    blockPtr = nullptr;

    // create the new one
    blockPtr = std::make_shared<UseTemporaryAllocationBlock>(pageToWriteTo, numBytesInPage);

    // and here's where we write the ouput to
    outputVec = makeObject<Vector<Handle<Input>>>(10);
}

template <class Output, class Input>
bool FilterQueryProcessor<Output, Input>::fillNextOutputPage() {

    Vector<Handle<Input>>& myInVec = *(inputVec);
    Vector<Handle<Input>>& myOutVec = *(outputVec);

    // if we are finalized, see if there are some left over records
    if (finalized) {
        getRecord(outputVec);
        return false;
    }

    // we are not finalized, so process the page
    try {
        int vecSize = myInVec.size();
        for (; posInInput < vecSize; posInInput++) {
            inputObject = myInVec[posInInput];
            if (filterFunc()) {
                myOutVec.push_back(inputObject);
                // inputObject->print();
            }
        }
        // std::cout << "Returning false" << std::endl;

        return false;

    } catch (NotEnoughSpace& n) {
        // std::cout << "Got not enough space " << std::endl;
        getRecord(outputVec);
        // blockPtr = nullptr;
        return true;
    }
}

// must be called repeately after all of the input pages have been sent in...
template <class Output, class Input>
void FilterQueryProcessor<Output, Input>::finalize() {
    finalized = true;
}

// must be called before freeing the memory in output page
template <class Output, class Input>
void FilterQueryProcessor<Output, Input>::clearOutputPage() {
    outputVec = nullptr;
    blockPtr = nullptr;
}

// must be called before freeing the memory in input page
template <class Output, class Input>
void FilterQueryProcessor<Output, Input>::clearInputPage() {
    inputVec = nullptr;
    inputObject = nullptr;
}
}

#endif
