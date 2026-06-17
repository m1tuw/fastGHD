output_file="/mnt/c/Users/crist/Documents/ccqQDAGS/runqueries/outputs/trad_j3_yk_log.txt"
params_file="/mnt/c/Users/crist/Documents/query-pattern-finder/j3.txt"
 
 
 while IFS= read -r params; do
     # Run the program with a timeout
     echo "Running experiment with parameters: $params"
     timeout 1800 ./build/j3_ghd $params
 
     # Check if the program timed out
     if [ $? -eq 124 ]; then
         # Log the timeout message
         printf "timeout\n" > $output_file
     fi
     if [ $? -eq 139 ]; then
         # Log the segfault message
         printf "segfault\n" > $output_file
     fi
 done < "$params_file"