#!/bin/bash

if [ -z "$1" ] || [ -z "$2" ]; then
    echo "Uso: $0 <subpasta> <nome_base>"
    exit 1
fi

subfolder="$1"
name_base="$2"
input_folder="reports/fpga/$subfolder/rep"

if [ ! -d "$input_folder" ]; then
    echo "Erro: Diretório '$input_folder' não existe."
    exit 1
fi

values=()

for file in "$input_folder"/${name_base}*.rep; do
    if [ -f "$file" ]; then
        critical_path=$(grep -oP 'Critical Path:\s+\K[0-9.e-]+' "$file")
        if [ ! -z "$critical_path" ]; then
            critical_path=$(echo "$critical_path" | tr '.' ',')
            values+=("$critical_path")
        fi
    fi
done

if [ ${#values[@]} -gt 0 ]; then
    for ((i = 0; i < ${#values[@]}; i++)); do
        printf "%s" "${values[i]}"
        # Se não for o último da linha, imprime ";"
        if (( (i + 1) % 10 != 0 && i != ${#values[@]} - 1 )); then
            printf ";"
        fi
        # Se for o décimo, imprime quebra de linha
        if (( (i + 1) % 10 == 0 )); then
            printf "\n"
        fi
    done
    # Se o último grupo tiver menos de 10, quebra linha final
    if (( ${#values[@]} % 10 != 0 )); then
        printf "\n"
    fi
else
    echo "Nenhum arquivo correspondente ou valor encontrado."
fi
