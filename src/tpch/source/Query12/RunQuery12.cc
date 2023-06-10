#ifndef RUN_QUERY12_CC
#define RUN_QUERY12_CC

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
#include "Handle.h"
#include "LambdaCreationFunctions.h"
#include "UseTemporaryAllocationBlock.h"
#include "Pipeline.h"
#include "VectorSink.h"
#include "HashSink.h"
#include "MapTupleSetIterator.h"
#include "VectorTupleSetIterator.h"
#include "ComputePlan.h"
#include "QueryOutput.h"
#include "ScanUserSet.h"
#include "WriteUserSet.h"

#include "TPCHSchema.h"
#include "Query12.h"

#include <ctime>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <chrono>
#include <fcntl.h>


using namespace std;
using namespace tpch;

/*
select
l_shipmode,
sum(case
when o_orderpriority ='1-URGENT'
or o_orderpriority ='2-HIGH' then 1
else 0
end) as high_line_count,
sum(case
when o_orderpriority <> '1-URGENT'
and o_orderpriority <> '2-HIGH' then 1
else 0
end) as low_line_count
from
orders,
lineitem where
o_orderkey = l_orderkey
and l_shipmode in ('[SHIPMODE1]', '[SHIPMODE2]') and l_commitdate < l_receiptdate
and l_shipdate < l_commitdate
and l_receiptdate >= date '[DATE]'
and l_receiptdate < date '[DATE]' + interval '1' year
group by l_shipmode
order by l_shipmode;

*/


int main(int argc, char* argv[]) {

    bool whetherToRegisterLibraries = false;
    if (argc > 1) {
        if (strcmp(argv[1], "Y") == 0) {
            whetherToRegisterLibraries = true;
        }
    }


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

    if (whetherToRegisterLibraries == true) {
        std::string errMsg;
        pdbClient.registerType ("libraries/libQ12LineItemSelection.so", errMsg);
        pdbClient.registerType ("libraries/libQ12JoinOut.so", errMsg);
        pdbClient.registerType ("libraries/libQ12Join.so", errMsg);
        pdbClient.registerType ("libraries/libQ12ValueClass.so", errMsg);
        pdbClient.registerType ("libraries/libQ12AggOut.so", errMsg);
        pdbClient.registerType ("libraries/libQ12Agg.so", errMsg);
    }    


    // now, create the sets for storing Customer Data
    std::string errMsg;
    pdbClient.removeSet("tpch", "q12_output_set", errMsg);
    if (!pdbClient.createSet<Q12AggOut>(
            "tpch", "q12_output_set", errMsg)) {
        cout << errMsg << endl;
    } else {
        cout << "Created set.\n";
    }

    // for allocations
    const UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 256};

    // make the query graph
    Handle<Computation> myOrderScanner = makeObject<ScanUserSet<Order>>("tpch", "order");
    Handle<Computation> myLineItemScanner = makeObject<ScanUserSet<LineItem>>("tpch", "lineitem");
    Handle<Computation> myQ12LineItemSelection = makeObject<Q12LineItemSelection>();
    Handle<Computation> myQ12Join = makeObject<Q12Join>();
    Handle<Computation> myQ12Agg = makeObject<Q12Agg>();
    Handle<Computation> myQ12Writer = makeObject<WriteUserSet<Q12AggOut>> ("tpch", "q12_output_set");
    myQ12LineItemSelection->setInput(myLineItemScanner);
    myQ12Join->setInput(0, myOrderScanner);
    myQ12Join->setInput(1, myQ12LineItemSelection);
    myQ12Agg->setInput(myQ12Join);
    myQ12Writer->setInput(myQ12Agg);


    // Query Execution and Time Calculation

    auto begin = std::chrono::high_resolution_clock::now();

    if (!pdbClient.queryClient.executeComputations(errMsg, "TPCHQuery12", myQ12Writer)) {
        std::cout << "Query failed. Message was: " << errMsg << "\n";
        return 1;
    }

    std::cout << std::endl;
    auto end = std::chrono::high_resolution_clock::now();

    float timeDifference =
        (float(std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count())) /
        (float)1000000000;



    // Printing results to double check
    std::cout << "to print result..." << std::endl;


    SetIterator<Q12AggOut> result =
            pdbClient.getSetIterator<Q12AggOut>("tpch", "q12_output_set");

    std::cout << "Query results: ";
    int count = 0;
    for (auto a : result) {
        std::cout << a->l_shipmode << ":" << a->value.high_line_count << "," << a->value.low_line_count << std::endl;
        count++;
    }
    std::cout << "Output count:" << count << "\n";
    std::cout << "#TimeDuration for query execution: " << timeDifference << " Second " << std::endl;

    // Remove the output set
    if (!pdbClient.removeSet("tpch", "q12_output_set", errMsg)) {
        cout << "Not able to remove the set: " + errMsg;
        exit(-1);
    } else {
        cout << "Set removed. \n";
    }

    // Clean up the SO files.
    int code = system("scripts/cleanupSoFiles.sh");
    if (code < 0) {

        std::cout << "Can't cleanup so files" << std::endl;
    }

}
#endif
