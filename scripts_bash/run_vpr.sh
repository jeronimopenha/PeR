#!/bin/bash

# Verificar se os parâmetros obrigatórios foram passados
if [ -z "$1" ] || [ -z "$2" ]; then
    echo "Erro: Você deve fornecer o nome base e o nome da pasta base como parâmetros."
    echo "Uso: $0 <nome_base> <pasta_base> [num_processos]"
    exit 1
fi

# Parâmetros
nome_base="$1"
pasta_base="$2"
num_processos=${3:-4}

# Diretórios com base no nome da pasta
input_folder1="reports/fpga/EPFL/${pasta_base}/net"
input_folder2="reports/fpga/EPFL/${pasta_base}/place"
output_folder="reports/fpga/EPFL/${pasta_base}/rep"

# Extensões
ext1=".net"
ext2=".place"

# Criar pasta de saída se não existir
mkdir -p "$output_folder"

# Função para processar os arquivos
processar_arquivo() {
    local k=$1
    local y=$2
    local arquivo1="${input_folder1}/${nome_base}_k${k}_${y}${ext1}"
    local arquivo2="${input_folder2}/${nome_base}_k${k}_${y}${ext2}"
    local output_file="${output_folder}/${nome_base}_k${k}_${y}.rep"

    if [ -f "$arquivo1" ] && [ -f "$arquivo2" ]; then
        echo "Processando arquivos: $arquivo1 e $arquivo2"

        case "$k" in
            3) bin/vpr "$arquivo1" arch/k3-n1.xml "$arquivo2" test.route --route_only -nodisp > "$output_file" ;;
            4) bin/vpr "$arquivo1" arch/k4-n1.xml "$arquivo2" test.route --route_only -nodisp > "$output_file" ;;
            5) bin/vpr "$arquivo1" arch/k5-n1.xml "$arquivo2" test.route --route_only -nodisp > "$output_file" ;;
            6) bin/vpr "$arquivo1" arch/k6-n1.xml "$arquivo2" test.route --route_only -nodisp > "$output_file" ;;
            *) echo "Aviso: Comando não definido para _k$k" ;;
        esac
    else
        echo "Aviso: Arquivo não encontrado: $arquivo1 ou $arquivo2"
    fi
}

# Exportar função e variáveis para subprocessos
export -f processar_arquivo
export input_folder1 input_folder2 output_folder nome_base ext1 ext2

# Gerar lista de tarefas
tasks_file=$(mktemp)
for k in 3 4 5 6; do
    for y in {0..9}; do
        echo "$k $y" >> "$tasks_file"
    done
done

# Executar em paralelo
cat "$tasks_file" | xargs -n 2 -P "$num_processos" bash -c 'processar_arquivo "$@"' _

# Limpar
rm -f "$tasks_file"

echo "Processamento concluído!"
