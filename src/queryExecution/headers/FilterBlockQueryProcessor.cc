
#ifndef FILTER_BLOCK_QUERY_PROCESSOR_CC
#define FILTER_BLOCK_QUERY_PROCESSOR_CC
#include "PDBDebug.h"
#include "InterfaceFunctions.h"
#include "FilterBlockQueryProcessor.h"
#include "SharedEmployee.h"

namespace pdb {

template <class Output, class Input>
FilterBlockQueryProcessor<Output, Input>::~FilterBlockQueryProcessor() {
    // std :: cout << "running FilterBlockQueryProcessor destructor" << std :: endl;
    this->inputBlock = nullptr;
    this->outputBlock = nullptr;
    this->context = nullptr;
    this->inputObject = nullptr;
}


template <class Output, class Input>
FilterBlockQueryProcessor<Output, Input>::FilterBlockQueryProcessor(
    Selection<Output, Input>& forMe) {

    // get a copy of the lambdas for query processing
    filterPred = forMe.getProjectionSelection(inputObject);
    finalized = false;
}

template <class Output, class Input>
FilterBlockQueryProcessor<Output, Input>::FilterBlockQueryProcessor(SimpleLambda<bool> filterPred) {

    // get a copy of the lambdas for filter processing
    this->filterPred = filterPred;
    finalized = false;
}

// no need to do anything
template <class Output, class Input>
void FilterBlockQueryProcessor<Output, Input>::initialize() {
    filterFunc = filterPred.getFunc();
    finalized = false;
}

// loads up another input page to process
template <class Output, class Input>
void FilterBlockQueryProcessor<Output, Input>::loadInputBlock(Handle<GenericBlock> inputBlock) {
    this->inputBlock = inputBlock;
    this->batchSize = this->inputBlock->getBlock().size();
    posInInput = 0;
}

// load up another output page to process
template <class Output, class Input>
Handle<GenericBlock>& FilterBlockQueryProcessor<Output, Input>::loadOutputBlock() {

    // and here's where we write the ouput to
    this->outputBlock = makeObject<GenericBlock>();
    return this->outputBlock;
}

template <class Output, class Input>
bool FilterBlockQueryProcessor<Output, Input>::fillNextOutputBlock() {
    // std :: cout << "Filter processor is running" << std :: endl;
    Vector<Handle<Output>>& myInVec = (inputBlock->getBlock());
    Vector<Handle<Output>>& myOutVec = (outputBlock->getBlock());

    // if we are finalized, see if there are some left over records
    if (finalized) {
        return false;
    }

    // int totalObjects = 0;
    // we are not finalized, so process the page
    try {
        int vecSize = myInVec.size();
        // std :: cout << "filterProcessor: posInInput=" << posInInput << ",input object num =" <<
        // vecSize << std :: endl;
        for (; posInInput < vecSize; posInInput++) {
            inputObject = myInVec[posInInput];
            // std :: cout << "posInInput=" << posInInput << std :: endl;
            if (filterFunc()) {
                // std :: cout << "to push back posInInput=" << posInInput << std :: endl;
                myOutVec.push_back(inputObject);
                // totalObjects ++;
                // std :: cout << "push back posInInput=" << posInInput << std :: endl;
            }
        }
        // std :: cout << "Filter processor processed an input block with "<< totalObjects << "
        // output objects" << std :: endl;
        return false;

    } catch (NotEnoughSpace& n) {
        // std :: cout << "Filter processor consumed current page with "<< totalObjects << "
        // objects" << std :: endl;
        // std :: cout << "posInInput =" << posInInput << std :: endl;
        if (this->context != nullptr) {
            // getRecord (context->outputVec);
            context->setOutputFull(true);
        }
        return true;
    }
}

// must be called repeately after all of the input pages have been sent in...
template <class Output, class Input>
void FilterBlockQueryProcessor<Output, Input>::finalize() {
    finalized = true;
}

// must be called before freeing the memory in output page
template <class Output, class Input>
void FilterBlockQueryProcessor<Output, Input>::clearOutputBlock() {
    outputBlock = nullptr;
}

// must be called before freeing the memory in input page
template <class Output, class Input>
void FilterBlockQueryProcessor<Output, Input>::clearInputBlock() {
    inputBlock = nullptr;
    inputObject = nullptr;
}
}

#endif
