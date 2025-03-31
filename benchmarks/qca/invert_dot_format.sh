#!/bin/bash

if [ -z "$1" ] || [ -z "$2" ]; then
  echo "Uso: $0 <pasta-com-dot> <pasta-de-saida>"
  exit 1
fi

PASTA_IN="$1"
PASTA_OUT="$2"

# Cria a pasta de saída se não existir
mkdir -p "$PASTA_OUT"

for arquivo_dot in "$PASTA_IN"/*.dot; do
  nome_base=$(basename "$arquivo_dot" .dot)
  saida="${PASTA_OUT}/${nome_base}_inverted.dot"
  mapa="${PASTA_OUT}/${nome_base}.map"

  echo "🔁 Processando $nome_base.dot..."

  declare -A map  # Mapeia nomes originais para inteiros
  id=0
  arestas=()

  # Lê linhas válidas com arestas sólidas
  while read -r linha; do
    [[ "$linha" == *"style = solid"* ]] || continue
    no1=$(echo "$linha" | awk '{print $1}')
    no2=$(echo "$linha" | awk '{print $3}')
    no2="${no2//;/}"

    # Atribui IDs únicos
    if [ -z "${map[$no1]}" ]; then
      map[$no1]=$id
      ((id++))
    fi
    if [ -z "${map[$no2]}" ]; then
      map[$no2]=$id
      ((id++))
    fi

    arestas+=("${map[$no2]} -> ${map[$no1]};")
  done < "$arquivo_dot"

  # Escreve .dot invertido
  echo "digraph network {" > "$saida"
  for a in "${arestas[@]}"; do
    echo "  $a" >> "$saida"
  done
  echo "}" >> "$saida"

  # Escreve mapeamento reverso
  > "$mapa"
  for nome in "${!map[@]}"; do
    echo "${map[$nome]} $nome" >> "$mapa"
  done

  echo "✅ Gerado: $saida"
  echo "📄 Mapeamento: $mapa"

  unset map
  unset arestas
done
