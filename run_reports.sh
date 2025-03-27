
#!/bin/bash

# Lista de execuções (nome_base, pasta_base, [num_processos])
configs=(
        "too-lrg yott 16"
        "alu4 yott 16"
        "apex2 yott 16"
        "apex4 yott 16"
        "ex5p yott 16"
        "misex3 yott 16"
        "ex1010 yott 16"
)

# Caminho para o script principal
SCRIPT="./run_vpr.sh"

# Loop pelas configurações
for config in "${configs[@]}"; do
    echo "Executando: $SCRIPT $config"
    $SCRIPT $config
    echo "---------------------------"
done

echo "Todas as execuções foram concluídas."
