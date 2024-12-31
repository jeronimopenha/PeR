#!/bin/bash

# Verificar se um parâmetro foi passado
if [ -z "$1" ]; then
    echo "Erro: Você deve fornecer o nome base como parâmetro."
    echo "Uso: $0 <nome_base>"
    exit 1
fi

# Nome base passado por parâmetro
nome_base="$1"

# Diretórios contendo os arquivos
input_folder1="reports/fpga/yoto_base/net"
input_folder2="reports/fpga/yoto_base/place"
output_folder="reports/fpga/yoto_base/out"

# Extensões possíveis
ext1=".net"
ext2=".place"

# Iterar sobre os números de k3 a k6
for k in 3 4 5 6; do
# Iterar sobre os índices de 0 a 9
    for y in {0..9}; do
        # Criar o nome completo para os arquivos com extensão .net e .place
        arquivo1="${input_folder1}/${nome_base}_k${k}_${y}${ext1}"
        arquivo2="${input_folder2}/${nome_base}_k${k}_${y}${ext2}"

        # Verificar se os arquivos existem e processá-los
        for arquivo in "$arquivo1"; do
            if [ -f "$arquivo" ]; then
                echo "Processando arquivo: $arquivo"

                # Executar comandos distintos dependendo do _kx
                case "$k" in
                    3)
                        echo "Executando comando para _k3 no arquivo: $arquivo"
                        # Substitua pelo comando real para _k3
                        #comando_para_k3 "$arquivo"
                        bin/vpr $arquivo1  arch/k3-n1.xml $arquivo2 test.route --route_only -nodisp > "${output_folder}/${nome_base}_k${k}_${y}.out"
                        ;;
                    4)
                        echo "Executando comando para _k4 no arquivo: $arquivo"
                        # Substitua pelo comando real para _k4
                        #comando_para_k4 "$arquivo"
                        bin/vpr $arquivo1  arch/k4-n1.xml $arquivo2 test.route --route_only -nodisp > "${output_folder}/${nome_base}_k${k}_${y}.out"
                        ;;
                    5)
                        echo "Executando comando para _k5 no arquivo: $arquivo"
                        # Substitua pelo comando real para _k5
                        #comando_para_k5 "$arquivo"
                        bin/vpr $arquivo1  arch/k5-n1.xml $arquivo2 test.route --route_only -nodisp > "${output_folder}/${nome_base}_k${k}_${y}.out"
                        ;;
                    6)
                        echo "Executando comando para _k6 no arquivo: $arquivo"
                        # Substitua pelo comando real para _k6
                        bin/vpr $arquivo1  arch/k6-n1.xml $arquivo2 test.route --route_only -nodisp > "${output_folder}/${nome_base}_k${k}_${y}.out"
                        ;;
                    *)
                        echo "Aviso: Comando não definido para _k$k"
                        ;;
                esac
            else
                echo "Aviso: Arquivo não encontrado: $arquivo"
            fi
        done
    done
done

echo "Processamento concluído!"
