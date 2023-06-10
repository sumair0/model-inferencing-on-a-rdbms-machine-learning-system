#ifndef TEST_LA_04_CC
#define TEST_LA_04_CC


// by Binhang, May 2017
// to test matrix multiply implemented by join;

#include "PDBDebug.h"
#include "PDBString.h"
#include "Query.h"
#include "Lambda.h"
#include "PDBClient.h"
#include "LAScanMatrixBlockSet.h"
#include "LAWriteMatrixBlockSet.h"
#include "LASillyMultiply1Join.h"
#include "LASillyMultiply2Aggregate.h"
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
void loadMatrix (PDBClient & pdbClient, String dbName, String setName, int blockSize, int rowNum, int colNum, int blockRowNums, int blockColNums, std::string errMsg) {

     int total = 0;
     int i=0; 
     int j=0;
     while (true) {

          pdb::makeObjectAllocatorBlock(blockSize * 1024 * 1024, true);

          pdb::Handle<pdb::Vector<pdb::Handle<MatrixBlock>>> storeMatrix1 =
              pdb::makeObject<pdb::Vector<pdb::Handle<MatrixBlock>>>();

          try {
              for (i=i; i < rowNum; i++) {
                  for (j=j; j < colNum; j++) {
                      pdb::Handle<MatrixBlock> myData =
                         pdb::makeObject<MatrixBlock>(i, j, blockRowNums, blockColNums);
                      // Foo initialization
                      for (int ii = 0; ii < blockRowNums; ii++) {
                          for (int jj = 0; jj < blockColNums; jj++) {
                              (*(myData->getRawDataHandle()))[ii * blockColNums + jj] = i + j + ii + jj;
                          }
                      }

                      std::cout << "New block: " << total << std::endl;
                      storeMatrix1->push_back(myData);
                      total++;
                 }
              }
              if (!pdbClient.sendData<MatrixBlock>(
                  std::pair<std::string, std::string>(setName, dbName),
                  storeMatrix1,
                  errMsg)) {
                      std::cout << "Failed to send data to dispatcher server" << std::endl;
                      exit(1);
              }
              break;
          } catch (NotEnoughSpace& e) {
              if (!pdbClient.sendData<MatrixBlock>(
                  std::pair<std::string, std::string>(setName, dbName),
                  storeMatrix1,
                  errMsg)) {
                      std::cout << "Failed to send data to dispatcher server" << std::endl;
                      exit(1);
              } 
          }
          PDB_COUT << total << " MatrixBlock data sent to dispatcher server~~" << std::endl;
    }
   // to write back all buffered records
    pdbClient.flushData(errMsg);
}



