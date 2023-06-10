#ifndef TPCH_REGISTER_AND_CREATE_SETS_CC
#define TPCH_REGISTER_AND_CREATE_SETS_CC

#include "CatalogClient.h"

#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include <map>
#include <chrono>
#include <sstream>
#include <vector>
#include <ctime>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <chrono>
#include <fcntl.h>

#include "PDBDebug.h"
#include "PDBString.h"
#include "Query.h"
#include "Lambda.h"
#include "PDBClient.h"
#include "DataTypes.h"
#include "InterfaceFunctions.h"

#include "Part.h"
#include "Supplier.h"
#include "LineItem.h"
#include "Order.h"
#include "Customer.h"
#include "SumResultWriteSet.h"
#include "CustomerWriteSet.h"
#include "CustomerSupplierPartGroupBy.h"
#include "CustomerMultiSelection.h"
#include "ScanCustomerSet.h"
#include "SupplierData.h"
#include "CountAggregation.h"
#include "SumResult.h"

#include "Handle.h"
#include "Lambda.h"
#include "LambdaCreationFunctions.h"
#include "UseTemporaryAllocationBlock.h"
#include "Pipeline.h"
#include "SelectionComp.h"
#include "WriteBuiltinEmployeeSet.h"
#include "SupervisorMultiSelection.h"
#include "VectorSink.h"
#include "HashSink.h"
#include "MapTupleSetIterator.h"
#include "VectorTupleSetIterator.h"
#include "ComputePlan.h"

#include "QueryOutput.h"
#include "DataTypes.h"

#include <ctime>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <chrono>
#include <fcntl.h>
#include "WriteStringSet.h"

using namespace std;


#define KB 1024
#define MB (1024 * KB)
#define GB (1024 * MB)

#define BLOCKSIZE (256 * MB)

int main(int argc, char* argv[]) {

    // Connection info
    string masterHostname = "localhost";
    int masterPort = 8108;

    // register the shared employee class
    pdb::PDBLoggerPtr clientLogger = make_shared<pdb::PDBLogger>("clientLog");

    PDBClient pdbClient(
            masterPort, masterHostname,
            clientLogger,
            false,
            true);

    CatalogClient catalogClient(
            masterPort,
            masterHostname,
            clientLogger);

    string errMsg;

    cout << "Register Types Part, Supplier, LineItem, Order, Customer \n";
    if (!pdbClient.registerType("libraries/libPart.so", errMsg))
        cout << "Not able to register libPart type.\n";

    if (!pdbClient.registerType("libraries/libSupplier.so", errMsg))
        cout << "Not able to register libSupplier type.\n";

    if (!pdbClient.registerType("libraries/libLineItem.so", errMsg))
        cout << "Not able to register libLineItem type.\n";

    if (!pdbClient.registerType("libraries/libOrder.so", errMsg))
        cout << "Not able to register libOrder type.\n";

    if (!pdbClient.registerType("libraries/libCustomer.so", errMsg))
        cout << "Not able to register libCustomer type.\n";

    if (!pdbClient.registerType("libraries/libVirtualCustomer.so", errMsg))
        cout << "Not able to register libVirtualCustomer type.\n";

    cout << errMsg << endl;


    // now, create a new database
    if (!pdbClient.createDatabase("TPCH_db", errMsg)) {
        cout << "Not able to create database: " + errMsg;
        exit(-1);
    } else {
        cout << "Created TPCH_db database.\n";
    }

    // now, create the sets for storing Customer Data
    if (!pdbClient.createSet<Customer>(
            "TPCH_db", "tpch_bench_set1", errMsg)) {
        cout << "Not able to create set: " + errMsg;
        exit(-1);
    } else {
        cout << "Created tpch_bench_set1  set.\n";
    }


    cout << "Register further Types ... \n";


    if (!pdbClient.registerType("libraries/libSumResultWriteSet.so", errMsg))
        cout << "Not able to register type libSumResultWriteSet.\n";

    if (!pdbClient.registerType("libraries/libCustomerWriteSet.so", errMsg))
        cout << "Not able to register type libCustomerWriteSet.\n";

    if (!pdbClient.registerType("libraries/libScanCustomerSet.so", errMsg))
        cout << "Not able to register type libScanCustomerSet. \n";

    if (!pdbClient.registerType("libraries/libCustomerMultiSelection.so", errMsg))
        cout << "Not able to register type libCustomerMapSelection. \n";

    if (!pdbClient.registerType("libraries/libCustomerStringSelection.so", errMsg))
        cout << "Not able to register type libCustomerStringSelection. \n";

    if (!pdbClient.registerType("libraries/libCustomerStringSelectionVirtual.so", errMsg))
        cout << "Not able to register type libCustomerStringSelectionVirtual. \n";

    if (!pdbClient.registerType("libraries/libCustomerStringSelectionNot.so", errMsg))
        cout << "Not able to register type libCustomerStringSelectionNot. \n";

    if (!pdbClient.registerType("libraries/libCustomerStringSelectionVirtualNot.so", errMsg))
        cout << "Not able to register type libCustomerStringSelectionVirtualNot. \n";

    if (!pdbClient.registerType("libraries/libCustomerIntegerSelection.so", errMsg))
        cout << "Not able to register type libCustomerIntegerSelection. \n";

    if (!pdbClient.registerType("libraries/libCustomerIntegerSelectionVirtual.so", errMsg))
        cout << "Not able to register type libCustomerIntegerSelectionVirtual. \n";

    if (!pdbClient.registerType("libraries/libCustomerIntegerSelectionNot.so", errMsg))
        cout << "Not able to register type libCustomerIntegerSelectionNot. \n";

    if (!pdbClient.registerType("libraries/libCustomerIntegerSelectionVirtualNot.so", errMsg))
        cout << "Not able to register type libCustomerIntegerSelectionVirtualNot. \n";


    if (!pdbClient.registerType("libraries/libCustomerSupplierPartGroupBy.so", errMsg))
        cout << "Not able to register type libCustomerSupplierPartGroupBy.\n";

    if (!pdbClient.registerType("libraries/libSupplierInfo.so", errMsg))
        cout << "Not able to register type  libSupplierInfo\n";

    if (!pdbClient.registerType("libraries/libCustomerSupplierPartFlat.so", errMsg))
        cout << "Not able to register type  libCustomerSupplierPartFlat\n";

    if (!pdbClient.registerType("libraries/libCountAggregation.so", errMsg))
        cout << "Not able to register type  libCountAggregation\n";

    if (!pdbClient.registerType("libraries/libCountCustomer.so", errMsg))
        cout << "Not able to register type  libCountCustomer\n";

    if (!pdbClient.registerType("libraries/libSupplierInfoWriteSet.so", errMsg))
        cout << "Not able to register type libSupplierInfoWriteSet\n";

    cout << errMsg << endl;


    // Clean up the SO files.
    int code = system("scripts/cleanupSoFiles.sh");
    if (code < 0) {

        std::cout << "Can't cleanup so files" << std::endl;
    }
}

#endif
