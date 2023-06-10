
#include "Object.h"
#include "PDBTemplateBase.h"
#include "Handle.h"
#include "Array.h"

#ifndef VECTOR_H
#define VECTOR_H

#include <cstddef>
#include <iostream>
#include <iterator>
#include <cstring>

// PRELOAD %Vector <Nothing>%

namespace pdb {

template <class ValueType>
class JoinMap;

// This is the basic Vector type that works correcrly with Objects and Handles.
// The operations have exactly the same interface as std :: vector, except that
// not all operations are implemented.

template <class TypeContained>
class Vector : public Object {

private:
    // this is where the data are actually stored
    Handle<Array<TypeContained>> myArray;

public:
    ENABLE_DEEP_COPY

    // this constructor pre-allocates initSize slots, and then initializes
    // numUsed of them, calling a no-arg constructor on each.  Thus, after
    // this call, size () will return numUsed
    Vector(uint32_t initSize, uint32_t numUsed);

    // this constructor pre-allocates initSize slots, but does not do anything
    // to them.  Thus, after this call, size () will return zero
    Vector(uint32_t initSize);

    // these operations all have the same semantics as in std :: vector
    Vector();
    size_t size() const;
    TypeContained& operator[](uint32_t which);
    TypeContained& operator[](uint32_t which) const;
    void assign(uint32_t which, const TypeContained& val);
    void push_back(const TypeContained& val);
    void push_back();
    void pop_back();
    void clear();
    TypeContained* c_ptr() const;
    void resize(uint32_t toMe);

    // added by Shangyu
    void print() const;
    void fill(const TypeContained& val);

    // beause the communicator needs to see inside to do efficient sends
    friend class PDBCommunicator;
    // this method is used to do a deep copy of this join map
    template <class T> friend Handle<JoinMap<T>> deepCopyJoinMap(Handle<JoinMap<T>>& copyMe);

};
}

#include "PDBVector.cc"

#endif
