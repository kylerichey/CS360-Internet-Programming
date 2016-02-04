#!/bin/bash 
for i in `seq 1 1000`;
do
	curl -s localhost:8081/test1.html > /dev/null &
	curl -s localhost:8081/test4.jpg > /dev/null &
	curl -s localhost:8081/test3.gif > /dev/null &
done    



