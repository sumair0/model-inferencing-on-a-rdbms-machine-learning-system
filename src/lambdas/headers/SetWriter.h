
#ifndef SET_WRITER_H
#define SET_WRITER_H

#include "Computation.h"
#include "TypeName.h"

namespace pdb {

template <class OutputClass>
class SetWriter : public Computation {

    std::string getComputationType() override {
        return std::string("SetWriter");
    }

    // gets the name of the i^th input type...
    std::string getIthInputType(int i) override {
        if (i == 0) {
            return getTypeName<OutputClass>();
        } else {
            return "";
        }
    }

    // get the number of inputs to this query type
    int getNumInputs() override {
        return 1;
    }

    // gets the output type of this query as a string
    std::string getOutputType() override {
        return getTypeName<OutputClass>();
    }

    bool needsMaterializeOutput() override {
        return true;
    }

    // below function implements the interface for parsing computation into a TCAP string
    std::string toTCAPString(std::vector<InputTupleSetSpecifier>& inputTupleSets,
                             int computationLabel,
                             std::string& outputTupleSetName,
                             std::vector<std::string>& outputColumnNames,
                             std::string& addedOutputColumnName) override {

        if (inputTupleSets.size() == 0) {
            return "";
        }
        InputTupleSetSpecifier inputTupleSet = inputTupleSets[0];
        return toTCAPString(inputTupleSet.getTupleSetName(),
                            inputTupleSet.getColumnNamesToKeep(),
                            inputTupleSet.getColumnNamesToApply(),
                            computationLabel,
                            outputTupleSetName,
                            outputColumnNames,
                            addedOutputColumnName);
    }

    // below function returns a TCAP string for this Computation
    std::string toTCAPString(std::string inputTupleSetName,
                             std::vector<std::string>& inputColumnNames,
                             std::vector<std::string>& inputColumnsToApply,
                             int computationLabel,
                             std::string& outputTupleSetName,
                             std::vector<std::string>& outputColumnNames,
                             std::string& addedOutputColumnName) {
        std::string ret = std::string("out() <= OUTPUT (") + inputTupleSetName + " (" +
            inputColumnsToApply[0] + ")" + std::string(", '") + std::string("EmptySet()") +
            std::string("', '") + std::string("EmptySet()") + std::string("', '") +
            getComputationType() + std::string("_") + std::to_string(computationLabel) +
            std::string("')\n");
        outputTupleSetName = "out";
        outputColumnNames.push_back("");
        addedOutputColumnName = "";
        this->setTraversed(true);
        this->setOutputTupleSetName(outputTupleSetName);
        this->setOutputColumnToApply(addedOutputColumnName);
        return ret;
    }
};
}

#endif
