#!/bin/bash

# Verifica se a pasta base foi passada
if [ -z "$1" ]; then
    echo "Erro: informe a pasta base que contém net/place/rep (ex: yott)"
    echo "Uso: $0 <pasta_base>"
    exit 1
fi

pasta_base="$1"
net_folder="reports/fpga/EPFL/$pasta_base/net"
place_folder="reports/fpga/EPFL/$pasta_base/place"
rep_folder="reports/fpga/EPFL/$pasta_base/rep"
pbs_dir="reports/fpga/EPFL/$pasta_base/pbs"

mkdir -p "$rep_folder" "$pbs_dir"

for net_file in "$net_folder"/*.net; do
    base=$(basename "$net_file" .net)
    place_file="$place_folder/$base.place"
    route_file="$rep_folder/$base.route"
    out_file="$rep_folder/$base.out"
    pbs_file="$pbs_dir/$base.pbs"

    if [[ -f "$place_file" ]]; then
        cat > "$pbs_file" <<EOF
#!/bin/bash
#PBS -N vpr_${pasta_base}_$base
#PBS -l nodes=1:ppn=1,mem=10240mb
#PBS -m ae
#PBS -M jeronimo.penha@ufv.br
#PBS -o $base.pbs.out
#PBS -e $base.pbs.err
#PBS -V

cd \$PBS_O_WORKDIR

bin/vpr "$net_file" arch/k6-n1.xml "$place_file" "$route_file" --route_only -nodisp > "$out_file"
EOF
        echo "✔️ PBS gerado: $pbs_file"
    else
        echo "⚠️ Arquivo .place não encontrado para $base"
    fi

done
