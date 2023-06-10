
#ifndef LAMBDA_TYPES_H
#define LAMBDA_TYPES_H

#include "LambdaType.h"

namespace pdb {

// this lists the differenet LambdaType classes
template <typename T>
class PlusOp : public LambdaType<T> {

    Lambda<T> leftLambda;
    Lambda<T> rightLambda;

public:
    ~PlusOp() {}

    void addParam(Handle<Object>& addMe) {
        cout << "Bad: why add a param to a plus?\n";
    }

    std::string toString() {
        return "(" + leftLambda.toString() + " + " + rightLambda.toString() + ")";
    }

    PlusOp(Lambda<T> leftLambda, Lambda<T> rightLambda)
        : leftLambda(leftLambda), rightLambda(rightLambda) {}

    std::function<T()> getFunc() {
        std::function<T()> left = leftLambda.getFunc();
        std::function<T()> right = rightLambda.getFunc();
        return [=] { return left() + right(); };
    }

    FuncType getType() {
        return FuncType::Plus;
    }

    vector<Handle<Object>*> getParams() {
        vector<Handle<Object>*> left = leftLambda->getParams();
        vector<Handle<Object>*> right = rightLambda->getParams();
        vector<Handle<Object>*> returnVal;
        returnVal.insert(returnVal.end(), left.begin(), left.end());
        returnVal.insert(returnVal.end(), right.begin(), right.end());
        std::sort(returnVal.begin(), returnVal.end());
        returnVal.erase(std::unique(returnVal.begin(), returnVal.end()), returnVal.end());
        return returnVal;
    }
};

template <typename L, typename R>
class EqualsOp : public LambdaType<bool> {

    Lambda<L> leftLambda;
    Lambda<R> rightLambda;

public:
    ~EqualsOp() {}

    void addParam(Handle<Object>& addMe) {
        cout << "Bad: why add a param to an equals?\n";
    }

    std::string toString() {
        return "(" + leftLambda.toString() + " == " + rightLambda.toString() + ")";
    }

    EqualsOp(Lambda<L> leftLambda, Lambda<R> rightLambda)
        : leftLambda(leftLambda), rightLambda(rightLambda) {}

    std::function<bool()> getFunc() {
        std::function<L()> left = leftLambda.getFunc();
        std::function<R()> right = rightLambda.getFunc();
        return [=] { return left() == right(); };
    }

    FuncType getType() {
        return FuncType::Equals;
    }

    vector<Handle<Object>*> getParams() {
        vector<Handle<Object>*> left = leftLambda->getParams();
        vector<Handle<Object>*> right = rightLambda->getParams();
        vector<Handle<Object>*> returnVal;
        returnVal.insert(returnVal.end(), left.begin(), left.end());
        returnVal.insert(returnVal.end(), right.begin(), right.end());
        std::sort(returnVal.begin(), returnVal.end());
        returnVal.erase(std::unique(returnVal.begin(), returnVal.end()), returnVal.end());
        return returnVal;
    }
};

template <typename L, typename R>
class GreaterThanOp : public LambdaType<bool> {

    Lambda<L> leftLambda;
    Lambda<R> rightLambda;

public:
    ~GreaterThanOp() {}

    void addParam(Handle<Object>& addMe) {
        cout << "Bad: why add a param to a greater than?\n";
    }

    std::string toString() {
        return "(" + leftLambda.toString() + " > " + rightLambda.toString() + ")";
    }

    GreaterThanOp(Lambda<L> leftLambda, Lambda<R> rightLambda)
        : leftLambda(leftLambda), rightLambda(rightLambda) {}

    std::function<bool()> getFunc() {
        std::function<L()> left = leftLambda.getFunc();
        std::function<R()> right = rightLambda.getFunc();
        return [=] { return left() > right(); };
    }

    FuncType getType() {
        return FuncType::GreaterThan;
    }

    vector<Handle<Object>*> getParams() {
        vector<Handle<Object>*> left = leftLambda->getParams();
        vector<Handle<Object>*> right = rightLambda->getParams();
        vector<Handle<Object>*> returnVal;
        returnVal.insert(returnVal.end(), left.begin(), left.end());
        returnVal.insert(returnVal.end(), right.begin(), right.end());
        std::sort(returnVal.begin(), returnVal.end());
        returnVal.erase(std::unique(returnVal.begin(), returnVal.end()), returnVal.end());
        return returnVal;
    }
};
}

#endif
