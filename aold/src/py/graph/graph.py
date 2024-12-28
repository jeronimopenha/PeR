# cython: language_level=3


import pygraphviz as pgv
import networkx as nx
import random
from math import ceil, sqrt
from collections import defaultdict
from typing import List, Tuple, Dict


class Graph:

    def __init__(self, dot_path: str, dot_name: str):
        self.dot_path: str = dot_path
        self.dot_name: str = dot_name
        self.gv: pgv.AGraph = pgv.AGraph(self.dot_path, strict=False, directed=True)
        self.g: nx.DiGraph = nx.DiGraph(self.gv)
        self.nodes_str: List[str] = []  # list(self.g.nodes)
        self.edges_str: List[Tuple[str, str]] = []  # list(self.g.edges)
        self.n_edges: int = 0  # len(self.edges_str)
        self.nodes_to_idx: Dict[str, int] = {}
        self.idx_to_nodes: Dict[int, str] = {}
        self.neighbors_str: Dict[int, List[str]] = defaultdict(list)
        self.neighbors_idx: Dict[int, List[int]] = defaultdict(list)
        self.dir_neighbors_str: Dict[int, List[int]] = defaultdict(list)
        self.dir_neighbors_idx: Dict[int, List[int]] = defaultdict(list)
        self.input_nodes_str: list = []
        self.output_nodes_str: list = []
        self.n_cells: int = 0
        self.n_cells_sqrt: int = 0
        # self.clear_graph()
        self.get_nodes_vars()
        self.get_edges_vars()
        self.calc_cells_qty()
        self.n_nodes: int = len(self.nodes_str)
        self.input_nodes_idx: List[int] = self.get_nodes_idx(self.input_nodes_str)
        self.output_nodes_idx = self.get_nodes_idx(self.output_nodes_str)
        self.edges_idx = self.get_edges_idx(self.edges_str)
        self.longest_path = []
        self.longest_path_nodes = []
        self.longest_path_and_length()

    def clear_graph(self):
        g = self.g.copy()
        for edge in self.g.edges:
            if "invis" in self.g.edges[edge]["style"]:
                g.remove_edge(edge[0], edge[1])
            else:
                # correct the edges
                g.add_edge(edge[1], edge[0])
                g.remove_edge(edge[0], edge[1])

        self.g = g

    def get_nodes_vars(self):
        n_list = list(self.g.nodes)
        nodes_counter: int = 0
        for i, node in enumerate(n_list):
            nl = node.lower()
            if "level" in nl or "title" in nl:
                self.g.remove_node(node)
                continue
            self.nodes_str.append(node)
            self.nodes_to_idx[node] = nodes_counter
            self.idx_to_nodes[nodes_counter] = node
            if len(list(self.g.succ[node])) == 0:
                self.output_nodes_str.append(node)
            elif len(list(self.g.pred[node])) == 0:
                self.input_nodes_str.append(node)
            nodes_counter += 1

    def get_edges_vars(self):
        for e in list(self.g.edges):
            idx_1 = self.nodes_to_idx[e[1]]
            idx_0 = self.nodes_to_idx[e[0]]
            self.edges_str.append((e[0], e[1]))
            self.n_edges += 1

            self.neighbors_idx[idx_0].append(idx_1)
            self.neighbors_idx[idx_1].append(idx_0)
            self.neighbors_str[e[0]].append(e[1])
            self.neighbors_str[e[1]].append(e[0])

            self.dir_neighbors_idx[idx_0].append(idx_1)
            self.dir_neighbors_str[e[0]].append(e[1])

    def calc_cells_qty(self):
        # n_cells to contain input/output nodes in the borders
        # and base cells to contain base nodes
        total_in_out = len(self.output_nodes_str) + len(self.input_nodes_str)
        n_base_nodes = len(self.nodes_str) - total_in_out
        n_cells_base_sqrt: int = ceil(sqrt(n_base_nodes))
        n_cells_base: int = pow(n_cells_base_sqrt, 2)
        n_border_cells: int = n_cells_base_sqrt * 4
        while total_in_out > n_border_cells:
            n_cells_base_sqrt += 2
            n_border_cells = (n_cells_base_sqrt) * 4
        n_cells_base = pow(n_cells_base_sqrt, 2)
        total_cells: int = n_cells_base + n_border_cells
        self.n_cells_sqrt: int = ceil(sqrt(total_cells))
        self.n_cells: int = pow(self.n_cells_sqrt, 2)

    def get_edges_idx(self, edges):
        edges_idx = [(self.nodes_to_idx[a], self.nodes_to_idx[b])
                     for (a, b) in edges]
        return edges_idx

    def get_nodes_idx(self, nodes):
        return [self.nodes_to_idx[n] for n in nodes]

    def get_mesh_distances(self, make_shuffle: bool = True) -> list[list[int]]:
        distance_table_raw: list[list] = [[] for _ in range((self.n_cells_sqrt - 1) * 2)]
        for i in range(self.n_cells_sqrt):
            for j in range(self.n_cells_sqrt):
                if j == i == 0:
                    continue
                dist: int = i + j
                if [i, j] not in distance_table_raw[dist - 1]:
                    distance_table_raw[dist - 1].append([i, j])
                if [i, -j] not in distance_table_raw[dist - 1]:
                    distance_table_raw[dist - 1].append([i, -j])
                if [-i, -j] not in distance_table_raw[dist - 1]:
                    distance_table_raw[dist - 1].append([-i, -j])
                if [-i, j] not in distance_table_raw[dist - 1]:
                    distance_table_raw[dist - 1].append([-i, j])
        if make_shuffle:
            for d in distance_table_raw:
                if make_shuffle:
                    random.shuffle(d)
        return distance_table_raw  # mesh_distances

    def format_distance_table(distance_table_raw: List[List], make_shuffle: bool) -> List[List[int]]:
        distance_table: List[List[int]] = []
        for dist in distance_table_raw:
            if make_shuffle:
                random.shuffle(dist)
            distance_table.extend(dist)
        return distance_table

    # FIXME
    def get_edges_depth_first(self, make_shuffle: bool = True, with_priority: bool = False):
        input_list = self.input_nodes_str

        if make_shuffle:
            random.shuffle(input_list)

        stack = input_list.copy()
        edges = []
        visited = []
        while stack:
            n = stack.pop()
            if n in visited:
                continue
            else:
                visited.append(n)
            if with_priority:
                for neigh in self.dir_neighbors_str[n]:
                    if neigh not in self.longest_path_nodes:
                        if neigh not in visited:
                            stack.append(neigh)
                            edges.append((n, neigh))
                for neigh in self.dir_neighbors_str[n]:
                    if neigh in self.longest_path_nodes:
                        if neigh not in visited:
                            stack.append(neigh)
                            edges.append((n, neigh))
            else:
                for neigh in self.dir_neighbors_str[n]:
                    if neigh not in visited:
                        stack.append(neigh)
                        edges.append((n, neigh))

        return edges

    def get_edges_zigzag(self, make_shuffle: bool = True):

        output_list = [[node, 'IN'] for node in self.output_nodes_str]

        if make_shuffle:
            random.shuffle(output_list)

        stack = output_list.copy()
        edges = []
        visited = set()
        convergence = []

        fan_in = {node: list(self.g.predecessors(node)) for node in self.g.nodes()}
        fan_out = {node: list(self.g.successors(node)) for node in self.g.nodes()}

        if make_shuffle:
            for node in self.g.nodes():
                random.shuffle(fan_in[node])
                random.shuffle(fan_out[node])

        while stack:
            a, direction = stack.pop(0)  # get the top1
            visited.add(a)

            if direction == 'IN':  # direction == 'IN'

                if fan_out[a]:  # Case 3
                    b = fan_out[a][-1]
                    stack.insert(0, [a, 'IN'])
                    stack[:0] = [[a, 'IN']] * len(fan_in[a])
                    stack.insert(0, [b, 'OUT'])
                    fan_out[a].remove(b)
                    fan_in[b].remove(a)
                    if b in visited:
                        convergence.append([a, b])
                    edges.append([a, b, 'OUT'])

                elif fan_in[a]:  # Case 2
                    b = fan_in[a][-1]
                    stack.insert(0, [a, 'IN'])
                    stack[:0] = [[b, 'IN']] * len(fan_in[a])
                    fan_in[a].remove(b)
                    fan_out[b].remove(a)
                    if b in visited:
                        convergence.append([a, b])
                    edges.append([a, b, 'IN'])

            else:  # direction == 'OUT'

                if fan_in[a]:  # Case 3
                    b = fan_in[a][0]
                    stack.insert(0, [a, 'OUT'])
                    stack[:0] = [[a, 'OUT']] * len(fan_out[a])
                    stack.insert(0, [b, 'IN'])
                    fan_in[a].remove(b)
                    fan_out[b].remove(a)
                    if b in visited:
                        convergence.append([a, b])
                    edges.append([a, b, 'IN'])

                elif fan_out[a]:  # Case 2
                    b = fan_out[a][0]
                    stack.insert(0, [a, 'OUT'])
                    stack[:0] = [[b, 'OUT']] * len(fan_out[a])
                    fan_out[a].remove(b)
                    fan_in[b].remove(a)
                    if b in visited:
                        convergence.append([a, b])
                    edges.append([a, b, 'OUT'])

        return self.clear_edges(edges), self.clear_edges(edges, False), convergence

    @staticmethod
    def clear_edges(edges: List[List[str]], remove_placed_edges: bool = True) -> List[List[str]]:
        dic = set()
        dic.add(edges[0][0])
        new_edges = []
        for edge in edges:
            n1, n2 = edge[:2]
            if n2 not in dic or not remove_placed_edges:
                dic.add(n2)
                new_edges.append([n1, n2])
        return new_edges

    def get_cost(self, n_c, node1, node2, cell1, cell2):
        cost1_b = 0
        cost1_a = 0
        cost2_b = 0
        cost2_a = 0
        if node1 is not None:
            for n in self.neighbors_idx[node1]:
                cost1_b += self.get_manhattan_distance(cell1, n_c[n], self.n_cells_sqrt)
                if cell2 == n_c[n]:
                    cost1_a += self.get_manhattan_distance(cell1, cell2, self.n_cells_sqrt)
                else:
                    cost1_a += self.get_manhattan_distance(cell2, n_c[n], self.n_cells_sqrt)
        if node2 is not None:
            for n in self.neighbors_idx[node2]:
                cost2_b += self.get_manhattan_distance(cell2, n_c[n], self.n_cells_sqrt)
                if cell1 == n_c[n]:
                    cost2_a += self.get_manhattan_distance(cell2, cell1, self.n_cells_sqrt)
                else:
                    cost2_a += self.get_manhattan_distance(cell1, n_c[n], self.n_cells_sqrt)
        return cost1_b, cost1_a, cost2_b, cost2_a

    @staticmethod
    def get_manhattan_distance(cell1: int, cell2: int, n_cells_sqrt) -> int:
        cell1_x: int = cell1 % n_cells_sqrt
        cell1_y: int = cell1 // n_cells_sqrt
        cell2_x: int = cell2 % n_cells_sqrt
        cell2_y: int = cell2 // n_cells_sqrt
        return abs(cell1_y - cell2_y) + abs(cell1_x - cell2_x)

    def longest_path_and_length(self):
        nodes = []
        length = 0
        if nx.is_directed_acyclic_graph(self.g):
            # Find the longest path in the DAG
            nodes = nx.dag_longest_path(self.g)
            length = nx.dag_longest_path_length(self.g)
        self.longest_path_nodes = nodes
        path = []
        for i in range(0, len(nodes) - 1):
            path.append([nodes[i], nodes[i + 1]])
        self.longest_path = path

    def get_graph_annotations(self, edges: List[List[int]], convergences: List[List[int]]) -> Dict[
        int, List[List[int]]]:
        annotations: Dict[str, List[List[str]]] = {}

        # Initialization dictionary
        for edge in edges:
            key: str = self.func_key(f"{edge[0]}", f"{edge[1]}")
            annotations[key] = []

        for elem_cycle_begin, elem_cycle_end in convergences:
            walk_key: List[str] = []
            found_start = False
            count = 0
            value1 = -1

            for edge in reversed(edges):
                a = edge[0]
                b = edge[1]
                if elem_cycle_begin == b and not found_start:
                    value1 = a
                    key = Graph.func_key(f"{value1}", f"{elem_cycle_begin}")
                    walk_key.insert(0, key)
                    annotations[key].append([elem_cycle_end, count])
                    count += 1
                    found_start = True

                elif found_start and (value1 == b or elem_cycle_end == edge[0]):
                    value1, value2 = a, b
                    key = self.func_key(f"{value1}", f"{value2}")
                    if value1 != elem_cycle_end and value2 != elem_cycle_end:
                        walk_key.insert(0, key)
                        annotations[key].append([elem_cycle_end, count])
                        count += 1
                    else:
                        # Go back and update values
                        for k in range(count // 2):
                            dic_actual = annotations[walk_key[k]]
                            for dic_key, (node, count) in enumerate(dic_actual):
                                if node == elem_cycle_end:
                                    dic_actual[dic_key][1] = k + 1
                        break  # to the next on the vector convergence

        # clear invalid annotations
        placed_nodes = {None: True}
        for k, v in annotations.items():
            (a, b) = [int(i) for i in k.split()]
            placed_nodes[a] = True
            placed_nodes[b] = True
            for (c, _) in (v.copy()):
                if placed_nodes.get(c) is None:
                    annotations[k].remove([c, _])

        return annotations

    @staticmethod
    def func_key(val1: str, val2: str) -> str:
        return val1 + " " + val2

    @staticmethod
    def func_unkey(text: str) -> List[str]:
        return text.split(" ")
