
#include <cstddef>
#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <iterator>
#include <cstring>
#include <ctime>
#include <chrono>
#include <Object.h>

using namespace pdb;

class Employee : public Object {
    std::string* name;
    int age;

public:
    ~Employee() {
        delete name;
    }

    Employee(const Employee& toMe) {
        name = new std::string(*(toMe.name));
        age = toMe.age;
    }

    Employee& operator=(const Employee& toMe) {
        name = new std::string(*(toMe.name));
        age = toMe.age;
        return *this;
    }

    void print() {
        std::cout << "name is: " << *name << " age is: " << age << "\n";
    }

    Employee(std::string nameIn, int ageIn) {
        name = new std::string(nameIn);
        age = ageIn;
    }
};

int main() {

    // for timing
    auto begin = std::chrono::high_resolution_clock::now();

    std::vector<std::vector<Employee>>* data = new std::vector<std::vector<Employee>>();

    for (int i = 0; i < 100; i++) {

        std::vector<Employee> temp;
        for (int j = 0; j < 100; j++) {
            std::stringstream ss;
            ss << "myEmployee " << i << ", " << j;
            Employee temp2(ss.str(), j);
            temp.push_back(temp2);
        }

        data->push_back(temp);
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Duration to create all of the objects: "
              << std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count() << " ns."
              << std::endl;
}
