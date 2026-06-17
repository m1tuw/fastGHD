output_file="/home/anouk/Documents/qdags/qdags-main/runqueries/outputs/square_tadpole_yk_par.txt"
params_file="/home/anouk/Documents/wikidata/property_getter/query-pattern-finder/square_tadpole.txt"


while IFS= read -r params; do
    # Run the program with a timeout
    echo "Running experiment with parameters: $params"
    timeout 1800 ./square_tadpole $params

    # Check if the program timed out
    if [ $? -eq 124 ]; then
        # Log the timeout message
        echo "timeout" >> "$output_file"
    fi
done < "$params_file"
