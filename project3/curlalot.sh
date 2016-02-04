#!/bin/bash 
for i in `seq 1 1000`;
do
	curl -s cs360.kylerichey.com:3000/test1.html > /dev/null &
	curl -s cs360.kylerichey.com/test4.jpg > /dev/null &
	curl -s cs360.kylerichey.com/test3.gif > /dev/null &
done    



