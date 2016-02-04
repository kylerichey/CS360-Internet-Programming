#!/bin/bash 
rm curllocalhostlog
for i in `seq 1 2000`;
do
	curl -s localhost:8081/test1.html >> curllocalhostlog &
	curl -s localhost:8081/test2.txt >> curllocalhostlog &
	
done    



