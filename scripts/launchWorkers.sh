#!/bin/bash
# by Jia/modified by Carlos

pem_file=$1
masterIp=$2
numThreads=$3
sharedMem=$4
user=`id -u -n`
ip_len_valid=3

if [ -z ${pem_file} ];
    then echo "ERROR: please provide at least two parameters: one is your the path to your pem file and the other is the master IP";
    echo "Usage: scripts/startWorkers.sh #pem_file #masterIp #threadNum #sharedMemSize";
    exit -1;
fi

if [ -z ${masterIp} ];
    then echo "ERROR: please provide at least two parameters: one is the path to your pem file and the other is the master IP";
    echo "Usage: scripts/startWorkers.sh #pem_file #masterIp #threadNum #sharedMemSize";
    exit -1;
fi
    
if [ -z ${numThreads} ];
     then numThreads=4;
fi

if [ -z ${sharedMem} ];
     then sharedMem=10240;
fi

arr=($(awk '{print $0}' $PDB_HOME/conf/serverlist))
length=${#arr[@]}
echo "There are $length servers"
for (( i=0 ; i<=$length ; i++ ))
do
        ip_addr=${arr[i]}
        if [ ${#ip_addr} -gt "$ip_len_valid" ]
        then
        
                echo -e "\n+++++++++++ install and start server: $ip_addr"
                ssh -i $pem_file $user@$ip_addr "rm -rf /tmp/pdb_temp; 
                                                 mkdir /tmp/pdb_temp;
                                                 mkdir /tmp/pdb_temp/scripts;
                                                 mkdir /tmp/pdb_temp/bin; 
                                                 mkdir /tmp/pdb_temp/CatalogDir;
                                                 mkdir /tmp/pdb_temp/logs" &&
                                                 
                scp -i $pem_file -r $PDB_HOME/bin/test603 $user@$ip_addr:/tmp/pdb_temp/bin/ &&
                scp -i $pem_file -r $PDB_HOME/scripts/cleanupNode.sh $user@$ip_addr:/tmp/pdb_temp/scripts/ &&
                
                ssh -f -i $pem_file $user@$ip_addr "/tmp/pdb_temp/scripts/cleanupNode.sh; cd /tmp/pdb_temp;  nohup bin/test603 $numThreads $sharedMem $masterIp $ip_addr  &" &
                
                sleep 5
                echo -e "Waits 5 seconds before registering node metadata..."
                sleep 5
                
        fi
done

echo "servers are started"
