#!/usr/bin/env bash
set -euo pipefail
shopt -s nullglob

# Diretório a varrer: padrão = pasta atual; pode passar como 1º argumento
DIR="${1:-.}"
OUT_CSV="resultado_vpr.csv"

# Helper: pega a primeira linha que bate no padrão (ou vazio), sem matar o script
first_match() {
  # $1 = regex (grep -E), $2 = arquivo
  grep -E -m1 "$1" "$2" 2>/dev/null || true
}

# Cabeçalho
echo "bench;WIRES;caminho crítico;channels;Tplacement" > "$OUT_CSV"

# Varre recursivamente; remova -maxdepth 1 se quiser descer subpastas
while IFS= read -r -d '' f; do
  fname="$(basename "$f")"

  # bench: nome do arquivo sem _out.txt; remove prefixo "<solver>_timing<tag>_" se houver
  bench="${fname%_out.txt}"
  bench="$(sed -E 's/^[^_]+_timing[^_]+_//' <<<"$bench")"

  # 1) Tplacement: "# Placement took X seconds" OU "# Analytical Placement took X seconds"
  line_tp="$(first_match '# (Analytical Placement|Placement) took [0-9.]+ seconds' "$f")"
  tplacement="$(sed -nE 's/.* took ([0-9.]+) seconds.*/\1/p' <<<"$line_tp" | sed 's/\./,/' )"

  # 2) WIRES: "Total wirelength: NNNNN, average net length: ..."
  line_w="$(first_match 'Total wirelength:[[:space:]]*[0-9]+' "$f")"
  wires="$(sed -nE 's/.*Total wirelength:[[:space:]]*([0-9]+).*/\1/p' <<<"$line_w")"

  # 3) caminho crítico: "Final critical path delay (least slack): X ns, Fmax: ..."
  line_cc="$(first_match 'Final critical path delay \(least slack\):[[:space:]]*[0-9.]+' "$f")"
  crit="$(sed -nE 's/.*least slack\):[[:space:]]*([0-9.]+)[[:space:]]*ns.*/\1/p' <<<"$line_cc" | sed 's/\./,/' )"

  # 5) channels: "Best routing used a channel width factor of NN."
  line_ch="$(first_match 'Best routing used a channel width factor of[[:space:]]*[0-9]+' "$f")"
  channels="$(sed -nE 's/.*factor of[[:space:]]*([0-9]+).*/\1/p' <<<"$line_ch")"

  # Linha CSV
  echo "${bench};${wires};${crit};${channels};${tplacement}" >> "$OUT_CSV"

done < <(find "$DIR" -maxdepth 1 -type f -name '*_out.txt' -print0)

echo "OK: gerado ${OUT_CSV} em $(realpath "$OUT_CSV")"
