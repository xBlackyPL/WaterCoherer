#!/bin/bash

program=$1

for ((i=0; $i <= 100; i++ )) ; do
       $program >> output_time.txt
       echo "Iteration: $i"
       rm ./build/result.tiff
done
