#!/bin/bash

if [ $# -ne 2 ]; then
  echo "Uso: $0 <pasta_com_jsons> <arquivo_saida.csv>"
  exit 1
fi

PASTA="$1"
ARQUIVO_SAIDA="$2"

echo "dotName,nExtraL,success,wrongEdges,area,usedAreaPercentage,nodes,wires" > "$ARQUIVO_SAIDA"

for json in "$PASTA"/*.json; do
  dotName=$(jq -r '.dotName' "$json")
  success=$(jq -r '.success' "$json")
  wrongEdges=$(jq -r '.wrongEdges' "$json")
  area=$(jq -r '.area' "$json")
  usedAreaPercentage=$(jq -r '.usedAreaPercentage' "$json")
  nodes=$(jq -r '.nNodes' "$json")
  wires=$(jq -r '.wires' "$json")

  filename=$(basename "$json")
  nExtraL=$(echo "$filename" | sed -E 's/.*_([0-9]+)\.json/\1/')

  echo "$dotName,$nExtraL,$success,$wrongEdges,$area,$usedAreaPercentage,$nodes,$wires" >> "$ARQUIVO_SAIDA"
done
