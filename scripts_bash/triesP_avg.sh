#!/bin/bash

export LC_ALL=C


# Verifica se foi passado um argumento
if [ $# -ne 1 ]; then
    echo "Uso: $0 <pasta_com_arquivos_json>"
    exit 1
fi

PASTA="$1"
ARQUIVO_SAIDA="media_triesP.csv"

cd "$PASTA" || { echo "Pasta não encontrada: $PASTA"; exit 1; }

# Limpa ou cria o arquivo de saída
#echo -n > "$ARQUIVO_SAIDA"

# Loop pelos valores de Y na ordem desejada
for Y in 3 4 5 6; do
    kY="k$Y"

    # Encontra os prefixos únicos para esse kY
    for prefixo in $(ls *_${kY}_*.json 2>/dev/null | sed -E "s/(.*)_${kY}_[0-9]+\.json/\1/" | sort -u); do
        total=0
        count=0

        for arq in "${prefixo}_${kY}"_*.json; do
            if [ -f "$arq" ]; then
                tries=$(jq -r '.triesP' "$arq")
                if [[ "$tries" =~ ^[0-9]+$ ]]; then
                    total=$((total + tries))
                    count=$((count + 1))
                fi
            fi
        done

        if [ $count -gt 0 ]; then
            #media=$(echo "$total / $count" | bc -l)
            #media=$(LC_NUMERIC=C echo "$total / $count" | bc -l)
            media=$(echo "scale=2; $total / $count" | LC_NUMERIC=C bc)
            media_arredondada=$(printf "%.0f" "$media")
            echo "${prefixo}_${kY},$media_arredondada"
            #| tee -a "$ARQUIVO_SAIDA"
        fi
    done
done
