import networkx as nx

from src.py.graph.graph import Graph
from src.py.util.util import Util


def get_directed_graph_parameters(graph: Graph):
    if not graph:
        return None

    parameters = {}

    parameters['nodes'] = graph.g.number_of_nodes()
    parameters['edges'] = graph.g.number_of_edges()

    in_degree = dict(graph.g.in_degree())
    out_degree = dict(graph.g.out_degree())

    g_out = g_in = io = 0

    # biggest degree in and out
    for node in graph.g.nodes:
        inp = len(list(graph.g.predecessors(node)))
        outp = len(list(graph.g.successors(node)))
        if inp > g_in:
            g_in += inp
        if outp > g_out:
            g_out += outp
        if inp == 0 or outp == 0:
            io += 1

    parameters['g_input'] = g_in
    parameters['g_output'] = g_out

    parameters['io'] = io

    utilization = graph.n_nodes / graph.n_cells

    parameters['utilization'] = f"{utilization:.2%}"

    parameters['in_degree_avg'] = sum(in_degree.values()) / len(in_degree)
    parameters['out_degree_avg'] = sum(out_degree.values()) / len(out_degree)

    parameters['is_DAG'] = nx.is_directed_acyclic_graph(graph.g)

    if parameters['is_DAG']:
        # parameters['cycles'] = "None"
        parameters['average_shortest_path_length'] = "None"
        try:
            # parameters['dag_longest_path'] = nx.dag_longest_path(graph)
            parameters['dag_longest_path_length'] = nx.dag_longest_path_length(graph.g)
        except Exception as e:
            # parameters['dag_longest_path'] = "None"
            parameters['dag_longest_path_length'] = "None"
            print(f"Error: {e}")
    else:
        # parameters['dag_longest_path'] = "None"
        parameters['dag_longest_path_length'] = "None"
        try:
            parameters['average_shortest_path_length'] = nx.average_shortest_path_length(graph.g)
        except Exception as e:
            parameters['average_shortest_path_length'] = "None"
            print(f"Error: {e}")
        # parameters['cycles'] = list(nx.simple_cycles(graph))
    # parameters['strongly_connected_components'] = list(nx.strongly_connected_components(graph))
    # parameters['betweenness_centrality'] = nx.betweenness_centrality(graph)
    # parameters['closeness_centrality'] = nx.closeness_centrality(graph)
    parameters['degree_assortativity_coefficient'] = nx.degree_assortativity_coefficient(graph.g, x='in', y='out')

    return parameters


root_path = Util.verify_path(Util.get_project_root())
report_path = "reports/fpga/"
file_name = "complex_parameters"
# files = Util.get_files_list_by_extension(f"{root_path}/benchmarks/fpga/bench_test/", ".dot")
files = Util.get_files_list_by_extension(f"{root_path}/benchmarks/fpga/dot_TRETS/", ".dot")
# files = [["/home/jeronimo/GIT/PeR/benchmarks/fpga/bench_test/xor5_K4.dot", "xor5_K4.dot"]]
# files = [["/home/jeronimo/GIT/PeR/benchmarks/fpga/bench_test/z4ml_K4.dot", "z4ml_K4.dot"]]
parameters = {"dot_name": []}
for file in files:
    graph = Graph(file[0], file[1][:-4])
    parameter = get_directed_graph_parameters(graph)
    parameters["dot_name"].append(file[1][:-4])
    for k in parameter.keys():
        if k not in parameters.keys():
            parameters[k] = []
        parameters[k].append(parameter[k])
Util.save_cvs_data_rows(f"{root_path}{report_path}", file_name, parameters)
