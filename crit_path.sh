#!/bin/bash

# Diretório contendo os arquivos .out
input_folder="reports/fpga/yoto_base/out/"

# Verificar se o diretório existe
if [ ! -d "$input_folder" ]; then
    echo "Erro: O diretório $input_folder não foi encontrado."
    exit 1
fi

# Iterar sobre todos os arquivos .out no diretório
for arquivo in "${input_folder}"/*.out; do
    # Verificar se existem arquivos .out
    if [ -f "$arquivo" ]; then
        # Buscar pela linha com a frase "Critical Path: XXXX"
        linha=$(grep -E "Critical Path: [0-9]+\.[0-9]+e[+-][0-9]+" "$arquivo")

        # Verificar se encontrou a linha
        if [ -n "$linha" ]; then
            # Extrair o número da linha
            numero=$(echo "$linha" | grep -oE "[0-9]+\.[0-9]+e[+-][0-9]+")
            # Exibir o nome do arquivo e o número
            echo "Arquivo: $arquivo | Critical Path: $numero"
        else
            echo "Aviso: Nenhuma linha com 'Critical Path' encontrada em $arquivo"
        fi
    else
        echo "Aviso: Nenhum arquivo .out encontrado no diretório."
    fi
done

echo "Processamento concluído!"
