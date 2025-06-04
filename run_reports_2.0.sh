
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

"adder yoto_df"
"bar yoto_df"
"cavlc yoto_df"
"ctrl yoto_df"
"dec yoto_df"
"i2c yoto_df"
"int2float yoto_df"
"priority yoto_df"
"router yoto_df"
"sin yoto_df"

"adder yoto_df_prio"
"bar yoto_df_prio"
"cavlc yoto_df_prio"
"ctrl yoto_df_prio"
"dec yoto_df_prio"
"i2c yoto_df_prio"
"int2float yoto_df_prio"
"priority yoto_df_prio"
"router yoto_df_prio"
"sin yoto_df_prio"

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

"max sa"
"max yott"
"max yoto_df"
"max yoto_df_prio"

"voter sa"
"voter yoto_df_prio"
"voter yoto_df"
"voter yott"

"square sa"
"square yoto_df_prio"
"square yoto_df"
"square yott"

"sqrt yott"
"sqrt yoto_df"
"sqrt sa"
"sqrt yoto_df_prio"

"multiplier yott"
"multiplierv"
"multiplier yoto_df_prio"
"multiplier sa"

"log2 sa"
"log2 yoto_df_prio"
"log2 yott"
"log2 yoto_df"

"div yoto_df_prio"
"div yoto_df"
"div yott"
"div sa"

#        "too-lrg yott 16"
#        "alu4 yott 16"
#        "apex2 yott 16"
#        "apex4 yott 16"
#        "ex5p yott 16"
#        "misex3 yott 16"

#        "too-lrg yott_d 16"
#        "alu4 yott_d 16"
#        "apex2 yott_d 16"
#        "apex4 yott_d 16"
#        "ex5p yott_d 16"
#        "misex3 yott_d 16"

#        "too-lrg yott_io 16"
#        "alu4 yott_io 16"
#        "apex2 yott_io 16"
#        "apex4 yott_io 16"
#        "ex5p yott_io 16"
#        "misex3 yott_io 16"

#        "too-lrg yoto_df 16"
#        "alu4 yoto_df 16"
#        "apex2 yoto_df 16"
#        "apex4 yoto_df 16"
#        "ex5p yoto_df 16"
#        "misex3 yoto_df 16"

#        "too-lrg yoto_df_prio 16"
#        "alu4 yoto_df_prio 16"
#        "apex2 yoto_df_prio 16"
#        "apex4 yoto_df_prio 16"
#        "ex5p yoto_df_prio 16"
#        "misex3 yoto_df_prio 16"

#         "too-lrg sa 16"
#         "alu4 sa 16"
#         "apex2 sa 16"
#         "apex4 sa 16"
#         "ex5p sa 16"
#         "misex3 sa 16"

#       "ex1010 yott 16"
#       "ex1010 yoto_df_io_placed 16"
#        "ex1010 yoto_df_prio 16"
#        "ex1010 sa 16"


)


MAX_PROC=18
task_file=$(mktemp)

for config in "${configs[@]}"; do
    read -r nome_base pasta_base <<< "$config"
    for k in 3 4 5 6; do
        for y in {0..9}; do
            # Ignorar _0.* e _1.*
            if [[ "$y" -eq 0 || "$y" -eq 1 ]]; then
                continue
            fi
            echo "$nome_base $pasta_base $k $y" >> "$task_file"
        done
    done
done

# Executa tudo em paralelo usando xargs
cat "$task_file" | xargs -n 4 -P "$MAX_PROC" bash ./run_vpr_2.0.sh

rm -f "$task_file"
