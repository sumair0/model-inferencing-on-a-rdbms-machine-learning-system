#ifndef INT_DOUBLE_VECTOR_PAIR_H
#define INT_DOUBLE_VECTOR_PAIR_H

#include "Object.h"
#include "PDBVector.h"
#include "Handle.h"
#include <cstddef>

/* This class is for storing a (int, double vector) pair */
using namespace pdb;
class IntDoubleVectorPair : public Object {

private:
public:
    int myInt;
    Vector<double> myVector;

    ENABLE_DEEP_COPY

    ~IntDoubleVectorPair() {}
    IntDoubleVectorPair() {}

    IntDoubleVectorPair(int fromInt, Handle<Vector<double>>& fromVector) {
        this->myInt = fromInt;
        this->myVector = *fromVector;
    }

    void setInt(int fromInt) {
        this->myInt = fromInt;
    }

    void setVector(Handle<Vector<double>>& fromVector) {
        int size = fromVector->size();
        for (int i = 0; i < size; i++) {
            myVector.push_back((*fromVector)[i]);
        }
    }

    int getInt() {
        return this->myInt;
    }

    unsigned getUnsigned() {
        return this->myInt;
    }

    Vector<double>& getVector() {
        return this->myVector;
    }

    int& getKey() {
        return this->myInt;
    }

    Vector<double>& getValue() {
        return this->myVector;
    }
};

/* Overload the + operator */
namespace pdb {
inline Vector<double>& operator+(Vector<double>& lhs, Vector<double>& rhs) {
    int size = lhs.size();
    if (size != rhs.size()) {
        std::cout << "You cannot add two vectors in different sizes!" << std::endl;
        return lhs;
    }
    for (int i = 0; i < size; ++i) {
        if (rhs[i] != 0)
            lhs[i] = lhs[i] + rhs[i];
    }
    return lhs;
}
}

#endif
