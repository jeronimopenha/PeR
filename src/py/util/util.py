import csv
import os
from math import sqrt
from pathlib import Path
import json
from typing import List, Tuple, Dict
import pandas as pd
from matplotlib import pyplot as plt

from src.py.graph.graph import Graph


def get_project_root() -> str:
    path: Path = Path(__file__).parent.parent.parent.parent
    return str(path)


def write_json(path: str, file_name: str, data: Dict):
    path: str = verify_path(path)
    with open(path + file_name + '.json', 'w', encoding='utf-8') as file:
        json.dump(data, file, ensure_ascii=False, indent=4)


def verify_path(path: str) -> str:
    if path[-1] != '/':
        path = path + '/'
    return path


def get_files_list_by_extension(path: str, file_extension: str) -> List[Tuple[str, str]]:
    files_list_by_extension: List[Tuple[str, str]] = [
        (os.path.join(file_path, file_name), file_name)
        for file_path, _, filenames in os.walk(path)
        for file_name in filenames
        if os.path.splitext(file_name)[1] == file_extension
    ]
    return files_list_by_extension


def read_json(file: str) -> Dict:
    with open(file) as p_file:
        content_dic: Dict = json.load(p_file)
    return content_dic


def save_cvs_data_rows(path: str, file_name: str, data: Dict) -> None:
    with open(f"{path}{file_name}.csv", mode='w', newline='') as file:
        print(data.keys())
        writer: csv.DictWriter = csv.DictWriter(file, fieldnames=data.keys())
        writer.writeheader()
        rows = zip(*data.values())
        for row in rows:
            writer.writerow(dict(zip(data.keys(), row)))
    file.close()


def save_reports(per, path: str, file_name_pref: str, data) -> None:
    # l_data = len(data) // 10
    for i, rpt in enumerate(data):
        write_json(path, f"{per.graph.dot_name}_{file_name_pref}_{data[rpt]['exec_id']}", data[rpt])
        '''if i == 0:
            per.write_dot(path, f"{per.graph.dot_name}_{file_name_pref}_{data[rpt]['exec_id']}_placed.dot",
                          data[rpt]['placement'],
                          data[rpt]['n2c'])'''


def generate_pic():
    root_path: str = get_project_root()
    pics_folder: str = 'reports/fpga/pics/'

    files: list[list] = get_files_list_by_extension(f"{root_path}/reports/fpga/", ".json")
    report: dict = {}
    for file in files:
        rpt: dict = read_json(file[0])
        k = f"{rpt['dot_name']}_{rpt['placer']}_{rpt['edges_algorithm']}_{rpt['exec_id']}"
        if k not in report.keys():
            report[k] = {}
        report[k] = {
            'exec_id': rpt['exec_id'],
            'dot_name': rpt['dot_name'],
            'placer': rpt['placer'],
            'edges_algorithm': rpt['edges_algorithm'],
            'total_cost': rpt['total_cost'],
            'longest_path_cost': rpt['longest_path_cost']
        }

    # Step 1: Convert report dictionary into a pandas DataFrame for easier handling
    df = pd.DataFrame.from_dict(report, orient='index')

    # Step 3: Create scatter plots for each unique graph in 'dot_name'
    graphs = df['dot_name'].unique()

    for graph in graphs:
        # Filter the data for this graph
        graph_data = df[df['dot_name'] == graph]

        # Create a new figure for this graph
        plt.figure(figsize=(10, 6))

        # Group data by placer and edges_algorithm to create series, but include all exec_ids in the same series
        for (placer, edges_algorithm), group in graph_data.groupby(['placer', 'edges_algorithm']):
            # Plot multiple points for each combination of placer and edges_algorithm
            plt.scatter(group['total_cost'], group['longest_path_cost'], label=f'{placer} - {edges_algorithm}',
                        s=50)

        # Customize plot
        plt.title(f'Scatter Plot for {graph}')
        plt.xlabel('Total Cost')
        plt.ylabel('Longest Path Cost')
        plt.legend(title='Placer - Edges Algorithm')
        plt.grid(True)

        plt.savefig(verify_path(root_path) + pics_folder + graph, format='png')
        plt.close()


def generate_vpr_data(graph: Graph, data, net_path, place_path):
    net_name = f"{data['dot_name']}_{data['placer']}_{data['edges_algorithm']}_{data['exec_id']}.net"
    nodes_idx = graph.nodes_to_idx  # data["nodes_idx"]
    with open(net_path + net_name, 'w') as net_file:

        out_nodes = []
        pred_out_nodes = []

        for no in list(graph.g.nodes()):
            if "Level" in no and "title" in no:
                continue
            elif graph.g.out_degree(no) == 0:
                for pre in list(graph.g.predecessors(no)):
                    pre_ = nodes_idx[pre]
                    net_file.write(f".output out:{pre_}\n")
                    net_file.write(f"pinlist:  {pre_}\n\n")
                    out_nodes.append(nodes_idx[no])
                    pred_out_nodes.append(nodes_idx[pre])

            elif graph.g.in_degree(no) == 0:
                net_file.write(f".input {nodes_idx[no]}\n")
                net_file.write(f"pinlist:  {nodes_idx[no]}\n\n")
            else:
                net_file.write(f".clb {nodes_idx[no]}  # Only LUT used.\n")
                net_file.write('pinlist:')
                i = 0
                for pre in list(graph.g.predecessors(no)):
                    net_file.write(f" {nodes_idx[pre]}")
                    i += 1
                net_file.write(' open' * (4 - i))
                net_file.write(f" {nodes_idx[no]}")
                net_file.write(' open\n')

                net_file.write(f"subblock: {nodes_idx[no]}")
                for j in range(i):
                    net_file.write(f" {j}")
                net_file.write(' open' * (4 - i))
                net_file.write(' 4 open\n\n')

    net_file.close()

    place_name = f"{data['dot_name']}_{data['placer']}_{data['edges_algorithm']}_{data['exec_id']}.place"
    placement = data["placement"]
    n2c = data["n2c"]
    output_nodes = graph.output_nodes_idx

    grid_height = grid_width = int(sqrt(len(data["placement"])))

    with open(place_path + place_name, 'w') as place_file:
        place_file.write(f"Netlist file: net/{net_name}  Architecture file: arch/k4-n1.xml\n")
        place_file.write(f"Array size: {grid_width - 2} x {grid_height - 2} logic blocks \n")
        place_file.write("#block name\tX\tY\tsubblk\tblock_number\n")
        place_file.write("#----------\t--\t--\t------\t------------\n")

        counter = 0
        for node, idx in nodes_idx.items():
            cell = n2c[idx]
            if placement[cell] is not None:
                x = cell % grid_width
                y = cell // grid_width
                subblk = 0
                if idx in output_nodes:
                    i = out_nodes.index(idx)
                    place_file.write(f"out:{pred_out_nodes[i]}\t{x}\t{y}\t{subblk}\t#{counter}\n")
                else:
                    place_file.write(f"{idx}\t{x}\t{y}\t{subblk}\t#{counter}\n")
                counter += 1
