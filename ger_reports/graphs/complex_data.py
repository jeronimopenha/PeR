import networkx as nx

from graphs.graph import Graph
from graphs.util import verify_path, get_project_root, get_files_list_by_extension, save_cvs_data_rows


def get_directed_graph_parameters(gph: Graph):
    if not gph:
        return None

    prmts = {}

    prmts['nodes'] = gph.g.number_of_nodes()
    prmts['edges'] = gph.g.number_of_edges()

    in_degree = dict(gph.g.in_degree())
    out_degree = dict(gph.g.out_degree())

    g_out = g_in = io = 0

    # biggest degree in and out
    for node in gph.g.nodes:
        inp = len(list(gph.g.predecessors(node)))
        outp = len(list(gph.g.successors(node)))
        if inp > g_in:
            g_in += inp
        if outp > g_out:
            g_out += outp
        if inp == 0 or outp == 0:
            io += 1

    prmts['g_input'] = g_in
    prmts['g_output'] = g_out

    prmts['io'] = io

    utilization = gph.n_nodes / gph.n_cells

    prmts['utilization'] = f"{utilization:.2%}"

    prmts['in_degree_avg'] = sum(in_degree.values()) / len(in_degree)
    prmts['out_degree_avg'] = sum(out_degree.values()) / len(out_degree)

    prmts['is_DAG'] = nx.is_directed_acyclic_graph(gph.g)

    if prmts['is_DAG']:
        # parameters['cycles'] = "None"
        prmts['average_shortest_path_length'] = "None"
        try:
            # parameters['dag_longest_path'] = nx.dag_longest_path(graph)
            prmts['dag_longest_path_length'] = nx.dag_longest_path_length(gph.g)
        except Exception as e:
            # parameters['dag_longest_path'] = "None"
            prmts['dag_longest_path_length'] = "None"
            print(f"Error: {e}")
    else:
        # parameters['dag_longest_path'] = "None"
        prmts['dag_longest_path_length'] = "None"
        try:
            prmts['average_shortest_path_length'] = nx.average_shortest_path_length(gph.g)
        except Exception as e:
            prmts['average_shortest_path_length'] = "None"
            print(f"Error: {e}")
        # parameters['cycles'] = list(nx.simple_cycles(graph))
    # parameters['strongly_connected_components'] = list(nx.strongly_connected_components(graph))
    # parameters['betweenness_centrality'] = nx.betweenness_centrality(graph)
    # parameters['closeness_centrality'] = nx.closeness_centrality(graph)
    prmts['degree_assortativity_coefficient'] = nx.degree_assortativity_coefficient(gph.g, x='in', y='out')

    return prmts

if __name__ == "__main__":

    root_path = verify_path(get_project_root())
    base_path_origin = root_path + "benchmarks/fpga/eval/EPFL/"
    base_path_destiny = root_path + "reports/fpga/"
    file_name = "complex_parameters"

    files = get_files_list_by_extension(base_path_origin, ".dot")
    parameters = {"dot_name": []}
    for file in files:
        graph = Graph(file[0], file[1][:-4])
        parameter = get_directed_graph_parameters(graph)
        parameters["dot_name"].append(file[1][:-4])
        for k in parameter.keys():
            if k not in parameters.keys():
                parameters[k] = []
            parameters[k].append(parameter[k])
    save_cvs_data_rows(base_path_destiny, file_name, parameters)
