#ifndef TEST_85_H
#define TEST_85_H

// by Jia, May 2017

#include "Handle.h"
#include "Lambda.h"
#include "PDBClient.h"
#include "Supervisor.h"
#include "Employee.h"
#include "LambdaCreationFunctions.h"
#include "UseTemporaryAllocationBlock.h"
#include "Pipeline.h"
#include "SelectionComp.h"
#include "ScanSupervisorSet.h"
#include "WriteBuiltinEmployeeSet.h"
#include "SupervisorMultiSelection.h"
#include "SillyAggregation.h"
#include "AllSelectionWithCreation.h"
#include "DepartmentTotal.h"
#include "VectorSink.h"
#include "HashSink.h"
#include "MapTupleSetIterator.h"
#include "VectorTupleSetIterator.h"
#include "ComputePlan.h"
#include <ctime>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <chrono>
#include <fcntl.h>


// to run the aggregate, the system first passes each through the hash operation...
// then the system
using namespace pdb;

int main(int argc, char* argv[]) {


    bool printResult = true;
    bool clusterMode = false;
    std::cout << "Usage: #printResult[Y/N] #clusterMode[Y/N] #dataSize[MB] #masterIp #addData[Y/N]"
              << std::endl;
    if (argc > 1) {
        if (strcmp(argv[1], "N") == 0) {
            printResult = false;
            std::cout << "You successfully disabled printing result." << std::endl;
        } else {
            printResult = true;
            std::cout << "Will print result." << std::endl;
        }

    } else {
        std::cout << "Will print result. If you don't want to print result, you can add N as the "
                     "first parameter to disable result printing."
                  << std::endl;
    }

    if (argc > 2) {
        if (strcmp(argv[2], "Y") == 0) {
            clusterMode = true;
            std::cout << "You successfully set the test to run on cluster." << std::endl;
        } else {
            clusterMode = false;
            std::cout << "ERROR: cluster mode must be true" << std::endl;
            exit(1);
        }
    } else {
        std::cout << "Will run on local node. If you want to run on cluster, you can add any "
                     "character as the second parameter to run on the cluster configured by "
                     "$PDB_HOME/conf/serverlist."
                  << std::endl;
    }

    int numOfMb = 1024;  // by default we add 1024MB data
    if (argc > 3) {
        numOfMb = atoi(argv[3]);
    }
    std::cout << "To add data with size: " << numOfMb << "MB" << std::endl;

    std::string masterIp = "localhost";
    if (argc > 4) {
        masterIp = argv[4];
    }
    std::cout << "Master IP Address is " << masterIp << std::endl;

    bool whetherToAddData = true;
    if (argc > 5) {
        if (strcmp(argv[5], "N") == 0) {
            whetherToAddData = false;
        }
    }

    PDBLoggerPtr clientLogger = make_shared<PDBLogger>("clientLog");

    PDBClient pdbClient(
            8108,
            masterIp,
            clientLogger,
            false,
            true);

    CatalogClient catalogClient(
            8108,
            masterIp,
            clientLogger);

    string errMsg;

    if (whetherToAddData == true) {


        // now, create a new database
        if (!pdbClient.createDatabase("test85_db", errMsg)) {
            cout << "Not able to create database: " + errMsg;
            exit(-1);
        } else {
            cout << "Created database.\n";
        }

        // now, create a new set in that database
        if (!pdbClient.createSet<Supervisor>("test85_db", "test85_set", errMsg)) {
            cout << "Not able to create set: " + errMsg;
            exit(-1);
        } else {
            cout << "Created set.\n";
        }


        // Step 2. Add data

        int total = 0;
        if (numOfMb > 0) {
            int numIterations = numOfMb / 64;
            int remainder = numOfMb - 64 * numIterations;
            if (remainder > 0) {
                numIterations = numIterations + 1;
            }
            for (int num = 0; num < numIterations; num++) {
                int blockSize = 64;
                if ((num == numIterations - 1) && (remainder > 0)) {
                    blockSize = remainder;
                }
                makeObjectAllocatorBlock(blockSize * 1024 * 1024, true);
                Handle<Vector<Handle<Supervisor>>> storeMe =
                    makeObject<Vector<Handle<Supervisor>>>();
                char first = 'A', second = 'B', third = 'C', fourth = 'D';
                char myString[5];
                myString[4] = 0;
                try {
                    for (int i = 0; true; i++) {

                        myString[0] = first;
                        myString[1] = second;
                        myString[2] = third;
                        myString[3] = fourth;

                        first++;
                        if (first == 'Z') {
                            first = 'A';
                            second++;
                            if (second == 'Z') {
                                second = 'A';
                                third++;
                                if (third == 'Z') {
                                    third = 'A';
                                    fourth++;
                                    if (fourth == 'Z') {
                                        fourth = 'A';
                                    }
                                }
                            }
                        }
                        // std :: cout << myString << std :: endl;
                        Handle<Supervisor> myData = makeObject<Supervisor>(
                            "Steve Stevens", 20 + (i % 29), std::string(myString), i * 34.4);
                        storeMe->push_back(myData);
                        total++;
                        for (int j = 0; j < 10; j++) {
                            Handle<Employee> temp;
                            if (i % 2 == 0) {
                                temp = makeObject<Employee>("Steve Stevens",
                                                            20 + ((i + j) % 29),
                                                            std::string(myString),
                                                            j * 3.54);
                            } else {
                                temp = makeObject<Employee>("Albert Albertson",
                                                            20 + ((i + j) % 29),
                                                            std::string(myString),
                                                            j * 3.54);
                            }
                            (*storeMe)[i]->addEmp(temp);
                        }
                    }

                } catch (pdb::NotEnoughSpace& n) {
                    if (!pdbClient.sendData<Supervisor>(
                            std::pair<std::string, std::string>("test85_set", "test85_db"),
                            storeMe,
                            errMsg)) {
                        std::cout << "Failed to send data to dispatcher server" << std::endl;
                        return -1;
                    }
                }
                PDB_COUT << blockSize << "MB data sent to dispatcher server~~" << std::endl;
            }

            std::cout << "total=" << total << std::endl;

            // to write back all buffered records
            pdbClient.flushData(errMsg);
        }
    }

    // now, create a new set in that database to store output data
    PDB_COUT << "to create a new set for storing output data" << std::endl;
    if (!pdbClient.createSet<Employee>("test85_db", "output_set1", errMsg)) {
        cout << "Not able to create set: " + errMsg;
        exit(-1);
    } else {
        cout << "Created set.\n";
    }

    // now, create another new set in that database to store output data
    PDB_COUT << "to create a new set for storing output data" << std::endl;
    if (!pdbClient.createSet<DepartmentTotal>("test85_db", "output_set2", errMsg)) {
        cout << "Not able to create set: " + errMsg;
        exit(-1);
    } else {
        cout << "Created set.\n";
    }


    // this is the object allocation block where all of this stuff will reside
    const UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};
    // register this query class
    pdbClient.registerType("libraries/libWriteBuiltinEmployeeSet.so", errMsg);
    pdbClient.registerType("libraries/libScanSupervisorSet.so", errMsg);
    pdbClient.registerType("libraries/libSupervisorMultiSelection.so", errMsg);
    pdbClient.registerType("libraries/libSillyAggregation.so", errMsg);
    pdbClient.registerType("libraries/libAllSelectionWithCreation.so", errMsg);

    // create all of the computation objects
    Handle<Computation> myScanSet = makeObject<ScanSupervisorSet>("test85_db", "test85_set");
    Handle<Computation> myFlatten = makeObject<SupervisorMultiSelection>();
    myFlatten->setInput(myScanSet);
    Handle<Computation> myAgg = makeObject<SillyAggregation>("test85_db", "output_set2");
    myAgg->setInput(myFlatten);
    Handle<Computation> myFilter = makeObject<AllSelectionWithCreation>();
    myFilter->setInput(myFlatten);
    Handle<Computation> myWriteSet =
        makeObject<WriteBuiltinEmployeeSet>("test85_db", "output_set1");
    myWriteSet->setInput(myFilter);

    auto begin = std::chrono::high_resolution_clock::now();

    if (!pdbClient.executeComputations(errMsg, myWriteSet, myAgg)) {
        std::cout << "Query failed. Message was: " << errMsg << "\n";
        return 1;
    }
    std::cout << std::endl;

    auto end = std::chrono::high_resolution_clock::now();
    // std::cout << "Time Duration: " <<
    //      std::chrono::duration_cast<std::chrono::nanoseconds>(end-begin).count() << " ns." <<
    //      std::endl;

    std::cout << std::endl;

    // print the resuts of the first output set
    if (printResult == true) {
        std::cout << "to print result..." << std::endl;
        SetIterator<Employee> result =
                pdbClient.getSetIterator<Employee>("test85_db", "output_set1");
        std::cout << "Query results 1: ";
        int count = 0;
        for (auto a : result) {
            count++;
            if (count % 1000 == 0) {
                std::cout << count << ":";
                a->print();
            }
        }
        std::cout << "selection output count:" << count << "\n";
    }

    // print the resuts of the second output set
    if (printResult == true) {
        std::cout << "to print result..." << std::endl;
        SetIterator<DepartmentTotal> result =
                pdbClient.getSetIterator<DepartmentTotal>("test85_db", "output_set2");
        std::cout << "Query results 2: ";
        int count = 0;
        for (auto a : result) {
            count++;
            std::cout << count << ":";
            a->print();
        }
        std::cout << "aggregation output count:" << count << "\n";
    }

    if (clusterMode == false) {
        // and delete the sets
        pdbClient.deleteSet("test85_db", "output_set1");
        pdbClient.deleteSet("test85_db", "output_set2");
    } else {
        if (!pdbClient.removeSet("test85_db", "output_set1", errMsg)) {
            cout << "Not able to remove set: " + errMsg;
            exit(-1);
        } else {
            cout << "Removed set.\n";
        }
        if (!pdbClient.removeSet("test85_db", "output_set2", errMsg)) {
            cout << "Not able to remove set: " + errMsg;
            exit(-1);
        } else {
            cout << "Removed set.\n";
        }
    }
    int code = system("scripts/cleanupSoFiles.sh");
    if (code < 0) {
        std::cout << "Can't cleanup so files" << std::endl;
    }
    std::cout << "Time Duration: "
              << std::chrono::duration_cast<std::chrono::duration<float>>(end - begin).count()
              << " secs." << std::endl;
}


#endif
