#!/bin/bash

# Diretório contendo os arquivos
input_folder="reports/fpga/yoto_base/rep"

# Verificar se o diretório existe
if [ ! -d "$input_folder" ]; then
    echo "Erro: Diretório '$input_folder' não existe."
    exit 1
fi

# Verificar se o usuário informou a string base
if [ -z "$1" ]; then
    echo "Uso: $0 <nome_base>"
    exit 1
fi

# Base para os nomes dos arquivos
name_base="$1"

# Variável para armazenar os números
result_line=""

# Iterar sobre os arquivos que começam com a string base e têm extensão .rep
for file in "$input_folder"/${name_base}*.rep; do
    # Verificar se o arquivo existe (proteção contra casos sem correspondência)
    if [ -f "$file" ]; then
        # Extrair a linha com "Critical Path:"
        critical_path=$(grep -oP 'Critical Path:\s+\K[0-9.e-]+' "$file")

        if [ ! -z "$critical_path" ]; then
            # Substituir "." por ","
            critical_path=$(echo "$critical_path" | tr '.' ',')

            # Adicionar o valor à linha de resultados
            result_line+="$critical_path\t"
        fi
    fi
done

# Imprimir os resultados removendo o tab extra no final
if [ ! -z "$result_line" ]; then
    echo -e "${result_line%\\t}"
else
    echo "Nenhum arquivo correspondente ou valor encontrado."
fi
