#!/bin/bash
input_folder="patterns"
metric="$1"
output_folder="outputs_${metric}"

declare -A ghd_confs
ghd_confs=(
    ["bowtie"]="1"
    ["j3_ghd"]="3"
    ["j4_ghd"]="3"
    ["triangle_barbell"]="1"
    ["triangle_tadpole"]="1"
    ["ti4_ghd"]="3"
    ["square_tadpole"]="1"
    ["square_barbell"]="1"
    ["penta_barbell"]="1"
    ["t3_ghd"]="3"
    ["t4_ghd"]="3"
    ["ti3_ghd"]="3"
)

for pattern in "j3_ghd" "j4_ghd" "t3_ghd" "t4_ghd" "ti3_ghd" "ti4_ghd"
do
    params_file="${input_folder}/${pattern}.txt"
    # create output file
    output_file="${output_folder}/${pattern}_mj.csv"
    touch $output_file
    i=1
    if [ "$metric" = "space" ]; then
        echo -e "tuples,qdags" >> $output_file
    fi

    for method in "yk_par"
    do
        for (( ghd_conf=1; ghd_conf<=${ghd_confs[$pattern]}; ghd_conf++ ));
        do
            # create output file
            output_file="${output_folder}/${pattern}_${method}_${ghd_conf}.csv"
            touch $output_file
            if [ "$metric" = "space" ]; then
                echo -e "tuples,qdags,ghd,post mj,result" >> $output_file
            fi
            
            i=1

            while IFS= read -r params; do
                printf "\nRunning ${pattern}_${method}_${ghd_conf} ${i}:\n\t${params}\n"
                i=$((i+1))
                # Run the program with a timeout
                timeout 1800 ./build/${pattern} $params $metric $method $output_file $ghd_conf 
                exit_code=$?

                # check errors
                if  [ $exit_code -eq 124 ]; then
                    echo "timeout" >> $output_file
                    echo "##### timeout #####"
                elif [ $exit_code -eq 139 ]; then
                    echo "segfault" >> $output_file
                    echo "##### segfault #####"
                fi
            done < ${params_file}
        done
    done
done
