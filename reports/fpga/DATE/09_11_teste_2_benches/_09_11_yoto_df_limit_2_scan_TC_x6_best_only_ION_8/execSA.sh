#!/usr/bin/env bash
set -euo pipefail

# =============== CONFIG =================
# Caminhos fixos do VPR e do XML
VPR_BIN="/home/jeronimocosta/Documentos/GIT/vtr_dev/vtr-verilog-to-routing/vpr/vpr"
ARCH_XML="/home/jeronimocosta/Documentos/GIT/vtr_dev/vtr-verilog-to-routing/vtr_flow/arch/lut6_8.xml"

# Algoritmos de placement a executar para cada .place
ALGORITHMS=( "bounding_box" "criticality_timing" )

# Paralelismo (pode sobrescrever: N_THREADS=8 ./exec_place_parallel.sh)
N_THREADS="${N_THREADS:-16}"
# ========================================

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PLACE_DIR="$SCRIPT_DIR/place"

# Caminho absoluto com fallback (útil p/ FUSE/GVFS)
abs() { readlink -f "$1" 2>/dev/null || echo "$1"; }
VPR_BIN_ABS="$(abs "$VPR_BIN")"
ARCH_XML_ABS="$(abs "$ARCH_XML")"
PLACE_DIR_ABS="$(abs "$PLACE_DIR")"

# Checagens
[[ -x "$VPR_BIN_ABS" ]] || { echo "ERRO: VPR_BIN não encontrado/executável: $VPR_BIN_ABS" >&2; exit 1; }
[[ -f "$ARCH_XML_ABS" ]] || { echo "ERRO: ARCH_XML não encontrado: $ARCH_XML_ABS" >&2; exit 1; }
[[ -d "$PLACE_DIR_ABS" ]] || { echo "ERRO: PLACE_DIR não é diretório: $PLACE_DIR_ABS" >&2; exit 1; }

echo "Usando:"
echo "  VPR_BIN   = $VPR_BIN_ABS"
echo "  ARCH_XML  = $ARCH_XML_ABS"
echo "  PLACE_DIR = $PLACE_DIR_ABS"
echo "  N_THREADS = $N_THREADS"
echo

shopt -s nullglob

# --- Função de 1 job: (place_path, algo, blif_path, out_file_base) ---
run_one() {
  set -euo pipefail
  local place_path="$1" algo="$2" blif_path="$3" out_base="$4"

  local out_path="$SCRIPT_DIR/${algo}_${out_base}_out.txt"
  local tmpdir
  tmpdir="$(mktemp -d -t vprrun.XXXXXX)"
  trap 'rm -rf "$tmpdir"' RETURN

  echo "==> Iniciando: algo=${algo} | place=$(basename "$place_path") | blif=$(basename "$blif_path")"
  echo "    Temp dir: $tmpdir"

  (
    cd "$tmpdir"
    "$VPR_BIN_ABS" "$ARCH_XML_ABS" "$blif_path" \
      --pack \
      --route \
      --verify_file_digests off \
      --place_file "$place_path" \
      --place_algorithm "$algo" \
      --disp off
  ) >"$out_path" 2>&1

  echo "==> Finalizado: $(basename "$out_path")"
}

export -f run_one
export VPR_BIN_ABS ARCH_XML_ABS SCRIPT_DIR

# --- Monta a fila de jobs e executa com xargs paralelo ---
# Para cada place/NAME_k6_N.place:
#   - BLIF = ./NAME_k6.blif  (mesmo diretório do script)
#   - OUT base = NAME_k6_N   (basename do .place sem .place)
{
  find "$PLACE_DIR_ABS" -maxdepth 1 -type f -name '*.place' -print0 | \
  while IFS= read -r -d '' pfile; do
    pbase="$(basename "$pfile" .place)"        # ex.: voter_k6_0
    # extrai o "NAME_k6" removendo o sufixo "_<dígito>" (N entre 0..9)
    bench="${pbase%_[0-9]}"
    blif="$SCRIPT_DIR/../${bench}.blif"

    if [[ ! -f "$blif" ]]; then
      echo "AVISO: BLIF não encontrado p/ $pbase → esperado: $(basename "$blif") — pulando." >&2
      continue
    fi

    for algo in "${ALGORITHMS[@]}"; do
      # Args NUL-separated: place, algo, blif, out_base
      printf '%s\0%s\0%s\0%s\0' "$pfile" "$algo" "$blif" "$pbase"
    done
  done
} | xargs -0 -n4 -P "$N_THREADS" bash -c 'run_one "$0" "$1" "$2" "$3"'

echo "Concluído com sucesso."

