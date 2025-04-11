import sys
import networkx as nx

from src.py.graph.graph import Graph
from src.py.util.util import get_files_list_by_extension, get_project_root, verify_path


def create_statistic(path_dot):
    g = nx.DiGraph(nx.drawing.nx_pydot.read_dot(path_dot))

    cont_input = 0
    cont_output = 0
    cont_no = 0
    cont_edges = 0
    cont_fanout = 0

    for no in list(g.nodes()):
        cont_no += 1
        cont_edges += g.out_degree(no)
        if g.in_degree(no) == 0:
            cont_input += 1
        if g.out_degree(no) == 0:
            cont_output += 1
        cont_fanout += g.out_degree(no)

    print("INPUT      = %d" % (cont_input))
    print("OUTPUT     = %d" % (cont_output))
    print("NODES      = %d" % (cont_no))
    print("EDGES      = %d" % (cont_edges))
    print("FANOUT AVG = %.2f" % (cont_fanout / (cont_no - cont_output)))


def create_net(dot_origin, dot_destiny):
    # print(path_net)
    arq = open(dot_destiny, 'w')

    graph = Graph(dot_origin, "dot")

    graph.output_nodes_str.sort()

    with open(dot_destiny, 'w') as net_file:

        nodes = list(graph.g.nodes())
        nodes.sort()

        k = 6

        if "_k3" in dot_destiny:
            k = 3
        elif "_k4" in dot_destiny:
            k = 4
        elif "_k5" in dot_destiny:
            k = 5

        for no in nodes:
            a = graph.g.in_degree(no)
            b = graph.g.out_degree(no)
            if graph.g.out_degree(no) == 0:
                for pre in list(graph.g.predecessors(no)):
                    net_file.write(f".output out_{no}:{pre}\n")
                    net_file.write(f"pinlist:  {pre}\n\n")

            elif graph.g.in_degree(no) == 0:
                net_file.write(f".input {no}\n")
                net_file.write(f"pinlist:  {no}\n\n")
            else:
                net_file.write(f".clb {no}  # Only LUT used.\n")
                net_file.write('pinlist:')
                i = 0
                for pre in list(graph.g.predecessors(no)):
                    net_file.write(f" {pre}")
                    i += 1
                net_file.write(' open' * (k - i))
                net_file.write(f" {no}")
                net_file.write(' open\n')

                net_file.write(f"subblock: {no}")
                for j in range(i):
                    net_file.write(f" {j}")
                net_file.write(' open' * (k - i))
                net_file.write(f' {k} open\n\n')

    net_file.close()


if __name__ == "__main__":

    root_path = verify_path(get_project_root())
    base_path_origin = root_path + "benchmarks/fpga/dot_TRETS/"
    base_path_destiny = root_path + "benchmarks/fpga/net_TRETS/"

    files = get_files_list_by_extension(base_path_origin, ".dot")
    for file in files:
        create_net(file[0], f"{base_path_destiny}{file[1][:-4]}.net")
