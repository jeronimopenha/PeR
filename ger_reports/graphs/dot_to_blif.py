import networkx as nx

from graphs.util import get_project_root, get_files_list_by_extension


def parse_dot_fast(dot_path):
    G = nx.DiGraph()
    with open(dot_path, 'r') as f:
        for line in f:
            line = line.strip()
            if '->' in line:
                src, dst = line.split('->')
                src = src.strip().strip(';').strip('"')
                dst = dst.strip().strip(';').strip('"')
                G.add_edge(src, dst)
            elif line and not line.startswith('//') and not 'digraph' in line and not '{' in line and not '}' in line:
                node = line.strip().strip(';').strip('"')
                G.add_node(node)
    return G


def dot_to_blif(base_path, dot_path, dot_file: str, model_name="circuit"):
    # Lê o grafo do .dot
    G = parse_dot_fast(dot_path)  # nx.drawing.nx_pydot.read_dot(dot_path)
    G = nx.DiGraph(G)  # garante que é um dígrafo

    # Identifica entradas e saídas
    entradas = [n for n in G.nodes if G.in_degree(n) == 0]
    saidas = [n for n in G.nodes if G.out_degree(n) == 0 and G.in_degree(n) == 1]
    intermediarios = [n for n in G.nodes if n not in entradas and n not in saidas]

    with open(f"{base_path}{dot_file}.blif", 'w') as f:
        f.write(f".model {model_name}\n")
        f.write(".inputs " + " ".join(entradas) + "\n")
        f.write(".outputs " + " ".join(saidas) + "\n\n")

        # Gera LUTs (como .names com tabela genérica)
        for node in intermediarios + saidas:  # saídas também são LUTs com 1 entrada
            inputs = list(G.predecessors(node))
            output = node
            if not inputs:
                continue  # deve ser entrada
            f.write(".names " + " ".join(inputs) + f" {output}\n")
            f.write("1" * len(inputs) + " 1\n")  # tabela verdade genérica
            f.write("\n")

        f.write(".end\n")

    print(f"Arquivo BLIF gerado: {base_path}{dot_file}.blif")


if __name__ == "__main__":
    root_path = get_project_root()
    base_path = f"{root_path}/benchmarks/fpga/bench_test_old/"
    files = get_files_list_by_extension(base_path, ".dot")
    for file in files:
        dot_to_blif(base_path, file[0], file[1][:-5])
