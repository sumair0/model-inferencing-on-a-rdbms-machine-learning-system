#ifndef TPCH_DATA_GENERATOR_CC
#define TPCH_DATA_GENERATOR_CC

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

// Run a Cluster on Localhost
// ./bin/pdb-cluster localhost 8108 Y
// ./bin/pdb-server 4 4000 localhost:8108 localhost:8109
// ./bin/pdb-server 4 4000 localhost:8108 localhost:8110

// TPCH data set is available here https://drive.google.com/file/d/0BxMSXpJqaNfNMzV1b1dUTzVqc28/view
// Just unzip the file and put the folder in main directory of PDB

#define KB 1024
#define MB (1024 * KB)
#define GB (1024 * MB)

#define BLOCKSIZE (256 * MB)


// A function to parse a Line
std::vector<std::string> parseLine(std::string line) {
    stringstream lineStream(line);
    std::vector<std::string> tokens;
    string token;
    while (getline(lineStream, token, '|')) {
        tokens.push_back(token);
    }
    return tokens;
}


void dataGenerator(std::string scaleFactor,
                   pdb::DispatcherClient dispatcherClient,
                   int noOfCopies) {

    // All files to parse:
    string PartFile = "tables_scale_" + scaleFactor + "/part.tbl";
    string supplierFile = "tables_scale_" + scaleFactor + "/supplier.tbl";
    string orderFile = "tables_scale_" + scaleFactor + "/orders.tbl";
    string lineitemFile = "tables_scale_" + scaleFactor + "/lineitem.tbl";
    string customerFile = "tables_scale_" + scaleFactor + "/customer.tbl";

    // Common constructs:
    string line;
    string delimiter = "|";
    ifstream infile;

    // ####################################
    // ####################################
    // ##########                 #########
    // ##########      Part       #########
    // ##########                 #########
    // ####################################
    // ####################################

    infile.open(PartFile.c_str());

    vector<pdb::Handle<Part>> partList;
    map<int, pdb::Handle<Part>> partMap;

    while (getline(infile, line)) {

        std::vector<string> tokens = parseLine(line);

        int partID = atoi(tokens.at(0).c_str());

        if (partID % 10000 == 0)
            cout << "Part ID: " << partID << endl;

        pdb::Handle<Part> part = pdb::makeObject<Part>(atoi(tokens.at(0).c_str()),
                                                       tokens.at(1),
                                                       tokens.at(2),
                                                       tokens.at(3),
                                                       tokens.at(4),
                                                       atoi(tokens.at(5).c_str()),
                                                       tokens.at(6),
                                                       atof(tokens.at(7).c_str()),
                                                       tokens.at(8));

        partList.push_back(part);

        // Populate the hash:
        partMap[atoi(tokens.at(0).c_str())] = part;
    }

    infile.close();
    infile.clear();

    // ####################################
    // ####################################
    // ##########                 #########
    // ##########     Supplier    #########
    // ##########                 #########
    // ####################################
    // ####################################

    infile.open(supplierFile.c_str());
    vector<pdb::Handle<Supplier>> supplierList;
    map<int, pdb::Handle<Supplier>> supplierMap;

    while (getline(infile, line)) {
        std::vector<string> tokens = parseLine(line);


        int supplierKey = atoi(tokens.at(0).c_str());

        pdb::Handle<Supplier> tSupplier = pdb::makeObject<Supplier>(supplierKey,
                                                                    tokens.at(1),
                                                                    tokens.at(2),
                                                                    atoi(tokens.at(3).c_str()),
                                                                    tokens.at(4),
                                                                    atof(tokens.at(5).c_str()),
                                                                    tokens.at(6));
        supplierList.push_back(tSupplier);

        if (supplierKey % 1000 == 0)
            cout << "Supplier ID: " << supplierKey << endl;

        // Populate the hash:
        supplierMap[atoi(tokens.at(0).c_str())] = tSupplier;
    }

    infile.close();
    infile.clear();

    // ####################################
    // ####################################
    // ##########                 #########
    // ##########     LineItem    #########
    // ##########                 #########
    // ####################################
    // ####################################

    // Open "LineitemFile": Iteratively (Read line, Parse line, Create Objects):
    infile.open(lineitemFile.c_str());

    pdb::Handle<pdb::Vector<LineItem>> lineItemList = pdb::makeObject<pdb::Vector<LineItem>>();

    map<int, pdb::Handle<pdb::Vector<LineItem>>> lineItemMap;

    while (getline(infile, line)) {

        std::vector<string> tokens = parseLine(line);

        int orderKey = atoi(tokens.at(0).c_str());
        int partKey = atoi(tokens.at(1).c_str());
        int supplierKey = atoi(tokens.at(2).c_str());

        Part tPart;
        Supplier tSupplier;

        // Find the appropriate "Part"
        if (partMap.find(partKey) != partMap.end()) {
            tPart = *partMap[partKey];
        } else {
            throw invalid_argument("There is no such Part.");
        }

        // Find the appropriate "Part"
        if (supplierMap.find(supplierKey) != supplierMap.end()) {
            tSupplier = *supplierMap[supplierKey];
        } else {
            throw invalid_argument("There is no such Supplier.");
        }

        pdb::Handle<LineItem> tLineItem = pdb::makeObject<LineItem>(tokens.at(0),
                                                                    orderKey,
                                                                    tSupplier,
                                                                    tPart,
                                                                    atoi(tokens.at(3).c_str()),
                                                                    atof(tokens.at(4).c_str()),
                                                                    atof(tokens.at(5).c_str()),
                                                                    atof(tokens.at(6).c_str()),
                                                                    atof(tokens.at(7).c_str()),
                                                                    tokens.at(8),
                                                                    tokens.at(9),
                                                                    tokens.at(10),
                                                                    tokens.at(11),
                                                                    tokens.at(12),
                                                                    tokens.at(13),
                                                                    tokens.at(14),
                                                                    tokens.at(15));

        if (orderKey % 100000 == 0)
            cout << "LineItem ID: " << orderKey << endl;

        // Populate the hash:
        if (lineItemMap.find(orderKey) != lineItemMap.end()) {
            // the key already exists in the map
            lineItemMap[orderKey]->push_back(*tLineItem);
        } else {
            // make a new vector
            pdb::Handle<pdb::Vector<LineItem>> lineItemList =
                pdb::makeObject<pdb::Vector<LineItem>>();

            // push in the vector
            lineItemList->push_back(*tLineItem);
            // put in the map
            lineItemMap[orderKey] = lineItemList;
        }
    }

    infile.close();
    infile.clear();

    // ####################################
    // ####################################
    // ########## 				  #########
    // ##########       Order     #########
    // ##########                 #########
    // ####################################
    // ####################################

    // Open "OrderFile": Iteratively (Read line, Parse line, Create Objects):
    infile.open(orderFile.c_str());

    map<int, pdb::Vector<Order>> orderMap;

    while (getline(infile, line)) {
        std::vector<string> tokens = parseLine(line);

        int orderKey = atoi(tokens.at(0).c_str());
        int customerKey = atoi(tokens.at(1).c_str());


        pdb::Handle<Order> tOrder = pdb::makeObject<Order>(*lineItemMap[orderKey],
                                                           orderKey,
                                                           customerKey,
                                                           tokens.at(2),
                                                           atof(tokens.at(3).c_str()),
                                                           tokens.at(4),
                                                           tokens.at(5),
                                                           tokens.at(6),
                                                           atoi(tokens.at(7).c_str()),
                                                           tokens.at(8));

        if (orderKey % 100000 == 0)
            cout << "Order ID: " << orderKey << endl;

        // Populate the hash:
        if (orderMap.find(customerKey) != orderMap.end()) {
            orderMap[customerKey].push_back(*tOrder);
        } else {
            pdb::Handle<pdb::Vector<Order>> orderList = pdb::makeObject<pdb::Vector<Order>>();
            orderList->push_back(*tOrder);
            orderMap[customerKey] = *orderList;
        }
    }

    infile.close();
    infile.clear();

    // ####################################
    // ####################################
    // ##########                 #########
    // ##########    Customers    #########
    // ##########                 #########
    // ####################################
    // ####################################

    cout << "Started Creating Customers ..." << endl;


    size_t sendingObjectSize = 0;
    string errMsg;

    // make a new Allocation Block
    pdb::makeObjectAllocatorBlock((size_t)BLOCKSIZE, true);
    pdb::Handle<pdb::Vector<pdb::Handle<Customer>>> storeMeCustomerList =
        pdb::makeObject<pdb::Vector<pdb::Handle<Customer>>>();

    // Copy the same data multiple times to make it bigger.
    for (int i = 0; i < noOfCopies; ++i) {


        // Open "CustomerFile": Iteratively (Read line, Parse line, Create Objects):
        infile.open(customerFile.c_str());
        //		cout << "Storing copy number " << i << endl;
        pdb::Handle<Customer> objectToAdd = nullptr;

        while (getline(infile, line)) {
            std::vector<string> tokens = parseLine(line);
            int customerKey = atoi(tokens.at(0).c_str());

            try {

                objectToAdd = pdb::makeObject<Customer>(orderMap[customerKey],
                                                        customerKey,
                                                        tokens.at(1),
                                                        tokens.at(2),
                                                        atoi(tokens.at(3).c_str()),
                                                        tokens.at(4),
                                                        atof(tokens.at(5).c_str()),
                                                        tokens.at(6),
                                                        tokens.at(7));
                storeMeCustomerList->push_back(objectToAdd);

            } catch (NotEnoughSpace& e) {

                //				std::cout << "Got into Exception part. " << std::endl;

                // First send the existing data over
                if (storeMeCustomerList->size() > 0) {
                    if (!dispatcherClient.sendData<Customer>(
                            std::pair<std::string, std::string>("tpch_bench_set1", "TPCH_db"),
                            storeMeCustomerList,
                            errMsg)) {
                        std::cout << "Failed to send data to dispatcher server" << std::endl;
                    }
                    sendingObjectSize += storeMeCustomerList->size();

                    std::cout << "Copy Number: " << i
                              << "  Sending data! Count: " << sendingObjectSize << std::endl;
                } else {
                    std::cout << "Vector is zero." << sendingObjectSize << std::endl;
                }

                // make a allocation Block and a new vector.
                // pdb::makeObjectAllocatorBlock((size_t) BLOCKSIZE, true);
                storeMeCustomerList->clear();

                // retry to make the object and add it to the vector
                try {
                    objectToAdd = pdb::makeObject<Customer>(orderMap[customerKey],
                                                            customerKey,
                                                            tokens.at(1),
                                                            tokens.at(2),
                                                            atoi(tokens.at(3).c_str()),
                                                            tokens.at(4),
                                                            atof(tokens.at(5).c_str()),
                                                            tokens.at(6),
                                                            tokens.at(7));
                    storeMeCustomerList->push_back(objectToAdd);
                } catch (NotEnoughSpace& e) {
                    std::cerr << "This should not happen that we have not enough space after new "
                                 "allocation. Object size is too large. "
                              << std::endl;
                    return;
                }
            }
        }

        // send the rest of data at the end, it can happen that the exception never happens.
        if (!dispatcherClient.sendData<Customer>(
                std::pair<std::string, std::string>("tpch_bench_set1", "TPCH_db"),
                storeMeCustomerList,
                errMsg)) {
            std::cout << "Failed to send data to dispatcher server" << std::endl;
        }
        sendingObjectSize += storeMeCustomerList->size();

        std::cout << "Send the rest of the data at the end: " << sendingObjectSize << std::endl;


        // make a allocation Block and a new vector.
        //		pdb::makeObjectAllocatorBlock((size_t) BLOCKSIZE, true);
        //		storeMeCustomerList = pdb::makeObject<pdb::Vector<pdb::Handle<Customer>>>();

        storeMeCustomerList->clear();

        infile.close();
        infile.clear();
    }
}

