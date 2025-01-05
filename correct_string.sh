#!/bin/bash

# Diretório contendo os arquivos
input_folder="reports/fpga/yoto_base_zz/place"

# Verificar se o diretório existe
if [ ! -d "$input_folder" ]; then
    echo "Erro: O diretório '$input_folder' não existe."
    exit 1
fi

# Iterar por todos os arquivos na pasta
for file in "$input_folder"/*; do
    # Verificar se é um arquivo regular
    if [ -f "$file" ]; then
        echo "Processando arquivo: $file"

        # Substituir "yoto_base" por "yoto_base_p" no arquivo
        sed -i 's/yoto_base/yoto_base_zz/g' "$file"
    fi
done

echo "Substituição concluída!"
