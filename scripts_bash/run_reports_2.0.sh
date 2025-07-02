
#!/bin/bash

# Lista de execuções (nome_base, pasta_base, [num_processos])
configs=(
#"adder yott"
#"bar yott"
#"cavlc yott"
#"ctrl yott"
#"dec yott"
#"i2c yott"
#"int2float yott"
#"priority yott"
#"router yott"
#"sin yott"

#"adder yoto_df"
#"bar yoto_df"
#"cavlc yoto_df"
#"ctrl yoto_df"
#"dec yoto_df"
#"i2c yoto_df"
#"int2float yoto_df"
#"priority yoto_df"
#"router yoto_df"
#"sin yoto_df"
#
#"adder yoto_df_prio"
#"bar yoto_df_prio"
#"cavlc yoto_df_prio"
#"ctrl yoto_df_prio"
#"dec yoto_df_prio"
#"i2c yoto_df_prio"
#"int2float yoto_df_prio"
#"priority yoto_df_prio"
#"router yoto_df_prio"
#"sin yoto_df_prio"
#
"adder sa"
"bar sa"
"cavlc sa"
"ctrl sa"
"dec sa"
"i2c sa"
"int2float sa"
"priority sa"
"router sa"
"sin sa"
#
#"max sa"
#"max yott"
#"max yoto_df"
#"max yoto_df_prio"
#
#"voter sa"
#"voter yoto_df_prio"
#"voter yoto_df"
#"voter yott"
#
#"square sa"
#"square yoto_df_prio"
#"square yoto_df"
#"square yott"
#
#"sqrt yott"
#"sqrt yoto_df"
#"sqrt sa"
#"sqrt yoto_df_prio"
#
#"multiplier yott"
#"multiplier yoto_df"
#"multiplier yoto_df_prio"
#"multiplier sa"
#
#"log2 sa"
#"log2 yoto_df_prio"
#"log2 yott"
#"log2 yoto_df"
#
#"div yoto_df_prio"
#"div yoto_df"
#"div yott"
#"div sa"

)


MAX_PROC=15
task_file=$(mktemp)

for config in "${configs[@]}"; do
    read -r nome_base pasta_base <<< "$config"
    for k in 6; do 
    #3 4 5 
        for y in {0..9}; do
            # Ignorar _0.* e _1.*
            #if [[ "$y" -eq 0 || "$y" -eq 1 ]]; then
            #    continue
            #fi
            echo "$nome_base $pasta_base $k $y" >> "$task_file"
        done
    done
done

# Executa tudo em paralelo usando xargs
cat "$task_file" | xargs -n 4 -P "$MAX_PROC" bash ./run_vpr_2.0.sh

rm -f "$task_file"
