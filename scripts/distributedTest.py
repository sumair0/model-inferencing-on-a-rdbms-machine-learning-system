#!python
#JiaNote: to run integrated tests on standalone node for storing and querying data


import subprocess
import time
class bcolors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'


print("#################################")
print("RUN DISTRIBUTED INTEGRATION TESTS")
print("#################################")

try:
    for num in range(1, 100):
        #cleanup
        print bcolors.OKBLUE + "cleanup environment" + bcolors.ENDC
        subprocess.call(['bash', './scripts/cleanupNode.sh'])
        print bcolors.OKBLUE + "waiting for 10 seconds for system to be fully cleared..." + bcolors.ENDC
        time.sleep(10)

        #run bin/pdb-cluster
        print bcolors.OKBLUE + "start a pdbServer as the coordinator" + bcolors.ENDC
        serverProcess = subprocess.Popen(['bin/pdb-cluster', 'localhost', '8108', 'Y'])
        print bcolors.OKBLUE + "waiting for 9 seconds for server to be fully started..." + bcolors.ENDC
        time.sleep(9)
        

        #run bin/pdb-server for instance 1
        print bcolors.OKBLUE + "start a pdbServer as the 1st worker" + bcolors.ENDC
        serverProcess = subprocess.Popen(['bin/pdb-server', '1', '512', 'localhost:8108', 'localhost:8109'])
        print bcolors.OKBLUE + "waiting for 9 seconds for server to be fully started..." + bcolors.ENDC
        time.sleep(9)

        #run bin/pdb-server for instance 2
        print bcolors.OKBLUE + "start a pdbServer as the 2nd worker" + bcolors.ENDC
        serverProcess = subprocess.Popen(['bin/pdb-server', '1', '512', 'localhost:8108', 'localhost:8110'])
        print bcolors.OKBLUE + "waiting for 9 seconds for server to be fully started..." + bcolors.ENDC
        time.sleep(9)

        #run bin/pdb-server for instance 3
        print bcolors.OKBLUE + "start a pdbServer as the 3rd worker" + bcolors.ENDC
        serverProcess = subprocess.Popen(['bin/pdb-server', '1', '512', 'localhost:8108', 'localhost:8111'])
        print bcolors.OKBLUE + "waiting for 9 seconds for server to be fully started..." + bcolors.ENDC
        time.sleep(9)

        #run bin/pdb-server for instance 4
        print bcolors.OKBLUE + "start a pdbServer as the 4th worker" + bcolors.ENDC
        serverProcess = subprocess.Popen(['bin/pdb-server', '1', '512', 'localhost:8108', 'localhost:8112'])
        print bcolors.OKBLUE + "waiting for 9 seconds for server to be fully started..." + bcolors.ENDC
        time.sleep(9)

        #run bin/test52
        print bcolors.OKBLUE + "start a query client to store and query data from pdb cluster" + bcolors.ENDC
        subprocess.check_call(['bin/test52', 'N', 'Y', '1024', 'localhost'])


except subprocess.CalledProcessError as e:
    print bcolors.FAIL + "[ERROR] in running distributed integration tests" + bcolors.ENDC
    print e.returncode
else:
    print bcolors.OKBLUE + "[PASSED] distributed integration tests" + bcolors.ENDC

print bcolors.OKBLUE + str(num) + " tests passed in total" + bcolors.ENDC
