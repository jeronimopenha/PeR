#!/bin/bash

nome_base="$1"
pasta_base="$2"
k="$3"
y="$4"

# Caminhos
input_folder1="reports/fpga/EPFL/${pasta_base}/net"
input_folder2="reports/fpga/EPFL/${pasta_base}/place"
output_folder="reports/fpga/EPFL/${pasta_base}/rep"

# Arquivos
arquivo1="${input_folder1}/${nome_base}_k${k}_${y}.net"
arquivo2="${input_folder2}/${nome_base}_k${k}_${y}.place"
arquivo3="${input_folder2}/${nome_base}_k${k}_${y}.route"
output_file="${output_folder}/${nome_base}_k${k}_${y}.rep"

# Cria pasta de saída se necessário
mkdir -p "$output_folder"

# Verifica se arquivos existem
if [ -f "$arquivo1" ] && [ -f "$arquivo2" ]; then
    echo "Processando: $arquivo1"

    case "$k" in
        3) bin/vpr "$arquivo1" arch/k3-n1.xml "$arquivo2" "$arquivo3" --route_only -nodisp > "$output_file" ;;
        4) bin/vpr "$arquivo1" arch/k4-n1.xml "$arquivo2" "$arquivo3" --route_only -nodisp > "$output_file" ;;
        5) bin/vpr "$arquivo1" arch/k5-n1.xml "$arquivo2" "$arquivo3" --route_only -nodisp > "$output_file" ;;
        6) bin/vpr "$arquivo1" arch/k6-n1.xml "$arquivo2" "$arquivo3" --route_only -nodisp > "$output_file" ;;
        *) echo "Aviso: k=$k não suportado" ;;
    esac
else
    echo "Aviso: Arquivo não encontrado: $arquivo1 ou $arquivo2"
fi
