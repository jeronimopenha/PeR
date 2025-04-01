#!/bin/bash

# Check if the directory is provided
if [ -z "$1" ]; then
  echo "Usage: $0 <path_to_directory>"
  exit 1
fi

if [ -z "$2" ]; then
  echo "Usage: $0 <path_to_output>"
  exit 1
fi


# Directory containing .v files
INPUT_DIR=$1

# Output directory for .dot files (you can change this or set it dynamically)
OUTPUT_DIR=$2
mkdir -p "$OUTPUT_DIR"  # Create the output directory if it doesn't exist

# ABC binary path (assuming ABC is installed and accessible)
ABC_BINARY="/usr/bin/abc"  # Modify this path as needed
YOSYS_BINARY="/usr/bin/yosys"


# List of K values for the "if" command
K_VALUES=(4 5 6)

# Loop through all .v files in the input directory
for verilog_file in "$INPUT_DIR"/*.v; do
  if [ -f "$verilog_file" ]; then
    # Extract the filename without extension
    base_filename=$(basename "$verilog_file" .v)

    # Run ABC 3 times, one for each K value
    for K in "${K_VALUES[@]}"; do
      # Define the output .dot file path, including the K value in the name
      dot_file="$OUTPUT_DIR/${base_filename}_K${K}.dot"
      blif_file="$OUTPUT_DIR/${base_filename}_K${K}.blif"


      # Run ABC with the Verilog file, using the "if" command with -K $K, and export to .dot
      $ABC_BINARY -c "read $verilog_file; if -K $K; write_blif $blif_file"
      $YOSYS_BINARY -p "read_blif $blif_file; show -format dot -o $dot_file"

      # Check if ABC successfully created the .dot file
      if [ -f "$dot_file" ]; then
        echo "Successfully created: $dot_file"
      else
        echo "Failed to create: $dot_file"
      fi
    done
  fi
done
