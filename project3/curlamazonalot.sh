#!/bin/bash 
rm curlamazonlog
for i in `seq 1 2000`;
do
	curl -s cs360.kylerichey.com:3000/test1.html >> curlamazonlog &
	curl -s cs360.kylerichey.com:3000/test2.txt >> curlamazonlog &
	
done    



