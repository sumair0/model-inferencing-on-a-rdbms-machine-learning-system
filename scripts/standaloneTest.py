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
print("RUN STANDALONE INTEGRATION TESTS")
print("#################################")

try:
    for num in range(1, 100):
        #cleanup
        print bcolors.OKBLUE + "cleanup environment" + bcolors.ENDC
        subprocess.call(['bash', './scripts/cleanupNode.sh'])
        print bcolors.OKBLUE + "waiting for 10 seconds for system to be fully cleared..." + bcolors.ENDC
        time.sleep(10)
        #run bin/pdb-server
        print bcolors.OKBLUE + "start a standalone pdbServer" + bcolors.ENDC
        serverProcess = subprocess.Popen(['bin/pdb-server', '1', '512'])
        print bcolors.OKBLUE + "waiting for 10 seconds for server to be fully started..." + bcolors.ENDC
        time.sleep(10)
        #run bin/test46 1024
        print bcolors.OKBLUE + "start a data client to store data to pdbServer" + bcolors.ENDC
        subprocess.check_call(['bin/test46', '640'])
        #run bin/test44 in a loop for 10 times
        print bcolors.OKBLUE + "start a query client to query data from pdbServer" + bcolors.ENDC
        subprocess.check_call(['bin/test44', 'n'])
        print bcolors.OKBLUE + "waiting for 50 seconds for connections to be fully closed..." + bcolors.ENDC
        time.sleep(50)
except subprocess.CalledProcessError as e:
    print bcolors.FAIL + "[ERROR] in running standalone integration tests" + bcolors.ENDC
    print e.returncode
else:
    print bcolors.OKBLUE + "[PASSED] standalone integration tests" + bcolors.ENDC

print bcolors.OKBLUE + str(num) + " tests passed in total" + bcolors.ENDC
