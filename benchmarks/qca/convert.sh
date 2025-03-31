#!/bin/bash

# Verifica se a pasta foi passada
if [ -z "$1" ]; then
  echo "Uso: $0 <pasta-com-verilog>"
  exit 1
fi

PASTA="$1"

# Percorre todos os arquivos .v da pasta
for arquivo_v in "$PASTA"/*.v; do
  # Extrai o nome base sem extensão
  nome_base=$(basename "$arquivo_v" .v)

  echo "🔧 Processando $nome_base..."

  # Gera o arquivo .blif com Yosys
  yosys -p "
    read_verilog \"$arquivo_v\";
    hierarchy -check;
    proc;
    flatten;
    techmap;
    opt;
    write_blif \"$PASTA/$nome_base.blif\";
  "

  # Gera o .dot com ABC
  abc -c "read_blif \"$PASTA/$nome_base.blif\";  write_dot \"$PASTA/$nome_base.dot\""

  echo "✅ Gerado: $nome_base.blif e $nome_base.dot"
done