// pdb::Handle<pdb::Vector<pdb::Handle<Customer>>>generateSmallDataset(int maxNoOfCustomers) {
//
//	int maxPartsInEachLineItem = 4;
//	int maxLineItemsInEachOrder = 4;
//	int maxOrderssInEachCostomer = 4;
//
//	pdb::Handle<pdb::Vector<pdb::Handle<Customer>>> customers =
//pdb::makeObject<pdb::Vector<pdb::Handle<Customer>>>();
//
//	//4. Make Customers
//	for (int customerID = 0; customerID < maxNoOfCustomers; ++customerID) {
//		pdb::Handle<pdb::Vector<Order>> orders = pdb::makeObject<pdb::Vector<Order>> ();
//		//3. Make Order
//		for (int orderID = 0; orderID < maxOrderssInEachCostomer; ++orderID) {
//			pdb::Handle<pdb::Vector<LineItem>> lineItems = pdb::makeObject<pdb::Vector<LineItem>>
//();
//			//2.  Make LineItems
//			for (int i = 0; i < maxLineItemsInEachOrder; ++i) {
//				pdb::Handle<Part> part = pdb::makeObject<Part>(i, "Part-" + to_string(i), "mfgr",
//"Brand1", "type1", i, "Container1", 12.1, "Part Comment1");
//				pdb::Handle<Supplier> supplier = pdb::makeObject<Supplier>(i, "Supplier-" + to_string(i),
//"address", i, "Phone1", 12.1, "Supplier Comment1");
//				pdb::Handle<LineItem> lineItem = pdb::makeObject<LineItem>("Linetem-" + to_string(i), i,
//*supplier, *part, i, 12.1, 12.1, 12.1, 12.1, "ReturnFlag1", "lineStatus1", "shipDate",
//"commitDate", "receiptDate",
//						"sgipingStruct", "shipMode1", "Comment1");
//				lineItems->push_back(*lineItem);
//			}
//
//			pdb::Handle<Order> order = pdb::makeObject<Order>(*lineItems, orderID, 1, "orderStatus", 1,
//"orderDate", "OrderPriority", "clerk", 1, "Order Comment1");
//			orders->push_back(*order);
//		}
//
//		pdb::Handle<Customer> customer = pdb::makeObject<Customer>(*orders, customerID, "CustomerName
//" + to_string(customerID), "address",1, "phone", 12.1, "mktsegment", "Customer Comment "+
//to_string(customerID));
//		customers->push_back(customer);
//	}
//
//	return customers;
//
//}

