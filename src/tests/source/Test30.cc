
#ifndef TEST_30_CC
#define TEST_30_CC

#include "StorageClient.h"
#include "PDBVector.h"
#include "InterfaceFunctions.h"
#include "SharedEmployee.h"

// this won't be visible to the v-table map, since it is not in the built in types directory

int main(int argc, char* argv[]) {

    std::cout << "Firstly, make sure to run 'bin/test28' in a different window to provide a "
                 "catalog/storage server, with a forked backend server.\n";
    std::cout << "Secondly, make sure to run 'bin/test24' to create the source set.\n";
    std::cout << "Thirdly, make sure to run 'bin/test26' first to add data to the source set.\n";
    std::cout << "Secondly, make sure to run 'bin/test31'  to create the destination set.\n";
    std::cout << "Now, you can run this test case:\n";
    std::cout << std::endl;
    std::cout << std::endl;
    std::cout << "You can provide 4 arguments:" << std::endl;
    std::cout << "(1) the database name of the source set (MUST be created before running Test30);"
              << std::endl;
    std::cout << "(2) the name of the source set (MUST be created before running Test30);"
              << std::endl;
    std::cout
        << "(3) the database name of the destination set (MUST be created before running Test30);"
        << std::endl;
    std::cout << "(4) the name of the destination set (MUST be created before running Test30);"
              << std::endl;
    std::cout << std::endl;
    std::cout << std::endl;
    std::cout << "By default Test24_Database123 will be used for source database name, "
                 "Test24_Set123 will be used for source set name\n";
    std::cout << "By default Test31_Database123 will be used for source database name, "
                 "Test31_Set123 will be used for source set name\n";
    std::cout << std::endl;
    std::cout << std::endl;

    std::string srcDatabaseName("Test24_Database123");
    std::string srcSetName("Test24_Set123");
    std::string destDatabaseName("Test31_Database123");
    std::string destSetName("Test31_Set123");

    if (argc == 5) {
        srcDatabaseName = argv[1];
        srcSetName = argv[2];
        destDatabaseName = argv[3];
        destSetName = argv[4];
    }

    std::cout << "to copy data from set with  database name: " << srcDatabaseName
              << " and set name: " << srcSetName << std::endl;
    std::cout << "to set with database name: " << destDatabaseName
              << " and set name: " << destSetName << std::endl;
    std::cout << std::endl;
    std::cout << std::endl;

    // start a client
    bool usePangea = true;
    pdb::StorageClient temp(8108, "localhost", make_shared<pdb::PDBLogger>("clientLog"), usePangea);
    string errMsg;

    // now, copy data from the source set to the destination set
    temp.copyData<SharedEmployee>(
        srcDatabaseName, srcSetName, destDatabaseName, destSetName, errMsg);

    // now, remove data from the destination set
    std::cout << "to remove the destination set." << std::endl;
    temp.removeSet<SharedEmployee>(destDatabaseName, destSetName, errMsg);

    // now, add the destination set again
    std::cout << "to create a new destination set for next test." << std::endl;
    temp.createSet<SharedEmployee>(destDatabaseName, destSetName, errMsg);
}

#endif