int main(int argc, char* argv[]) {
    bool printResult = false;
    bool clusterMode = true;
    bool createData = false;
    bool runQuery = true;
    bool removeData = false;
    std::cout
        << "Usage: #createData[Y/N] #runQuery[Y/N] #removeData[Y/N] BlockSize "
        << std::endl;
    if (argc > 1) {
        if (strcmp(argv[1], "Y") == 0) {
            createData = true;
            std::cout << "You successfully enabled create data" << std::endl;
        } else {
            createData = false;
            std::cout << "disabled create data" << std::endl;
        }
    } else {
        std::cout << "createData is set to false"
                  << std::endl;
    }

    if (argc > 2) {
        if (strcmp(argv[2], "N") == 0) {
            runQuery =false;
            std::cout << "You successfully disabled run query" << std::endl;
        } else {
           runQuery  =true;
        }
    } else {
        std::cout << "run query is set to true"
                  << std::endl;
    }
    if(argc >3){
	if(strcmp(argv[3],"N") == 0) {
	removeData =false;
            std::cout << "You successfully disabled remove data" << std::endl;
        } else {
           removeData  =true;
        }
    } else {
        std::cout << "remove data is set to false"
                  << std::endl;
    }             


    int blockSize = 64;  // by default we Substract 64MB data
    if (argc > 4) {
        blockSize = atoi(argv[4]);
    }


    std::cout << "To Substract data with size: " << blockSize << "MB" << std::endl;

    String masterIp = "localhost";
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

        // Step 1. Create Database and Set
        // now, register a type for user data
        // TODO: once sharedLibrary is supported, Substract this line back!!!
        pdbClient.registerType("libraries/libMatrixMeta.so", errMsg);
        pdbClient.registerType("libraries/libMatrixData.so", errMsg);
        pdbClient.registerType("libraries/libMatrixBlock.so", errMsg);
	if(createData == true){
          // now, create a new database
          if (!pdbClient.createDatabase("LA04_db", errMsg)) {
              cout << "Not able to create database: " + errMsg;
              exit(-1);
          } else {
              cout << "Created database.\n";
          }
          // now, create the first matrix set in that database
          if (!pdbClient.createSet<MatrixBlock>("LA04_db", "LA_input_set1", errMsg, (size_t)64*(size_t)1024*(size_t)1024, "loadLeftMatrixForMultiply")) {
              cout << "Not able to create set: " + errMsg;
              exit(-1);
          } else {
              cout << "Created set.\n";
          }

          // now, create the first matrix set in that database
          if (!pdbClient.createSet<MatrixBlock>("LA04_db", "LA_input_set2", errMsg, (size_t)64*(size_t)1024*(size_t)1024, "loadRightMatrixForMultiply")) {
              cout << "Not able to create set: " + errMsg;
              exit(-1);
          } else {
              cout << "Created set.\n";
          }


           // Step 2. Substract data
          int matrixRowNums = 1;
          int matrixColNums = 5000;
          int matrixRColNums = 1;
          int blockRowNums = 1000;
          int blockColNums = 1000;


          loadMatrix(pdbClient, "LA04_db", "LA_input_set1", blockSize, matrixRowNums, matrixColNums, 1000, 1000, errMsg);
          loadMatrix(pdbClient, "LA04_db", "LA_input_set2", blockSize, matrixColNums, matrixRowNums, 1000, 1000, errMsg); 

       }
    // now, create a new set in that database to store output data

    if (runQuery) {

    PDB_COUT << "to create a new set for storing output data" << std::endl;
    if (!pdbClient.createSet<MatrixBlock>("LA04_db", "LA_product_set", errMsg)) {
        cout << "Not able to create set: " + errMsg;
        exit(-1);
    } else {
        cout << "Created set.\n";
    }

    // Step 3. To execute a Query
    // for allocations
    const UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};

    // register this query class
    pdbClient.registerType("libraries/libLASillyMultiply1Join.so", errMsg);
    pdbClient.registerType("libraries/libLASillyMultiply2Aggregate.so", errMsg);
    pdbClient.registerType("libraries/libLAScanMatrixBlockSet.so", errMsg);
    pdbClient.registerType("libraries/libLAWriteMatrixBlockSet.so", errMsg);



    Handle<Computation> myMatrixSet1 = makeObject<LAScanMatrixBlockSet>("LA04_db", "LA_input_set1");
    Handle<Computation> myMatrixSet2 = makeObject<LAScanMatrixBlockSet>("LA04_db", "LA_input_set2");

    Handle<Computation> myMultiply1Join = makeObject<LASillyMultiply1Join>();
    myMultiply1Join->setInput(0, myMatrixSet1);
    myMultiply1Join->setInput(1, myMatrixSet2);

    Handle<Computation> myMultiply2Aggregate = makeObject<LASillyMultiply2Aggregate>();
    myMultiply2Aggregate->setInput(myMultiply1Join);

    Handle<Computation> myProductWriteSet =
        makeObject<LAWriteMatrixBlockSet>("LA04_db", "LA_product_set");
    myProductWriteSet->setInput(myMultiply2Aggregate);

    auto begin = std::chrono::high_resolution_clock::now();

    if (!pdbClient.executeComputations(errMsg, myProductWriteSet)) {
        std::cout << "Query failed. Message was: " << errMsg << "\n";
        return 1;
    }
    std::cout << std::endl;

    auto end = std::chrono::high_resolution_clock::now();


    std::cout << "Time Duration: "
              << std::chrono::duration_cast<std::chrono::duration<float>>(end - begin).count()
              << " secs." << std::endl;

    std::cout << std::endl;
    // print the resuts
    if (printResult == true) {
        std::cout << "to print result..." << std::endl;

        SetIterator<MatrixBlock> input1 =
            pdbClient.getSetIterator<MatrixBlock>("LA04_db", "LA_input_set1");
        std::cout << "Input Matrix 1:" << std::endl;
        int countIn1 = 0;
        for (auto a : input1) {
            countIn1++;
            std::cout << countIn1 << ":";
            a->print();
            std::cout << std::endl;
        }
        std::cout << "Matrix1 input block nums:" << countIn1 << "\n";

        SetIterator<MatrixBlock> input2 =
            pdbClient.getSetIterator<MatrixBlock>("LA04_db", "LA_input_set2");
        std::cout << "Input Matrix 2:" << std::endl;
        int countIn2 = 0;
        for (auto a : input2) {
            countIn2++;
            std::cout << countIn2 << ":";
            a->print();
            std::cout << std::endl;
        }
        std::cout << "Matrix2 input block nums:" << countIn2 << "\n";


        SetIterator<MatrixBlock> result =
            pdbClient.getSetIterator<MatrixBlock>("LA04_db", "LA_product_set");
        std::cout << "Multiply query results: " << std::endl;
        int countOut = 0;
        for (auto a : result) {
            countOut++;
            std::cout << countOut << ":";
            a->print();

            std::cout << std::endl;
        }
        std::cout << "Product output count:" << countOut << "\n";
    }
       if (!pdbClient.removeSet("LA04_db", "LA_product_set", errMsg)) {
            cout << "Not able to remove output set: " + errMsg;
            exit(-1);
        } else { 
            cout << "Removed output set.\n";
        }

}

if(removeData == true){

        if (!pdbClient.removeSet("LA04_db", "LA_input_set1", errMsg)) {
            cout << "Not able to remove set 1: " + errMsg;
            exit(-1);
        } else {
            cout << "Removed set 1.\n";
        }

       if (!pdbClient.removeSet("LA04_db", "LA_input_set2", errMsg)) {
            cout << "Not able to remove set 2: " + errMsg;
            exit(-1);
        } else { 
            cout << "Removed set 2.\n";
        }





}
    int code = system("scripts/cleanupSoFiles.sh");
    if (code < 0) {
        std::cout << "Can't cleanup so files" << std::endl;
    }
}

#endif
