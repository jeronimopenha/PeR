#!/bin/bash

NET_DIR="benchmarks/fpga/net_EPFL"
ARCH="arch/k6-n1.xml"
OUT_DIR="pbs_jobs"

mkdir -p "$OUT_DIR"

for netfile in "$NET_DIR"/*.net; do
    base=$(basename "$netfile" .net)
    pbs_file="$OUT_DIR/$base.pbs"

    cat > "$pbs_file" <<EOF
#!/bin/bash
#PBS -N vpr_$base
#PBS -l nodes=1:ppn=1,mem=500mb
#PBS -o $base.out.txt
#PBS -e $base.err.txt
#PBS -V
#PBS -m ae
#PBS -M jeronimo.penha@ufv.br


cd \$PBS_O_WORKDIR

bin/vpr "$netfile" $ARCH "${base}.place" "${base}.route" --place_only -nodisp > "${base}.out"
EOF
done
