#!/bin/bash

if [ -z "$1" ] || [ -z "$2" ]; then
    echo "Uso: $0 <pasta> <prefixo>"
    exit 1
fi

folder="$1"
prefix="$2"

if [ ! -d "$folder" ]; then
    echo "Erro: pasta '$folder' não existe."
    exit 1
fi

declare -A total_tries
declare -A total_misses
declare -A count

# Processar os arquivos
for file in "$folder"/${prefix}_k[3-6]_[0-9].json; do
    [[ -f "$file" ]] || continue

    group=$(echo "$file" | grep -oP 'k[3-6]')

    tries=$(jq '.tries' "$file")
    misses=$(jq '.cacheMisses' "$file")

    if [[ "$tries" =~ ^[0-9]+$ ]] && [[ "$misses" =~ ^[0-9]+$ ]]; then
        total_tries[$group]=$(( total_tries[$group] + tries ))
        total_misses[$group]=$(( total_misses[$group] + misses ))
        count[$group]=$(( count[$group] + 1 ))
    fi
done

# Cabeçalho
echo "k3;;k4;;k5;;k6;;"
echo "Tries;Misses;Tries;Misses;Tries;Misses;Tries;Misses"

# Linha de dados
for k in k3 k4 k5 k6; do
    if [[ ${count[$k]} -gt 0 ]]; then
        avg_tries=$(( (total_tries[$k] + count[$k] / 2) / count[$k] ))
        avg_misses=$(( (total_misses[$k] + count[$k] / 2) / count[$k] ))
        printf "%d;%d;" "$avg_tries" "$avg_misses"
    else
        # Sem dados — coloca vazio
        printf ";%;"
    fi
done

printf "\n"