int main(int argc, char* argv[]) {

    // TPCH Data file scale - Data should be in folder named "tables_scale_"+"scaleFactor"
    string scaleFactor = "0.1";
    int noOfCopies = 1;

    if (argc > 1) {
        scaleFactor = std::string(argv[1]);
    }

    if (argc > 2) {
        noOfCopies = atoi(argv[2]);
    }

    // Connection info
    string masterHostname = "localhost";
    int masterPort = 8108;

    // register the shared employee class
    pdb::PDBLoggerPtr clientLogger = make_shared<pdb::PDBLogger>("clientLog");

    pdb::PDBClient pdbClient(
            masterPort, masterHostname, clientLogger, false, true);

    string errMsg;
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

    // now, create a new database
    if (!pdbClient.createDatabase("TPCH_db", errMsg)) {
        cout << "Not able to create database: " + errMsg;
        exit(-1);
    } else {
        cout << "Created database.\n";
    }

    // now, create the sets for storing Customer Data
    if (!pdbClient.createSet<Customer>(
            "TPCH_db", "tpch_bench_set1", errMsg)) {
        cout << "Not able to create set: " + errMsg;
        exit(-1);
    } else {
        cout << "Created set.\n";
    }

    pdb::makeObjectAllocatorBlock((size_t)2 * GB, true);

    // Generate the data
    dataGenerator(scaleFactor, pdbClient.getDispatcherClient(), noOfCopies);

    //	pdb::Handle<pdb::Vector<pdb::Handle<Customer>>>  storeMeCustomerList =
    //generateSmallDataset(4);
    //	pdb::Record<Vector<Handle<Customer>>>  *myBytes = getRecord <Vector <Handle <Customer>>>
    //(storeMeCustomerList);
    //	size_t sizeOfCustomers = myBytes->numBytes();
    //	cout << "Size of Customer Vector is: " << sizeOfCustomers << endl;

    //	// store copies of the same dataset.
    //	for (int i = 1; i <= noOfCopies; ++i) {
    //		cout << "Storing Vector of Customers - Copy Number : " << i << endl;
    //
    //		if (!dispatcherClient.sendData<Customer>(std::pair<std::string,
    //std::string>("tpch_bench_set1", "TPCH_db"), storeMeCustomerList, errMsg)) {
    //			std::cout << "Failed to send data to dispatcher server" << std::endl;
    //			return -1;
    //		}
    //	}

    // flush to disk
    //	cout<< "Flush data to disk" << endl;
    //	distributedStorageManagerClient.flushData(errMsg);
    //	cout << errMsg << endl;


    if (!pdbClient.registerType("libraries/libSumResultWriteSet.so", errMsg))
        cout << "Not able to register type libSumResultWriteSet.\n";

    if (!pdbClient.registerType("libraries/libCustomerWriteSet.so", errMsg))
        cout << "Not able to register type libCustomerWriteSet.\n";

    if (!pdbClient.registerType("libraries/libScanCustomerSet.so", errMsg))
        cout << "Not able to register type libScanCustomerSet. \n";

    if (!pdbClient.registerType("libraries/libCustomerMultiSelection.so", errMsg))
        cout << "Not able to register type libCustomerMapSelection. \n";

    if (!pdbClient.registerType("libraries/libCustomerSupplierPartGroupBy.so", errMsg))
        cout << "Not able to register type libCustomerSupplierPartGroupBy.\n";

    if (!pdbClient.registerType("libraries/libSupplierData.so", errMsg))
        cout << "Not able to register type  libSupplierData\n";

    if (!pdbClient.registerType("libraries/libCustomerSupplierPartFlat.so", errMsg))
        cout << "Not able to register type  libCustomerSupplierPartFlat\n";

    if (!pdbClient.registerType("libraries/libCountAggregation.so", errMsg))
        cout << "Not able to register type  libCountAggregation\n";

    if (!pdbClient.registerType("libraries/libCountCustomer.so", errMsg))
        cout << "Not able to register type  libCountCustomer\n";


    //	// FIRST WE CHECK THE NUBMER OF STORED CUSTOMERS
    //
    //	// #################################
    //	//   GET the CUSTOMER Number
    //	// #################################
    //
    //
    //	// now, create the sets for storing Customer Data
    //	if (!distributedStorageManagerClient.createSet<CustomerWriteSet>("TPCH_db",
    //"output_setCustomer", errMsg)) {
    //		cout << "Not able to create set: " + errMsg;
    //		exit(-1);
    //	} else {
    //		cout << "Created set.\n";
    //	}
    //	// for allocations
    //	const UseTemporaryAllocationBlock tempBlock_Customers { 1024 * 1024 * 128 };
    //
    //	// make the query graph
    //	Handle<Computation> myScanSet_CUSTOMER = makeObject<ScanCustomerSet>("TPCH_db",
    //"tpch_bench_set1");
    //
    //	Handle<Computation> myWriteSet_Customer = makeObject<CustomerWriteSet>("TPCH_db",
    //"output_setCustomer");
    //	myWriteSet_Customer->setInput(myScanSet_CUSTOMER);
    //
    //	auto begin = std::chrono::high_resolution_clock::now();
    //
    //	if (!queryClient.executeComputations(errMsg, myWriteSet_Customer)) {
    //		std::cout << "Query failed. Message was: " << errMsg << "\n";
    //		return 1;
    //	}
    //
    //	std::cout << std::endl;
    //	auto end = std::chrono::high_resolution_clock::now();
    //	std::cout << "Time Duration: " << std::chrono::duration_cast<std::chrono::nanoseconds>(end -
    //begin).count() << " ns." << std::endl;
    //
    //	SetIterator<Customer> result_Customers = queryClient.getSetIterator<Customer>("TPCH_db",
    //"output_setCustomer");
    //	int customerCount = 0;
    //
    //	// a set to check if we stored all of the customers correctly.
    //	set<int> supplierIDs;
    //
    //	// GO deep inside each customer and add their suppliers IDs to the set.
    //	for (auto a : result_Customers) {
    //		customerCount++;
    //	}
    //
    //	std::cout << "Number of Customers Stored:" << customerCount << "\n";
    //
    //	// CLEAN UP. Remove the Customer output set
    //	if (!distributedStorageManagerClient.removeSet("TPCH_db", "output_setCustomer", errMsg)) {
    //		cout << "Not able to remove the set: " + errMsg;
    //		exit(-1);
    //	} else {
    //		cout << "Set removed. \n";
    //	}
    //
    //
    //
    //
    //	// #################################
    //	// HERE IS THE MAIN EXPERIMENT
    //	// #################################
    //
    //	int count = 0;
    //
    //	// now, create the sets for storing Customer Data
    //	if (!distributedStorageManagerClient.createSet<SumResult>("TPCH_db", "t_output_set_1",
    //errMsg)) {
    //		cout << "Not able to create set: " + errMsg;
    //		exit(-1);
    //	} else {
    //		cout << "Created set.\n";
    //	}
    //
    //	// for allocations
    //	const UseTemporaryAllocationBlock tempBlock { 1024 * 1024 * 128 };
    //
    //	// make the query graph
    //	Handle<Computation> myScanSet = makeObject<ScanCustomerSet>("TPCH_db", "tpch_bench_set1");
    //
    //	Handle<Computation> myFlatten = makeObject<CustomerMultiSelection>();
    //	myFlatten->setInput(myScanSet);
    //
    //	Handle<Computation> myGroupBy = makeObject<CustomerSupplierPartGroupBy>();
    ////	myGroupBy->setAllocatorPolicy(noReuseAllocator);
    //	myGroupBy->setInput(myFlatten);
    //
    //	// Get the count by doing a count aggregation on the final results
    //	Handle<Computation> countAggregation = makeObject<CountAggregation>();
    //	countAggregation->setInput(myGroupBy);
    //
    //
    //	Handle<Computation> myWriteSet = makeObject<CustomerSupplierPartWriteSet>("TPCH_db",
    //"t_output_set_1");
    //	myWriteSet->setInput(countAggregation);
    //
    //
    //	// Query Execution and Time Calculation
    //
    //	begin = std::chrono::high_resolution_clock::now();
    //
    //	if (!queryClient.executeComputations(errMsg, myWriteSet)) {
    //		std::cout << "Query failed. Message was: " << errMsg << "\n";
    //		return 1;
    //	}
    //
    //	std::cout << std::endl;
    //	end = std::chrono::high_resolution_clock::now();
    //
    //	float timeDifference = (float(std::chrono::duration_cast<std::chrono::nanoseconds>(end -
    //begin).count())) / (float) 1000000000;
    //
    //	std::cout << "#TimeDuration: " << timeDifference << " Second " << std::endl;
    //
    //
    //
    //
    //	// Just printing results to double check.
    //
    //
    //	std::cout << "Print result..." << std::endl;
    //	SetIterator<SumResult> result = queryClient.getSetIterator<SumResult>("TPCH_db",
    //"t_output_set_1");
    //
    //	std::cout << "Query results: ";
    //	count = 0;
    //	for (auto a : result) {
    //		count++;
    //		std::cout<<"Total count is: " << a->total <<std::endl;
    //	}
    //	std::cout << "Output count:" << count << "\n";
    //
    //
    //	// Remove the output set
    //	if (!distributedStorageManagerClient.removeSet("TPCH_db", "t_output_set_1", errMsg)) {
    //		cout << "Not able to remove the set: " + errMsg;
    //		exit(-1);
    //	} else {
    //		cout << "Set removed. \n";
    //	}
    //
    //


    //
    //	// Clean up the SO files.
    //	int code = system("scripts/cleanupSoFiles.sh");
    //	if (code < 0) {
    //
    //		std::cout << "Can't cleanup so files" << std::endl;
    //
    //	}
}

#endif
