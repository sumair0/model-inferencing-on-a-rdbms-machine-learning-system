
#ifndef TEST_70_H
#define TEST_70_H


// by Jia, Mar 2017
// To dump a set


#include "PDBDebug.h"
#include "PDBString.h"
#include "Query.h"
#include "Lambda.h"
#include "DistributedStorageManagerClient.h"
#include "DataTypes.h"
#include <ctime>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <chrono>
#include <fcntl.h>

/* This test uses data and selection of builtInType to demonstrate a distributed query with
 * distributed storage */

/*  Note that data size must be larger than #numTotalThreadsInCluster*#PageSize */
/*  Below test case is tested using 8GB data in 4-node cluster, each node run 12 threads */
using namespace pdb;
int main(int argc, char* argv[]) {

    if (argc < 6) {
        std::cout << "Usage: #databaseName #setName #outputFilePath #masterIp #outputFormat"
                  << std::endl;
    }
    std::string databaseName = argv[1];
    std::string setName = argv[2];
    std::string outputFilePath = argv[3];
    std::string masterIp = argv[4];
    std::string outputFormat = argv[5];

    pdb::PDBLoggerPtr clientLogger = make_shared<pdb::PDBLogger>("clientLog");
    pdb::DistributedStorageManagerClient temp(8108, masterIp, clientLogger);

    std::string errMsg;
    bool ret = temp.exportSet(databaseName, setName, outputFilePath, outputFormat, errMsg);
    if (ret == false) {
        std::cout << errMsg << std::endl;
    } else {
        std::cout << "Set successfully exported to " << outputFilePath << std::endl;
    }

    int code = system("scripts/cleanupSoFiles.sh");
    if (code < 0) {
        std::cout << "Can't cleanup so files" << std::endl;
    }
}

#endif
