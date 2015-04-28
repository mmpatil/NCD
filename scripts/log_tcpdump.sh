#!/bin/bash
cleanup()
{
  kill %2
  kill %1
  return $?
}

control_c()
{
  cleanup
  echo -en "\n*** tcpdump logging complete ***\n"
  exit $?
}

trap control_c SIGINT
trap control_c SIGTERM

FILENAME=`./make_log.sh tcpdump`
nc 9876 -l&

#sudo iptables -A OUTPUT -p tcp --tcp-flags RST RST -j DROP

./test_runner.sh "sudo tcpdump -i any -v" > $FILENAME&
#./test_runner.sh "sudo tcpdump -i any -v -w $FILENAME" > ${FILENAME}_meta
wait

#sudo iptables -D OUTPUT -p tcp --tcp-flags RST RST -j DROP
