#!python
#JiaNote: to run integrated tests on standalone node for storing and querying data


import subprocess
import sys
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

threadNum = "2"
#JiaNote: because now we have 256MB pages, and we define that
#(conf->getShmSize()/conf->getPageSize()-2 > 2+2*numThreads+1)
#so it means shared memory size must be larger than 9 pages, at least 10 pages
#if you can't meet this memory requirement in your platform, please
#(1) change page size smaller, in src/conf/headers/Configuration.h DEFAULT_PAGE_SIZE, to be 64MB, then set buffer pool size larger than 9*YOUR_PAGE_SIZE
sharedMemorySize = "2560"
#sharedMemorySize = "640"

def startPseudoCluster():
    try:
        #run bin/pdb-cluster
        print bcolors.OKBLUE + "start a pdbServer as the coordinator" + bcolors.ENDC
        serverProcess = subprocess.Popen(['bin/pdb-cluster', 'localhost', '8108', 'Y'])
        print bcolors.OKBLUE + "waiting for 9 seconds for server to be fully started..." + bcolors.ENDC
        time.sleep(9)

        #run bin/pdb-server for worker
        num = 0;
        with open('conf/serverlist.test') as f:
            for each_line in f:
                print bcolors.OKBLUE + "start a pdbServer at " + each_line + "as " + str(num) + "-th worker" + bcolors.ENDC
                num = num + 1
                serverProcess = subprocess.Popen(['bin/pdb-server', threadNum, sharedMemorySize, 'localhost:8108', each_line])
                print bcolors.OKBLUE + "waiting for 9 seconds for server to be fully started..." + bcolors.ENDC
                time.sleep(9)
                each_line = each_line.split(':')
                port = int(each_line[1])

    except subprocess.CalledProcessError as e:
        print bcolors.FAIL + "[ERROR] in starting peudo cluster" + bcolors.ENDC
        print e.returncode


print("#################################")
print("CLEAN UP THE TESTING ENVIRONMENT")
print("#################################")
subprocess.call(['bash', './scripts/cleanupNode.sh'])
print bcolors.OKBLUE + "waiting for 5 seconds for server to be fully cleaned up..."
time.sleep(5)

print("#################################")
print 'RUN DISTRIBUTED TEST:', sys.argv[1]
print("#################################")

try:
    #start pseudo cluster
    startPseudoCluster()

    #run test
    print bcolors.OKBLUE + "start a query client to store and query data from pdb cluster" + bcolors.ENDC
   
    subprocess.check_call([sys.argv[1], 'Y', 'Y', '64', 'localhost', sys.argv[2], sys.argv[3], sys.argv[4], sys.argv[5], sys.argv[6], sys.argv[7]])	

except subprocess.CalledProcessError as e:
    print bcolors.FAIL + "[ERROR] in running distributed test " + sys.argv[1] + bcolors.ENDC
    print e.returncode

else:
    print bcolors.OKBLUE + "[PASSED] distributed test " + sys.argv[1] + bcolors.ENDC
    #subprocess.call(['bash', './scripts/cleanupNode.sh'])



                                              
