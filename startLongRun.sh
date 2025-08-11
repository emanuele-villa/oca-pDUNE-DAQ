#!/bin/bash

# Exit if no argument is provided
if [ -z "$1" ]; then
  echo "No initial run number provided. Exiting."
  exit 0
fi

initial_run_number=$1
end_run_number=$((initial_run_number + 2))

for ((i=initial_run_number; i<end_run_number; i+=2))
do
    echo "******************************"
    echo "   Taking calibration run $i"
    echo "******************************"
    ./exe/startOCA 0 $i
    sleep 10
    ./exe/stopOCA
    echo "**********************************"
    echo "   Taking trigger run $((i+1))"
    echo "**********************************"
    ./exe/startOCA 0 $((i+1))
    #sleep $((3*60*60))   # 3 hours in seconds
    sleep 30
    ./exe/stopOCA
done
