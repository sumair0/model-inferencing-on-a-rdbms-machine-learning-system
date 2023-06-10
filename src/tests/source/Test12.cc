
#define NUM_OBJECTS 10371

#include <cstddef>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <iterator>
#include <cstring>
#include <chrono>
#include <ctime>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string>

#include "Handle.h"
#include "PDBVector.h"
#include "InterfaceFunctions.h"
#include "Employee.h"
#include "Supervisor.h"
#include "ZB_Company.h"

using namespace pdb;

int main() {

    makeObjectAllocatorBlock(1024 * 1024 * 24, true);

    // get the file size
    std::ifstream in2("testfile7", std::ifstream::ate | std::ifstream::binary);
    size_t fileLen = in2.tellg();

    // read in the guys that we wrote out
    int filedesc = open("testfile7", O_RDONLY);
    Record<ZB_Company>* myCompany = (Record<ZB_Company>*)malloc(fileLen);
    // added by Jia to remove warnings
    size_t sizeRead = read(filedesc, myCompany, fileLen);
    if (sizeRead == 0) {
        std::cout << "Read failed" << std::endl;
    }
    auto myComp = myCompany->getRootObject();
    myComp->print();

    // now, we'll create a new list of supervisors and copy them over
    Vector<Supervisor> mySups;
    for (int i = 0; i < 5; i++) {
        mySups.push_back(*(myComp->getSupervisor(i)));
    }

    close(filedesc);

    // and go ahead and write this guy out
    Handle<Vector<Supervisor>> myHandle = getHandle(mySups);
    auto myBytes = getRecord(myHandle);
    filedesc = open("testfile8", O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
    size_t sizeWritten = write(filedesc, myBytes, myBytes->numBytes());
    if (sizeWritten == 0) {
        std::cout << "Write failed" << std::endl;
    }
    close(filedesc);

    // now finally, we will read him in
    std::ifstream in3("testfile8", std::ifstream::ate | std::ifstream::binary);
    fileLen = in3.tellg();
    filedesc = open("testfile8", O_RDONLY);
    Record<Vector<Supervisor>>* allSups = (Record<Vector<Supervisor>>*)malloc(fileLen);
    // added by Jia to remove warnings
    sizeRead = read(filedesc, allSups, fileLen);
    if (sizeRead == 0) {
        std::cout << "Read failed" << std::endl;
    }
    auto mySupervisors = allSups->getRootObject();

    for (int i = 0; i < mySupervisors->size(); i++) {
        (*mySupervisors)[i].print();
    }

    free(allSups);
    free(myCompany);
}
