
#ifndef SIMPLE_REQUEST_CC
#define SIMPLE_REQUEST_CC

#include <functional>
#include <string>

#include "InterfaceFunctions.h"
#include "UseTemporaryAllocationBlock.h"
#include "PDBCommunicator.h"

using std::function;
using std::string;

#ifndef MAX_RETRIES
#define MAX_RETRIES 5
#endif
#define BLOCK_HEADER_SIZE 20
namespace pdb {

template <class RequestType, class ResponseType, class ReturnType, class... RequestTypeParams>
ReturnType simpleRequest(PDBLoggerPtr myLogger,
                         int port,
                         std::string address,
                         ReturnType onErr,
                         size_t bytesForRequest,
                         function<ReturnType(Handle<ResponseType>)> processResponse,
                         RequestTypeParams&&... args) {

    int numRetries = 0;

    while (numRetries <= MAX_RETRIES) {
        PDBCommunicator temp;
        string errMsg;
        bool success;

        if (temp.connectToInternetServer(myLogger, port, address, errMsg)) {
            myLogger->error(errMsg);
            myLogger->error("simpleRequest: not able to connect to server.\n");
            // return onErr;
            std::cout << "ERROR: can not connect to remote server with port=" << port
                      << " and address=" << address << std::endl;
            return onErr;
        }
        myLogger->info(std::string("Successfully connected to remote server with port=") +
                       std::to_string(port) + std::string(" and address=") + address);
        PDB_COUT << "Successfully connected to remote server with port=" << port
                 << " and address=" << address << std::endl;
        // build the request
        PDB_COUT << "bytesForRequest=" << bytesForRequest << std::endl;
        if (bytesForRequest <= BLOCK_HEADER_SIZE) {
            std::cout << "ERROR: too small buffer size for processing simple request" << std::endl;
            return onErr;
        }
        const UseTemporaryAllocationBlock tempBlock{bytesForRequest};
        PDB_COUT << "to make object" << std::endl;
        Handle<RequestType> request = makeObject<RequestType>(args...);
        ;
        PDB_COUT << "to send object" << std::endl;
        if (!temp.sendObject(request, errMsg)) {
            myLogger->error(errMsg);
            myLogger->error("simpleRequest: not able to send request to server.\n");
            if (numRetries < MAX_RETRIES) {
                numRetries++;
                continue;
            } else {
                return onErr;
            }
        }
        PDB_COUT << "sent object..." << std::endl;
        // get the response and process it
        ReturnType finalResult;
        size_t objectSize = temp.getSizeOfNextObject();
        if (objectSize == 0) {
            if (numRetries < MAX_RETRIES) {
                numRetries++;
                continue;
            } else {
                return onErr;
            }
        }
        void* memory = calloc(objectSize, 1);
        if (memory == nullptr) {
            errMsg = "FATAL ERROR in simpleRequest: Can't allocate memory";
            myLogger->error(errMsg);
            std::cout << errMsg << std::endl;
            exit(-1);
        }
        {
            Handle<ResponseType> result = temp.getNextObject<ResponseType>(memory, success, errMsg);
            if (!success) {
                myLogger->error(errMsg);
                myLogger->error("simpleRequest: not able to get next object over the wire.\n");
                /// JiaNote: we need free memory here !!!

                free(memory);
                if (numRetries < MAX_RETRIES) {
                    numRetries++;
                    continue;
                } else {
                    return onErr;
                }
            }

            finalResult = processResponse(result);
        }
        free(memory);
        return finalResult;
    }
    return onErr;
}

template <class RequestType, class SecondRequestType, class ResponseType, class ReturnType>
ReturnType simpleDoubleRequest(PDBLoggerPtr myLogger,
                               int port,
                               std::string address,
                               ReturnType onErr,
                               size_t bytesForRequest,
                               function<ReturnType(Handle<ResponseType>)> processResponse,
                               Handle<RequestType>& firstRequest,
                               Handle<SecondRequestType>& secondRequest) {

    PDBCommunicator temp;
    string errMsg;
    bool success;

    if (temp.connectToInternetServer(myLogger, port, address, errMsg)) {
        myLogger->error(errMsg);
        myLogger->error("simpleRequest: not able to connect to server.\n");
        // return onErr;
        std::cout << "ERROR: can not connect to remote server with port=" << port
                  << " and address=" << address << std::endl;
        return onErr;
    }
    std::cout << "Successfully connected to remote server with port=" << port
              << " and address=" << address << std::endl;
    // build the request
    if (!temp.sendObject(firstRequest, errMsg)) {
        myLogger->error(errMsg);
        myLogger->error("simpleDoubleRequest: not able to send first request to server.\n");
        return onErr;
    }

    if (!temp.sendObject(secondRequest, errMsg)) {
        myLogger->error(errMsg);
        myLogger->error("simpleDoubleRequest: not able to send second request to server.\n");
        return onErr;
    }

    // get the response and process it
    ReturnType finalResult;
    void* memory = calloc(temp.getSizeOfNextObject(), 1);
    if (memory == nullptr) {
        std::cout << "SimpleRequest.cc: failed to allocate memory" << std::endl;
        exit(1);
    }
    {
        Handle<ResponseType> result = temp.getNextObject<ResponseType>(memory, success, errMsg);
        if (!success) {
            myLogger->error(errMsg);
            myLogger->error("simpleRequest: not able to get next object over the wire.\n");
            return onErr;
        }

        finalResult = processResponse(result);
    }
    free(memory);
    return finalResult;
}
}
#endif
