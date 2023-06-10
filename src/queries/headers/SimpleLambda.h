
#ifndef SIMPLELAMBDA_H
#define SIMPLELAMBDA_H

#include <memory>
#include <vector>
#include <functional>
#include "Literal.h"
#include "Object.h"
#include "Handle.h"

namespace pdb {

template <typename Out>
class LambdaType;

// this is the lamda type... queries are written by supplying code that
// creates these objects
template <typename Out>
class SimpleLambda {

    std::shared_ptr<LambdaType<Out>> myData;

public:
    SimpleLambda() {}

    // returns a string giving the entire "parse tree" making up this lambda
    std::string toString() {
        return myData->toString();
    }

    // returns the function that actucally computes this lambda
    std::function<Out()> getFunc() {
        return myData->getFunc();
    }

    // copy constructor
    SimpleLambda(std::shared_ptr<LambdaType<Out>> myDataIn) {
        myData = myDataIn;
    }

    // get a list of pointers to all of the input parameters that are used by this lambda
    // this is going to be useful for compilation, so that we know what parameters are
    // referenced by what lambdas
    std::vector<Handle<Object>*>& getParams() {
        return myData->getParams();
    }

    // make a lambda with no parameters out of a function
    SimpleLambda(std::function<Out()> funcArg) {
        myData = std::make_shared<Literal<Out>>(funcArg);
    }

    // make a lambda with the specified parameter out of a function
    template <class T1, class... T2>
    SimpleLambda(Handle<T1>& firstParam, T2... rest) : SimpleLambda(rest...) {
        Handle<Object>* temp = (Handle<Object>*)&firstParam;
        myData->addParam(temp);
    }

    template <typename ClassType>
    SimpleLambda(Handle<ClassType>&, Out (ClassType::*arg)()) {
        myData = nullptr;
    }

    template <typename ClassType>
    static SimpleLambda<Out> makeSimpleLambdaUsingMember(ClassType&, Out&) {
        SimpleLambda<Out> temp;
        temp.myData = nullptr;
        return temp;
    }

    bool operator==(const SimpleLambda<Out>& rhs) {
        return myData == rhs.myData;
    }
};

// this helper function allows us to easily create Lambda objects
template <typename T, typename F>
auto makeSimpleLambda(T&& val, F&& func) {
    return SimpleLambda<decltype(func())>(val, func);
}

template <typename F>
auto makeSimpleLambda(F&& func) {
    return SimpleLambda<decltype(func())>(func);
}

template <typename ReturnType, typename ClassType>
auto makeSimpleLambdaUsingMethod(std::string,
                                 Handle<ClassType> var,
                                 ReturnType (ClassType::*arg)()) {
    return SimpleLambda<ReturnType>(var, arg);
}

template <typename ReturnType, typename ClassType>
auto makeSimpleLambdaUsingMember(std::string, ClassType& var, ReturnType& member) {
    return SimpleLambda<ReturnType>::makeSimpleLambdaUsingMember(var, member);
}

#define makeSimpleLambdaFromMethod(VAR, METHOD) \
    (makeSimpleLambdaUsingMethod(               \
        std::string(#METHOD), VAR, &std::remove_reference<decltype(*VAR)>::type::METHOD))

#define makeSimpleLambdaFromMember(VAR, MEMBER) \
    (makeSimpleLambdaUsingMember(std::string(#MEMBER), VAR, VAR->MEMBER))
}
#endif
