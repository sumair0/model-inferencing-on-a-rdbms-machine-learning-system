#!/bin/bash
# by Jia

ip_len_valid=3
pem_file=$1
user=$2

arr=($(awk '{print $0}' $PDB_HOME/conf/serverlist))
length=${#arr[@]}
echo "There are $length servers"
#echo "Server List: ${arr[@]}"
for (( i=0 ; i<=$length ; i++ ))
do
	ip_addr=${arr[i]}
        if [ ${#ip_addr} -gt "$ip_len_valid" ]
        then
            echo -e "\n+++++++++++ collect info for server: $ip_addr"            
            ssh -i $pem_file $user@$ip_addr '~/pdb_temp/local_sys_collect.sh' #todo
        fi
done


