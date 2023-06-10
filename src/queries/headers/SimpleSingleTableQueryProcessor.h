
#ifndef SINGLE_TABLE_QP_H
#define SINGLE_TABLE_QP_H

#include <memory>
#include "Handle.h"

namespace pdb {

class SimpleSingleTableQueryProcessor;
typedef std::shared_ptr<SimpleSingleTableQueryProcessor> SimpleSingleTableQueryProcessorPtr;

// this pure virtual class is spit out by a simple query class (like the Selection class)... it is
// then
// used by the system to process queries
//
class SimpleSingleTableQueryProcessor {

public:
    // must be called before the query processor is asked to do anything
    virtual void initialize() = 0;

    // loads up another input page to process
    virtual void loadInputPage(void* pageToProcess) = 0;

    // load up another output page to process
    virtual void loadOutputPage(void* pageToWriteTo, size_t numBytesInPage) = 0;

    virtual void loadInputObject(Handle<Object>& objectToProcess) {}

    // attempts to fill the next output page with data.  Returns true if it can.  If it
    // cannot, returns false, and the next call to loadInputPage should be made
    virtual bool fillNextOutputPage() = 0;

    // must be called after all of the input pages have been sent in
    virtual void finalize() = 0;

    // must be called before free the data in output page
    virtual void clearOutputPage() = 0;

    // must be called before free the data in input page
    virtual void clearInputPage() = 0;

    // has to process input page or not
    virtual bool needsProcessInput() {
        return true;
    }

    //to return number of unique hash keys
    virtual int getNumHashKeys() {
        return 0;
    }


};
}

#endif
