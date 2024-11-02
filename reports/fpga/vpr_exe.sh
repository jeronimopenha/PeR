#!/bin/bash

# Check if the correct number of arguments is provided
if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <input_folder> <output_folder>"
    exit 1
fi

# Get the input and output directories from command-line arguments
input_dir="$1"
#output_dir="$2"

# Create the output directory if it doesn't exist
#mkdir -p "$output_dir"

# Loop over all files in the input directory that contain "_k4" in their name
for file in "$input_dir"/*_k3*; do
    # Check if the file exists to avoid issues when no matching files are found
    if [ -f "$file" ]; then
        # Get the file name without the path and extension
        filename=$(basename -- "$file")
        filename_no_ext="${filename%.*}"

        bin/vpr "$file" "arch/k3-n1.xml" "vpr_reports/fast/${filename_no_ext}_place.out" "vpr_reports/fast/${filename_no_ext}_route.out" -nodisp -fast > "vpr_reports/fast/${filename_no_ext}.out"

        # Run the 'x' program on the file, assuming it generates two output files
        #./x "$file"

        # Move the two generated files to the output directory, renaming them with the file name suffix
        #mv output1.txt "$output_dir/output_${filename_no_ext}_1.txt"
        #mv output2.txt "$output_dir/output_${filename_no_ext}_2.txt"

        #echo "Processed $file and saved results as output_${filename_no_ext}_1.txt and output_${filename_no_ext}_2.txt"
    else
        echo "No files found with '_k4' in the name."
    fi
done
