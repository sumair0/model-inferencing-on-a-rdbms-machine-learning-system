
#ifndef TEST_62_H
#define TEST_62_H


// by Jia, Mar 2017
// to test TCAP generation

#include "PDBDebug.h"
#include "PDBString.h"
#include "Query.h"
#include "Lambda.h"
#include "ScanEmployeeSet.h"
#include "WriteStringSet.h"
#include "EmployeeSelection.h"
#include "SharedEmployee.h"
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

    Handle<Computation> myScanSet = makeObject<ScanEmployeeSet>("chris_db", "chris_set");
    Handle<Computation> myQuery = makeObject<EmployeeSelection>();
    Handle<Computation> myWriteSet = makeObject<WriteStringSet>("chris_db", "output_set1");

    std::string inputTupleSetName = "";
    std::vector<std::string> inputColumnNames;
    std::vector<std::string> inputColumnsToApply;
    std::string outputTupleSetName;
    std::vector<std::string> outputColumnNames;
    std::string addedOutputColumnName;

    InputTupleSetSpecifier inputTupleSet1(inputTupleSetName, inputColumnNames, inputColumnsToApply);
    std::vector<InputTupleSetSpecifier> inputTupleSets1;
    inputTupleSets1.push_back(inputTupleSet1);


    std::string myScanSetTcapString = myScanSet->toTCAPString(
        inputTupleSets1, 0, outputTupleSetName, outputColumnNames, addedOutputColumnName);
    // std :: cout << myScanSetTcapString;

    inputTupleSetName = outputTupleSetName;
    inputColumnNames.clear();
    for (int i = 0; i < outputColumnNames.size(); i++) {
        inputColumnNames.push_back(outputColumnNames[i]);
    }
    inputColumnsToApply.clear();
    inputColumnsToApply.push_back(addedOutputColumnName);
    outputTupleSetName = "";
    outputColumnNames.clear();
    addedOutputColumnName = "";

    PDB_COUT << "INPUT COLUMN NAMES:" << std::endl;
    for (int i = 0; i < inputColumnNames.size(); i++) {
        PDB_COUT << inputColumnNames[i] << std::endl;
    }
    PDB_COUT << "INPUT COLUMNS TO APPLY:" << std::endl;
    for (int i = 0; i < inputColumnsToApply.size(); i++) {
        PDB_COUT << inputColumnsToApply[i] << std::endl;
    }

    InputTupleSetSpecifier inputTupleSet2(inputTupleSetName, inputColumnNames, inputColumnsToApply);
    std::vector<InputTupleSetSpecifier> inputTupleSets2;
    inputTupleSets2.push_back(inputTupleSet2);

    std::string myQueryString = myQuery->toTCAPString(
        inputTupleSets2, 1, outputTupleSetName, outputColumnNames, addedOutputColumnName);
    // std :: cout << myQueryString;

    inputTupleSetName = outputTupleSetName;
    inputColumnNames.clear();
    for (int i = 0; i < outputColumnNames.size(); i++) {
        inputColumnNames.push_back(outputColumnNames[i]);
    }
    inputColumnsToApply.clear();
    inputColumnsToApply.push_back(addedOutputColumnName);
    outputTupleSetName = "";
    outputColumnNames.clear();
    addedOutputColumnName = "";

    InputTupleSetSpecifier inputTupleSet3(inputTupleSetName, inputColumnNames, inputColumnsToApply);
    std::vector<InputTupleSetSpecifier> inputTupleSets3;
    inputTupleSets3.push_back(inputTupleSet3);

    std::string myWriteSetTcapString = myWriteSet->toTCAPString(
        inputTupleSets3, 2, outputTupleSetName, outputColumnNames, addedOutputColumnName);
    std::cout << myScanSetTcapString << myQueryString << myWriteSetTcapString << std::endl;
    int code = system("scripts/cleanupSoFiles.sh");
    if (code < 0) {
        std::cout << "Can't cleanup so files" << std::endl;
    }
}

#endif
