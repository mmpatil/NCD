#!/bin/sh

# This script will run the experiment on the remote host
# It is responsible for invoking the test program and reporting
# results to the DB
#
# Early versions will use hard-coded IP addresses
# while later versions will use parameters to target the correct IP

#variables

# the IP that the planet lab nodes will target
#TODO change the target IP to be the real IP before deployment
PL_TARGET_IP=127.0.0.1
#COMMAND_NAME="echo $PL_TARGET_IP "
PROJECT=NCD
TIMEOUT=90

#functions

# get results from file temp.txt
get_results(){
    ./sql_test.sh $PROJECT "$TEST_NAME" "$COMMAND_NAME" $SUCCESS $(cat $1)
    rm $1
}

#Defaults
udp_default(){
    TEST_NAME=${TEST_NAME:="Default UDP"}
    COMMAND_NAME="../ncd_main -o ${OPTIONS} $PL_TARGET_IP"
    sudo timeout $TIMEOUT ${COMMAND_NAME} > temp.txt
    if [ "$?" = 124 ]
    then
        SUCCESS=0
    else
        SUCCESS=1
    fi
    get_results temp.txt
}

tcp_default(){
    TEST_NAME=${TEST_NAME:="Default TCP"}
    OPTIONS="$OPTIONS -T"
    udp_default
}

#Port 22222
udp_test(){
    TEST_NAME="UDP Port 22222"
    OPTIONS="-p 22222"
    udp_default
}

port_tests()
{

    COMMAND_NAME="../ncd_main -oL ${OPTIONS} $PL_TARGET_IP"
    ${COMMAND_NAME} > low.txt

    COMMAND_NAME="../ncd_main -oH ${OPTIONS} $PL_TARGET_IP"
    ${COMMAND_NAME} >high.txt



}

udp_ports(){
    TEST_NAME=${TEST_NAME:="UDP Different Ports"}
    sudo timeout port_tests
    if [ "$?" = 124 ]
    then
        SUCCESS=0
    else
        SUCCESS=1
    fi

    OPTIONS=
    sudo ./ncd_main $PL_TARGET_IP > temp.txt
    get_results temp.txt
}


#begin comment block
: '

# TOS
udp_default(){
    OPTIONS=
    sudo ./ncd_main $PL_TARGET_IP > temp.txt
    get_results temp.txt
}

udp_default(){
    OPTIONS=
    sudo ./ncd_main $PL_TARGET_IP > temp.txt
    get_results temp.txt
}


# Files
udp_default(){
    OPTIONS=
    sudo ./ncd_main $PL_TARGET_IP > temp.txt
    get_results temp.txt
}

udp_default(){
    OPTIONS=
    sudo ./ncd_main $PL_TARGET_IP > temp.txt
    get_results temp.txt
}

'
#end comment block

#main script

