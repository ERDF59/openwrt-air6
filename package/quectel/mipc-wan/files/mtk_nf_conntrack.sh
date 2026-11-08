#!/bin/sh

file="/tmp/nf_conntrack_dump.log"
if [ -d "/data/debuglog" ] ; then
    file="/data/debuglog/nf_conntrack_dump.log"
fi

run_cmd() {
    echo -e "\n----[$@]---- \n" >> $file
    eval "$@" >> $file
}

echo "" > $file

while [ true ]; do
    run_cmd "date"
    run_cmd "cat /proc/net/nf_conntrack"
    run_cmd "hwnat -g"
    sleep 1
done
