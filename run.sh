#!/bin/bash

declare -g -a exe_arr=('tls12_rm_cert1' 'tls12_rm_cert2')

# Compile it first
#make

tot_count=0
failed_count=0
passed_count=0

for item in ${exe_arr[@]}; do
    echo "Testing ${item}_server and ${item}_client"
    ./${item}_server 1>/dev/null &
    pid=$!
    ./${item}_client 1>/dev/null
    if [ $? != 0 ]; then
        ${item}_client failed
        ((failed_count++))
    fi
    wait $pid
    if [ $? != 0 ]; then
        ${item}_server failed
        ((failed_count++))
    fi
    tot_count=$((tot_count+2))
done

passed_count=$((tot_count-failed_count))

echo "====================================="
echo "Totat TC : ${tot_count}"
echo "Passed TC : ${passed_count}"
echo "Failed TC : ${failed_count}"
echo "====================================="



