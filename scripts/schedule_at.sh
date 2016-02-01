#!/bin/sh

while read line
do
    arr = $(echo $line | tr "\t" "\n")
    at $arr[0] << ENDMARKER
    $($arr[3])
        ENDMARKER
done < $1
