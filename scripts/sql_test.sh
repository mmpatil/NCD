#!/bin/sh
PROJECTNAME="set @projectName=\"$1\";"
TESTNAME="set @testName=\"$2\";"
TESTDATE="set @testDate=NOW();"
COMMANDNAME="set @commandName=\"$3\";"
HOSTIP="set @hostIP=\"$4\";"
DESTIP="set @destIP=\"$5\";"
IS_SUCCESS="set @isSuccess=$6;"
HIGH_TIME="set @highTime=$7;"
LOW_TIME="set @lowTime=$8;"
HIGH_LOSSSES="set @highLosses=\"$9\";"
LOW_LOSSES="set @lowLosses=\"${10}\";"
NUM_TAIL="set @numTail=${11};"
DEST_PORT="set @destPort=${12};"
SRC_PORT="set @srcPort=${13};"
NUM_PACKETS="set @numPackets=${14};"
PACKET_SIZE="set @packetSize=${15};"
PROTOCOL_NAME="set @protocolName=\"${16}\";"

SQL_VARIABLES="$PROJECTNAME $TESTNAME $TESTDATE $COMMANDNAME $HOSTIP $DESTIP $IS_SUCCESS $HIGH_TIME $LOW_TIME $HIGH_LOSSSES $LOW_LOSSES $NUM_TAIL $DEST_PORT $SRC_PORT $NUM_PACKETS $PACKET_SIZE $PROTOCOL_NAME"

SQL_COMMAND="$SQL_VARIABLES insert into data (project,test_name,test_date,command,host_ip, dest_ip, success, high_time, low_time, high_losses, low_losses, num_tail, dest_port, src_port, num_packets, packet_size, protocol) \
          Values (@projectName, @testName, @testDate, @commandName, @hostIP, @destIP , @isSuccess, @highTime, @lowTime, @highLosses, @lowLosses, @numTail, @destPort, @srcPort, @numPackets, @packetSize, @protocolName);"

#echo $SQL_COMMAND

mysql -u root -p test -e "$SQL_COMMAND"

