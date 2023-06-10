#ifndef TEST_76_H
#define TEST_76_H

// by Jia, May 2017

#include "Handle.h"
#include "Lambda.h"
#include "PDBClient.h"
#include "LambdaCreationFunctions.h"
#include "UseTemporaryAllocationBlock.h"
#include "Pipeline.h"
#include "SillySelection.h"
#include "SelectionComp.h"
#include "FinalSelection.h"
#include "AggregateComp.h"
#include "VectorSink.h"
#include "HashSink.h"
#include "MapTupleSetIterator.h"
#include "VectorTupleSetIterator.h"
#include "ComputePlan.h"
#include "Count.h"
#include "StringIntPair.h"
#include "ScanIntSet.h"
#include "ScanStringSet.h"
#include "ScanStringIntPairSet.h"
#include "SillyJoin.h"
#include "WriteStringSet.h"
#include "PDBString.h"
#include "LambdaIdentifier.h"
#include <ctime>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <chrono>
#include <fcntl.h>

/* distributed join test case */
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
            std::cout << "ERROR: cluster mode must be Y" << std::endl;
            exit(1);
        }
    } else {
        std::cout << "Will run on local node. If you want to run on cluster, you can add any "
                     "character as the second parameter to run on the cluster configured by "
                     "$PDB_HOME/conf/serverlist."
                  << std::endl;
    }

    int numOfMb = 256;  // by default we add 256MB data for each table
    int numOfMb1 = 128;
    int numOfMb2 = 64;
    if (argc > 3) {
        numOfMb = atoi(argv[3])/4;
        numOfMb1 = numOfMb / 2;
        numOfMb2 = numOfMb / 4;
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

    bool whetherToUseLambda = false;
    if (argc > 6) {
        if (strcmp(argv[6], "Y") == 0) {
           whetherToUseLambda = true;
        }
    }



    PDBLoggerPtr clientLogger = make_shared<PDBLogger>("clientLog");

    PDBClient pdbClient(
            8108,
            masterIp,
            clientLogger,
            false,
            true,
            argv[0]);

    CatalogClient catalogClient(
            8108,
            masterIp,
            clientLogger);

    string errMsg;
    // register this query class
    pdbClient.registerType("libraries/libSillyJoin.so", errMsg);
    pdbClient.registerType("libraries/libScanIntSet.so", errMsg);
    pdbClient.registerType("libraries/libScanStringIntPairSet.so", errMsg);
    pdbClient.registerType("libraries/libScanStringSet.so", errMsg);
    pdbClient.registerType("libraries/libWriteStringSet.so", errMsg);


    if (whetherToAddData == true) {


        // now, create a new database
        if (!pdbClient.createDatabase("test76_db", errMsg)) {
            cout << "Not able to create database: " + errMsg;
            exit(-1);
        } else {
            cout << "Created database.\n";
        }

        // now, create the int set in that database
        Handle<LambdaIdentifier> myLambdaForSet1 = nullptr;
        if (whetherToUseLambda) {
             myLambdaForSet1 = makeObject<LambdaIdentifier>("bin/test76-0", "JoinComp_3", "self_0");
        }
        if (!pdbClient.createSet<int>("test76_db", "test76_set1", errMsg, (size_t)64*(size_t)1024*(size_t)1024, "ToLoadSet1ForTest76", nullptr, myLambdaForSet1)) {
            cout << "Not able to create set: " + errMsg;
            exit(-1);
        } else {
            cout << "Created input set 1.\n";
        }

        // now, create the StringIntPair set in that database
        Handle<LambdaIdentifier> myLambdaForSet2 = nullptr;
        if (whetherToUseLambda) {
             myLambdaForSet2 = makeObject<LambdaIdentifier>("bin/test76-0", "JoinComp_3", "attAccess_1");
        }
        if (!pdbClient.createSet<StringIntPair>("test76_db", "test76_set2", errMsg, (size_t)64*(size_t)1024*(size_t)1024, "ToLoadSet2ForTest76", nullptr, myLambdaForSet2)) {
            cout << "Not able to create set: " + errMsg;
            exit(-1);
        } else {
            cout << "Created input set 2.\n";
        }

        // now, create the String set in that database
        Handle<LambdaIdentifier> myLambdaForSet3 = nullptr;
        if (whetherToUseLambda) {
            myLambdaForSet3 = makeObject<LambdaIdentifier> ("bin/test76-0", "JoinComp_3", "self_4");
        }
        if (!pdbClient.createSet<String>("test76_db", "test76_set3", errMsg, (size_t)64*(size_t)1024*(size_t)1024, "ToLoadSet3ForTest76", nullptr, myLambdaForSet3)) {
            cout << "Not able to create set: " + errMsg;
            exit(-1);
        } else {
            cout << "Created input set 3.\n";
        }



        // Step 2. Add data to set1
        int total = 0;
        int i = 0;
        if (numOfMb > 0) {
            int numIterations = numOfMb / 4;
            int remainder = numOfMb - 4 * numIterations;
            if (remainder > 0) {
                numIterations = numIterations + 1;
            }
            for (int num = 0; num < numIterations; num++) {
                int blockSize = 4;
                if ((num == numIterations - 1) && (remainder > 0)) {
                    blockSize = remainder;
                }
                makeObjectAllocatorBlock(blockSize * 1024 * 1024, true);
                Handle<Vector<Handle<int>>> storeMe = makeObject<Vector<Handle<int>>>();
                try {
                    for (i = 0; true; i++) {

                        Handle<int> myData = makeObject<int>(i);
                        storeMe->push_back(myData);
                        total++;
                    }

                } catch (pdb::NotEnoughSpace& n) {
                    std::cout << "got to " << i << " when producing data for input set 1.\n";
                    if (!pdbClient.sendData<int>(
                            std::pair<std::string, std::string>("test76_set1", "test76_db"),
                            storeMe,
                            errMsg)) {
                        std::cout << "Failed to send data to dispatcher server" << std::endl;
                        return -1;
                    }
                }
                PDB_COUT << blockSize << "MB data sent to dispatcher server~~" << std::endl;
            }

            std::cout << "input set 1: total=" << total << std::endl;

            // to write back all buffered records
            pdbClient.flushData(errMsg);
        }

        // Step 3. Add data to set2
        total = 0;
        i = 0;
        if (numOfMb1 > 0) {
            int numIterations = numOfMb1 / 4;
            int remainder = numOfMb1 - 4 * numIterations;
            if (remainder > 0) {
                numIterations = numIterations + 1;
            }
            for (int num = 0; num < numIterations; num++) {
                int blockSize = 4;
                if ((num == numIterations - 1) && (remainder > 0)) {
                    blockSize = remainder;
                }
                makeObjectAllocatorBlock(blockSize * 1024 * 1024, true);
                Handle<Vector<Handle<StringIntPair>>> storeMe =
                    makeObject<Vector<Handle<StringIntPair>>>();
                try {
                    for (i = 0; true; i++) {
                        std::ostringstream oss;
                        oss << "My string is " << i;
                        oss.str();
                        Handle<StringIntPair> myData = makeObject<StringIntPair>(oss.str(), i);
                        storeMe->push_back(myData);
                        total++;
                    }

                } catch (pdb::NotEnoughSpace& n) {
                    std::cout << "got to " << i << " when producing data for input set 2.\n";
                    if (!pdbClient.sendData<StringIntPair>(
                            std::pair<std::string, std::string>("test76_set2", "test76_db"),
                            storeMe,
                            errMsg)) {
                        std::cout << "Failed to send data to dispatcher server" << std::endl;
                        return -1;
                    }
                }
                PDB_COUT << blockSize << "MB data sent to dispatcher server~~" << std::endl;
            }

            std::cout << "input set 2: total=" << total << std::endl;

            // to write back all buffered records
            pdbClient.flushData(errMsg);
        }

        // Step 4. Add data to set3
        total = 0;
        i = 0;
        if (numOfMb2 > 0) {
            int numIterations = numOfMb2 / 4;
            int remainder = numOfMb2 - 4 * numIterations;
            if (remainder > 0) {
                numIterations = numIterations + 1;
            }
            for (int num = 0; num < numIterations; num++) {
                int blockSize = 4;
                if ((num == numIterations - 1) && (remainder > 0)) {
                    blockSize = remainder;
                }
                makeObjectAllocatorBlock(blockSize * 1024 * 1024, true);
                Handle<Vector<Handle<String>>> storeMe = makeObject<Vector<Handle<String>>>();
                try {
                    for (i = 0; true; i++) {
                        std::ostringstream oss;
                        oss << "My string is " << i;
                        oss.str();
                        Handle<String> myData = makeObject<String>(oss.str());
                        storeMe->push_back(myData);
                        total++;
                    }

                } catch (pdb::NotEnoughSpace& n) {
                    std::cout << "got to " << i << " when producing data for input set 3.\n";
                    if (!pdbClient.sendData<String>(
                            std::pair<std::string, std::string>("test76_set3", "test76_db"),
                            storeMe,
                            errMsg)) {
                        std::cout << "Failed to send data to dispatcher server" << std::endl;
                        return -1;
                    }
                }
                PDB_COUT << blockSize << "MB data sent to dispatcher server~~" << std::endl;
            }

            std::cout << "input set 3: total=" << total << std::endl;

            // to write back all buffered records
            pdbClient.flushData(errMsg);
        }
    }
    // now, create a new set in that database to store output data
    PDB_COUT << "to create a new set for storing output data" << std::endl;
    if (!pdbClient.createSet<String>("test76_db", "output_set1", errMsg, (size_t)64*(size_t)1024*(size_t)1024, "ToLoadOutputSetForTest76")) {
        cout << "Not able to create set: " + errMsg;
        exit(-1);
    } else {
        cout << "Created set.\n";
    }


    // this is the object allocation block where all of this stuff will reside
    const UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};
    // create all of the computation objects
    Handle<Computation> myScanSet1 = makeObject<ScanIntSet>("test76_db", "test76_set1");
    Handle<Computation> myScanSet2 = makeObject<ScanStringIntPairSet>("test76_db", "test76_set2");
    Handle<Computation> myScanSet3 = makeObject<ScanStringSet>("test76_db", "test76_set3");
    Handle<Computation> myJoin = makeObject<SillyJoin>();
    myJoin->setInput(0, myScanSet1);
    myJoin->setInput(1, myScanSet2);
    myJoin->setInput(2, myScanSet3);
    Handle<Computation> myCount = makeObject<Count>("test76_db", "output_set1");
    myCount->setInput(myJoin);
    auto begin = std::chrono::high_resolution_clock::now();

    if (!pdbClient.executeComputations(errMsg, myCount)) {
        std::cout << "Query failed. Message was: " << errMsg << "\n";
        return 1;

    }
    std::cout << std::endl;

    auto end = std::chrono::high_resolution_clock::now();
    // std::cout << "Time Duration: " <<
    //      std::chrono::duration_cast<std::chrono::nanoseconds>(end-begin).count() << " ns." <<
    //      std::endl;

    std::cout << std::endl;
    // print the resuts
    if (printResult == true) {
        std::cout << "to print result..." << std::endl;
        SetIterator<String> result = pdbClient.getSetIterator<String>("test76_db", "output_set1");

        std::cout << "Query results: ";
        int count = 0;
        for (auto a : result) {
            count++;
            std::cout << count << ":" << *a << ";";
        }
        std::cout << "join output count:" << count << "\n";
    }

    if (clusterMode == false) {
        // and delete the sets
        pdbClient.deleteSet("test76_db", "output_set1");
    } else {
        if (!pdbClient.removeSet("test76_db", "output_set1", errMsg)) {
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
