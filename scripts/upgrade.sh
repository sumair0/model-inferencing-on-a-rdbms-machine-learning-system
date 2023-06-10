#!/bin/bash

pem_file=$1

user=`id -u -n`
ip_len_valid=3
pdb_dir=$PDB_INSTALL
#echo "To strip shared libraries..."
#strip libraries/*.so
#echo "stripped all shared libraries!"

# By default disable strict host key checking
if [ "$PDB_SSH_OPTS" = "" ]; then
  PDB_SSH_OPTS="-o StrictHostKeyChecking=no"
fi

if [ -z ${pem_file} ];
then
  PDB_SSH_OPTS=$PDB_SSH_OPTS
else
  PDB_SSH_OPTS="-i ${pem_file} $PDB_SSH_OPTS"
fi

if [ "$PLINY_HOME" = "" ]; then
  echo "We do not have pliny dependency."
else
  rm -rf bin
  mkdir bin
  scripts/syncWithPliny.sh
fi

arr=($(awk '{print $0}' $PDB_HOME/conf/serverlist))
length=${#arr[@]}
echo "There are $length servers"
for (( i=0 ; i<=$length ; i++ ))
do
        ip_addr=${arr[i]}
        if [ ${#ip_addr} -gt "$ip_len_valid" ]
        then
                echo -e "\n+++++++++++ install server: $ip_addr"
                ssh $PDB_SSH_OPTS $user@$ip_addr "rm $pdb_dir/log.out; rm $pdb_dir/logs/*"
                scp $PDB_SSH_OPTS -r $PDB_HOME/bin/pdb-server $user@$ip_addr:$pdb_dir/bin/ 
                scp $PDB_SSH_OPTS -r $PDB_HOME/scripts/cleanupNode.sh $PDB_HOME/scripts/startWorker.sh $PDB_HOME/scripts/stopWorker.sh $PDB_HOME/scripts/checkProcess.sh $user@$ip_addr:$pdb_dir/scripts/
        fi
done


