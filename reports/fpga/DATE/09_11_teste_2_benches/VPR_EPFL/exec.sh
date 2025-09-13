#!/usr/bin/env bash
set -euo pipefail

# ===================== CONFIG =====================
# Caminhos
VPR_BIN="/home/jeronimocosta/Documentos/GIT/vtr_dev/vtr-verilog-to-routing/vpr/vpr"
ARCH_XML="/home/jeronimocosta/Documentos/GIT/vtr_dev/vtr-verilog-to-routing/vtr_flow/arch/lut6_8_f.xml"
#INPUT_DIR="/home/jeronimocosta/Documentos/GIT/PeR/benchmarks/fpga/eval/MINIST_bllif"

# Parâmetros de varredura
SOLVERS=( "qp-hybrid" "lp-b2b" )
TRADEOFFS=( "0" "1" )

# Paralelismo: escolha aqui quantas execuções simultâneas
N_THREADS="${N_THREADS:-8}"
# ==================================================

# Diretório onde o script está (para gravar os *_out.txt aqui)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

INPUT_DIR="$SCRIPT_DIR"


# Expande para caminhos absolutos (útil pois entraremos em diretórios temporários)
abs() { readlink -f "$1"; }
ARCH_XML_ABS="$(abs "$ARCH_XML")"
INPUT_DIR_ABS="$(abs "$INPUT_DIR")"
VPR_BIN_ABS="$(abs "$VPR_BIN")"

# Checagens
if [[ ! -x "$VPR_BIN_ABS" ]]; then
  echo "ERRO: VPR_BIN não encontrado/executável: $VPR_BIN_ABS" >&2
  exit 1
fi
if [[ ! -f "$ARCH_XML_ABS" ]]; then
  echo "ERRO: ARCH_XML não encontrado: $ARCH_XML_ABS" >&2
  exit 1
fi
if [[ ! -d "$INPUT_DIR_ABS" ]]; then
  echo "ERRO: INPUT_DIR não é diretório: $INPUT_DIR_ABS" >&2
  exit 1
fi

echo "Usando:"
echo "  VPR_BIN   = $VPR_BIN_ABS"
echo "  ARCH_XML  = $ARCH_XML_ABS"
echo "  INPUT_DIR = $INPUT_DIR_ABS"
echo "  N_THREADS = $N_THREADS"
echo

# Permite o for pular se não houver .blif
shopt -s nullglob

# Função que executa UMA combinação (em subshell para isolar 'set -e')
run_one() (
  set -euo pipefail

  local blif="$1"
  local solver="$2"
  local tradeoff="$3"

  local base; base="$(basename "$blif" .blif)"
  local t_tag="${tradeoff//./p}"
  local out_file="${solver}_timing${t_tag}_${base}_out.txt"
  local out_path="$SCRIPT_DIR/$out_file"

  # Cria diretório temporário por job
  local tmpdir
  tmpdir="$(mktemp -d -t vprrun.XXXXXX)"
  # Garante limpeza do tmpdir mesmo em erro
  trap 'rm -rf "$tmpdir"' EXIT

  echo "==> Iniciando: solver=${solver} | tradeoff=${tradeoff} | ${base}.blif"
  echo "    Temp dir: $tmpdir"
  (
    cd "$tmpdir"
    # Executa VPR; como passamos caminhos absolutos, funciona em qqr dir
    "$VPR_BIN_ABS" "$ARCH_XML_ABS" "$blif" \
      --analytical_place \
      --ap_analytical_solver "$solver" \
      --ap_timing_tradeoff "$tradeoff" \
      --route
  ) >"$out_path" 2>&1
# | tee "$out_path"

  echo "==> Finalizado: ${out_file}"
  # tmpdir removido pelo trap
)

export -f run_one
export VPR_BIN_ABS ARCH_XML_ABS SCRIPT_DIR

# --- Gera a fila de jobs (NUL-separated) e roda com xargs paralelo ---
# Para cada .blif no INPUT_DIR_ABS, combina com todos os SOLVERS e TRADEOFFS.
# Usamos separador NUL ( -print0 / -0 ) para nomes com espaços.
{
  find "$INPUT_DIR_ABS" -maxdepth 1 -type f -name '*.blif' -print0 | \
  while IFS= read -r -d '' blif; do
    for solver in "${SOLVERS[@]}"; do
      for t in "${TRADEOFFS[@]}"; do
        printf '%s\0%s\0%s\0' "$blif" "$solver" "$t"
      done
    done
  done
} | xargs -0 -n3 -P "$N_THREADS" bash -c 'run_one "$0" "$1" "$2"'

echo "Concluído com sucesso."



# Controle simples de paralelismo usando 'wait -n'
# (requer bash 5+, padrão no Arch/Manjaro)
#active_jobs=0
#pids=()

#for blif in "$INPUT_DIR_ABS"/*.blif; do
#  base="$(basename "$blif")"
#  for solver in "${SOLVERS[@]}"; do
#    for t in "${TRADEOFFS[@]}"; do

#      # Dispara job em background
#      run_one "$blif" "$solver" "$t" &
#      pids+=( "$!" )
#      ((active_jobs++))

#      # Se atingiu o limite, aguarda um terminar
#      if (( active_jobs >= N_THREADS )); then
#        # espera um job terminar
#        if ! wait -n; then
#          echo "Aviso: um job retornou erro." >&2
#        fi
#        ((active_jobs--))
#      fi

#    done
#  done
#done

# Espera todos os jobs restantes
#if ((${#pids[@]})); then
  # 'wait' sem args aguarda todos; reporta erro se algum falhar
#  if ! wait; then
#    echo "Concluído com alguns erros." >&2
#    exit 1
#  fi
#fi

echo "Concluído com sucesso."
