#ifndef TEST_LA_08_CC
#define TEST_LA_08_CC


// by Binhang, May 2017
// to test matrix rowMax implemented by aggregation;

#include "PDBDebug.h"
#include "PDBString.h"
#include "Query.h"
#include "Lambda.h"
#include "PDBClient.h"
#include "LAScanMatrixBlockSet.h"
#include "LAWriteMatrixBlockSet.h"
#include "LASillyRowMaxAggregate.h"
//#include "BuiltInMatrixBlock.h"
#include "MatrixBlock.h"
#include "Set.h"
#include "DataTypes.h"
#include <ctime>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <chrono>
#include <fcntl.h>


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
        }
    } else {
        std::cout << "Will run on local node. If you want to run on cluster, you can add any "
                     "character as the second parameter to run on the cluster configured by "
                     "$PDB_HOME/conf/serverlist."
                  << std::endl;
    }

    int numOfMb = 64;  // by default we add 64MB data
    if (argc > 3) {
        numOfMb = atoi(argv[3]);
    }
    numOfMb = 64;  // Force it to be 64 by now.


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

    pdb::PDBLoggerPtr clientLogger = make_shared<pdb::PDBLogger>("clientLog");

    PDBClient pdbClient(
            8108, masterIp,
            clientLogger,
            false,
            true);

    CatalogClient catalogClient(
            8108,
            masterIp,
            clientLogger);

    string errMsg;

    if (whetherToAddData == true) {
        // Step 1. Create Database and Set
        // now, register a type for user data
        // TODO: once sharedLibrary is supported, add this line back!!!
        pdbClient.registerType("libraries/libMatrixMeta.so", errMsg);
        pdbClient.registerType("libraries/libMatrixData.so", errMsg);
        pdbClient.registerType("libraries/libMatrixBlock.so", errMsg);

        // now, create a new database
        if (!pdbClient.createDatabase("LA08_db", errMsg)) {
            cout << "Not able to create database: " + errMsg;
            exit(-1);
        } else {
            cout << "Created database.\n";
        }

        // now, create a new set in that database
        if (!pdbClient.createSet<MatrixBlock>("LA08_db", "LA_input_set", errMsg)) {
            cout << "Not able to create set: " + errMsg;
            exit(-1);
        } else {
            cout << "Created set.\n";
        }


        // Step 2. Add data

        int total = 0;
        if (numOfMb > 0) {
            int numIterations = numOfMb / 64;
            std::cout << "Number of MB: " << numOfMb << "Number of Iterations: " << numIterations
                      << std::endl;
            int remainder = numOfMb - 64 * numIterations;
            if (remainder > 0) {
                numIterations = numIterations + 1;
            }
            for (int num = 0; num < numIterations; num++) {
                std::cout << "Iterations: " << num << std::endl;
                int blockSize = 64;
                if ((num == numIterations - 1) && (remainder > 0)) {
                    blockSize = remainder;
                }
                pdb::makeObjectAllocatorBlock(blockSize * 1024 * 1024, true);
                pdb::Handle<pdb::Vector<pdb::Handle<MatrixBlock>>> storeMe =
                    pdb::makeObject<pdb::Vector<pdb::Handle<MatrixBlock>>>();
                try {
                    // Write 100 Matrix of size 50 * 50
                    int matrixRowNums = 4;
                    int matrixColNums = 4;
                    int blockRowNums = 10;
                    int blockColNums = 5;
                    for (int i = 0; i < matrixRowNums; i++) {
                        for (int j = 0; j < matrixColNums; j++) {
                            pdb::Handle<MatrixBlock> myData =
                                pdb::makeObject<MatrixBlock>(i, j, blockRowNums, blockColNums);
                            // Foo initialization
                            for (int ii = 0; ii < blockRowNums; ii++) {
                                for (int jj = 0; jj < blockColNums; jj++) {
                                    (*(myData->getRawDataHandle()))[ii * blockColNums + jj] =
                                        i + j + ii + jj + 0.0;
                                }
                            }
                            std::cout << "New block: " << total << std::endl;
                            myData->print();
                            storeMe->push_back(myData);
                            total++;
                        }
                    }
                    for (int i = 0; i < storeMe->size(); i++) {
                        (*storeMe)[i]->print();
                    }
                    if (!pdbClient.sendData<MatrixBlock>(
                            std::pair<std::string, std::string>("LA_input_set", "LA08_db"),
                            storeMe,
                            errMsg)) {
                        std::cout << "Failed to send data to dispatcher server" << std::endl;
                        return -1;
                    }
                } catch (pdb::NotEnoughSpace& n) {
                    if (!pdbClient.sendData<MatrixBlock>(
                            std::pair<std::string, std::string>("LA_input_set", "LA08_db"),
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
    if (!pdbClient.createSet<MatrixBlock>("LA08_db", "LA_rowMax_set", errMsg)) {
        cout << "Not able to create set: " + errMsg;
        exit(-1);
    } else {
        cout << "Created set.\n";
    }

    // Step 3. To execute a Query
    // for allocations
    const UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};

    // register this query class
    pdbClient.registerType("libraries/libLASillyRowMaxAggregate.so", errMsg);
    pdbClient.registerType("libraries/libLAScanMatrixBlockSet.so", errMsg);
    pdbClient.registerType("libraries/libLAWriteMatrixBlockSet.so", errMsg);



    Handle<Computation> myScanSet = makeObject<LAScanMatrixBlockSet>("LA08_db", "LA_input_set");
    Handle<Computation> myQuery = makeObject<LASillyRowMaxAggregate>();
    myQuery->setInput(myScanSet);
    // myQuery->setOutput("LA08_db", "LA_rowMax_set");

    Handle<Computation> myWriteSet = makeObject<LAWriteMatrixBlockSet>("LA08_db", "LA_rowMax_set");
    myWriteSet->setInput(myQuery);

    auto begin = std::chrono::high_resolution_clock::now();

    if (!pdbClient.executeComputations(errMsg, myWriteSet)) {
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
        SetIterator<MatrixBlock> input =
            pdbClient.getSetIterator<MatrixBlock>("LA08_db", "LA_input_set");
        std::cout << "Query input: " << std::endl;
        int countIn = 0;
        for (auto a : input) {
            countIn++;
            std::cout << countIn << ":";
            a->print();
            std::cout << std::endl;
        }
        std::cout << "Matrix input block nums:" << countIn << "\n";


        SetIterator<MatrixBlock> result =
            pdbClient.getSetIterator<MatrixBlock>("LA08_db", "LA_rowMax_set");
        std::cout << "RowMax query results: " << std::endl;
        int countOut = 0;
        for (auto a : result) {
            countOut++;
            std::cout << countOut << ":";
            a->print();

            std::cout << std::endl;
        }
        std::cout << "RowMax output count:" << countOut << "\n";
    }

    if (clusterMode == false) {
        // and delete the sets
        pdbClient.deleteSet("LA08_db", "LA_rowMax_set");
    } else {
        if (!pdbClient.removeSet("LA08_db", "LA_rowMax_set", errMsg)) {
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
