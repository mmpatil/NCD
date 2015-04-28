#!/bin/sh
DATE="date +%F"
DIRNAME=../test_data/$(hostname)/$($DATE)
if [ -d "$DIRNAME" ]
then
	COUNT=`\ls -afq $DIRNAME | wc -l`
	COUNT=`expr $COUNT - 1`
else
	COUNT=1
	mkdir -p $DIRNAME
fi

echo $DIRNAME/"$1_$COUNT"
