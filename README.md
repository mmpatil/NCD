

NCD
===

Network Compression Detection
------------------------------

This project began as a class project for a networking course at CSUN, to
detect compression along a network transmission path. It has since become a
research project of its own, examining the ability to use existing network
facilities, particularly ICMP messages, to detect compression links in an
uncooperative environment.

At its core `NCD` is a command-line tool similar to `traceroute` or `ping` that
researchers and network admins can use to identify compression links along a
network path. We believe this technique can be broadly applied to the detection
of other middle-box behaviors. We are, however, focusing on network compression
as a viable test case. Eventually we would like NCD to have support for many
types of middle-boxes, such as deep packet inspection or other potentially
intrusive behaviors.

'NCD' works by exploiting the different processing times that high entropy and
low entropy data exhibit. A low entropy packet, i.e. containing all 0's, should
compress much better - and more quickly - than a high entropy packet,
containing "random" data. We expect the low entropy transmission times will
differ significantly from the high entropy data transmission times, and give us
a reliable way to detect compression. This work follows that of Vahab
Pournaghshband in [End-to-End Detection of Compression of Traffic Flows by
Intermediaries]
(http://lasr.cs.ucla.edu/vahab/resources/compression_detection.pdf), who is my
advisor on this project.

Current Status    [![Build Status](https://travis-ci.org/ilovepi/NCD.svg?branch=master)](https://travis-ci.org/ilovepi/NCD)
--------------
Currently `NCD` is under heavy development, and
isn't suitable for general use. It should be considered highly experimental,
and unstable at best. We hope that in the near future it will be a far more
robust tool that other researchers can use and improve. But by all means feel
free to explore the source code, and experiment with the tool. 

Once our research is complete, and we can reliably use `NCD` in uncooperative
environments we plan to make `NCD` a more fully featured, more reliable CLI
tool better suite for general use.

