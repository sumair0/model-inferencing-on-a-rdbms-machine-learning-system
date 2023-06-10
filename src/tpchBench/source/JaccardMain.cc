#ifndef TEST_LDA1_CC
#define TEST_LDA1_CC


// By Shangyu, June 2017
// LDA using Gibbs Sampling;

#include "PDBDebug.h"
#include "PDBVector.h"
#include "Query.h"
#include "Lambda.h"
#include "PDBClient.h"
#include "Set.h"
#include "DataTypes.h"
#include "TopKQueue.h"
#include "ScanCustomerSet.h"
#include "TopJaccard.h"
#include "JaccardResultWriter.h"

#include <ctime>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <chrono>
#include <fcntl.h>
#include <cstddef>
#include <iostream>
#include <fstream>
#include <math.h>
#include <random>
#include <sstream>

using namespace pdb;
int main(int argc, char* argv[]) {
    std::ofstream term("/dev/tty", std::ios_base::out);

    if (argc != 4) {
        std::cout << "Usage: #masterIp #k #queryFile\n";
        return (-1);
    }

    std::string masterIp;
    masterIp = argv[1];
    std::cout << "Master IP Address is " << masterIp << std::endl;

    int k = std::stoi(argv[2]);
    term << "The number of query results: " << k << std::endl;

    std::string queryFile;
    queryFile = argv[3];
    std::ifstream read(queryFile);
    int next;
    pdb::makeObjectAllocatorBlock(24 * 1024 * 1024, true);
    Handle<Vector<int>> myQuery = makeObject<Vector<int>>();
    while (read >> next) {
        myQuery->push_back(next);
    }

    pdb::PDBLoggerPtr clientLogger = make_shared<pdb::PDBLogger>("clientLog");
    pdb::PDBClient pdbClient(
            8108, masterIp, clientLogger, false, true);

    CatalogClient catalogClient(
            8108,
            masterIp,
            clientLogger);

    string errMsg;
    std::vector<std::string> v = {"libraries/libCustomer.so",
                                  "libraries/libLineItem.so",
                                  "libraries/libOrder.so",
                                  "libraries/libPart.so",
                                  "libraries/libSupplier.so",
                                  "libraries/libScanCustomerSet.so",
                                  "libraries/libTopJaccard.so",
                                  "libraries/libAllParts.so",
                                  "libraries/libJaccardResultWriter.so"};

    for (auto& a : v) {
        if (!pdbClient.registerType(a, errMsg)) {
            std::cout << "could not load library: " << errMsg << "\n";
        } else {
            std::cout << "loaded library: " << a << "\n";
        }
    }

    pdbClient.removeSet("TPCH_db", "result", errMsg);
    if (!pdbClient.createSet<TopKQueue<double, AllParts>>("TPCH_db", "result", errMsg)) {
        cout << "Not able to create set: " + errMsg;
        exit(-1);
    } else {
        cout << "Created set result.\n";
    }

    // Some meta data

    // Initialize the (wordID, topic prob vector)
    Handle<Computation> myScanSet = makeObject<ScanCustomerSet>("TPCH_db", "tpch_bench_set1");
    Handle<Computation> myTopK = makeObject<TopJaccard>(k, *myQuery);
    myTopK->setInput(myScanSet);
    Handle<Computation> myWriter = makeObject<JaccardResultWriter>("TPCH_db", "result");
    myWriter->setInput(myTopK);
    auto begin = std::chrono::high_resolution_clock::now();
    if (!pdbClient.executeComputations(errMsg, myWriter)) {
        std::cout << "Query failed. Message was: " << errMsg << "\n";
        return 1;
    }
    auto end = std::chrono::high_resolution_clock::now();

    std::cout << "The query is executed successfully!" << std::endl;
    float timeDifference =
        (float(std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count())) /
        (float)1000000000;
    std::cout << "#TimeDuration: " << timeDifference << " Second " << std::endl;

    // now iterate through the result
    SetIterator<TopKQueue<double, Handle<AllParts>>> result =
            pdbClient.getSetIterator<TopKQueue<double, Handle<AllParts>>>("TPCH_db", "result");
    std::map<int, int> resultMap;

    for (auto& a : result) {
        std::cout << "Got back " << a->size() << " items from the top-k query.\n";
        std::cout << "These items are:\n";

        for (int i = 0; i < a->size(); i++) {
            std::cout << "score: " << (*a)[i].getScore() << "\n";
            std::cout << "data: ";
            (*a)[i].getValue()->print();
            if (resultMap.count((*a)[i].getValue()->custKey) == 0) {
                resultMap[(*a)[i].getValue()->custKey] = 1;
            } else {
                resultMap[(*a)[i].getValue()->custKey] = resultMap[(*a)[i].getValue()->custKey] + 1;
            }
            std::cout << "\n\n";
        }
    }

    for (auto& a : resultMap) {
        std::cout << a.first << "  => " << a.second << '\n';
    }
}

#endif
