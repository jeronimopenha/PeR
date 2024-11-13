# cython: language_level=3


import multiprocessing as mp
import random
import cython

from math import exp
from typing import List

from src.py.graph.graph import Graph
from src.py.per.base.per import PeR, EdAlgEnum, PeR_Enum
from src.py.util.util import verify_path


class FPGAPeR(PeR):
    def __init__(self, graph: Graph):
        super().__init__()
        # random.seed(0)
        self.graph: Graph = graph
        # I will initialize the vector with the possible positions for inputs and outputs
        self.possible_pos_in_out: List[int] = []
        self.get_in_out_pos()

    def per(self, per_alg: PeR_Enum, parameters: List, n_exec: int = 1, parrallel: bool = True):
        manager: mp.Manager = mp.Manager()
        report = manager.dict()  # Shared report dictionary
        lock = mp.Lock()  # Lock for synchronized access
        n_workers = mp.cpu_count() - 1

        # List to hold process references
        processes: List = []

        if parrallel:

            # Spawn processes for each exec_id
            for exec_id in range(n_exec):
                if per_alg == PeR_Enum.SA:
                    p = mp.Process(target=FPGAPeR.sa_worker, args=(self, exec_id, report, lock, parameters))
                elif per_alg == PeR_Enum.YOTT:
                    p = mp.Process(target=FPGAPeR.yott_worker, args=(self, exec_id, report, lock, parameters))
                else:
                    p = mp.Process(target=FPGAPeR.yoto_worker, args=(self, exec_id, report, lock, parameters))

                processes.append(p)
                p.start()

                # Limit the number of concurrent workers
                if len(processes) >= n_workers:
                    for proc in processes:
                        proc.join()  # Wait for all workers to finish
                    processes.clear()  # Reset the list for the next batch

            # Wait for the remaining processes to finish
            for proc in processes:
                proc.join()

        else:
            for exec_id in range(n_exec):
                if per_alg == PeR_Enum.SA:
                    FPGAPeR.sa_worker(self, exec_id, report, lock, parameters)
                elif per_alg == PeR_Enum.YOTT:
                    FPGAPeR.yott_worker(self, exec_id, report, lock, parameters)
                else:
                    FPGAPeR.yoto_worker(self, exec_id, report, lock, parameters)
        rep = dict(report)
        return rep

    def sa_worker(cls, exec_id: int, report, lock, parameters: List):
        print(f"Starting SA {exec_id}_{cls.graph.dot_name}")
        # report
        t_min: float = 0.001
        t: int = 100

        # Cache graph attributes to avoid repeated lookups
        n_cells: int = cls.graph.n_cells
        n_cells_sqrt: int = cls.graph.n_cells_sqrt
        edges_idx: List = cls.graph.edges_idx
        longest_path_edges: int = cls.graph.get_edges_idx(cls.graph.longest_path)
        longest_path_nodes = cls.graph.longest_path_nodes

        # First I will start the placement of matrix
        placement = [None] * n_cells

        # Creating the n2c matrix
        n2c = [None] * cls.graph.n_nodes

        # now I need to place every note in the placement matrix
        # self.place_input_output_nodes(n2c, placement)
        nodes = cls.graph.input_nodes_idx + cls.graph.output_nodes_idx
        cls.place_nodes(n2c, placement, cls.possible_pos_in_out, nodes)

        nodes_clb: List[int] = [node for node in cls.graph.get_nodes_idx(cls.graph.nodes_str) if node not in nodes]

        possible_clb_pos: List[int] = []

        # Loop through rows (ignoring first and last row)
        matrix: List[int] = [n for n in range(n_cells)]
        m = n_cells_sqrt
        for i in range(1, m - 1):
            # Calculate start and end index for this row (ignoring first and last column)
            start: int = i * m + 1
            end: int = (i + 1) * m - 1

            # Iterate over the possible_clb_pos elements of this row
            for j in range(start, end):
                possible_clb_pos.append(matrix[j])
        cls.place_nodes(n2c, placement, possible_clb_pos, nodes_clb)

        # cls.write_dot(f"/home/jeronimo/GIT/PeR/reports/fpga/", "placed.dot", placement, n2c)

        while t >= t_min:
            for cell_a in range(1, n_cells - 1):
                for cell_b in range(1, n_cells - 1):
                    a = placement[cell_a]
                    b = placement[cell_b]
                    if (
                            cell_a == cell_b or  # same cell
                            (a is None and b is None) or  # empty cells
                            cell_a == n_cells_sqrt - 1 or
                            cell_b == n_cells_sqrt - 1 or
                            cell_a == n_cells - n_cells_sqrt or
                            cell_b == n_cells - n_cells_sqrt or
                            (cell_a in cls.possible_pos_in_out and cell_b not in cls.possible_pos_in_out) or
                            (cell_a not in cls.possible_pos_in_out and cell_b in cls.possible_pos_in_out)
                    ):
                        continue

                    cost_a_b, cost_a_a, cost_b_b, cost_b_a = cls.graph.get_cost(n2c, a, b, cell_a, cell_b)

                    cost_a = cost_a_a + cost_b_a
                    cost_b = cost_a_b + cost_b_b

                    try:
                        valor = exp((-1 * (cost_a - cost_b) / t))
                    except:
                        valor = -1.0

                    rnd = random.random()

                    if cost_a < cost_b or rnd <= valor:
                        if a is not None:
                            n2c[a] = cell_b

                        if b is not None:
                            n2c[b] = cell_a
                        placement[cell_a], placement[cell_b] = b, a
                        # cls.write_dot(f"/home/jeronimo/GIT/PeR/reports/fpga/", "placed.dot", placement, n2c)
                        # print(t, actual_cost)
                        # self.write_dot('/home/jeronimo/GIT/PeR/reports/fpga/', f"_placed.dot",
                        #               placement, n2c)
                t *= 0.999

        # cls.write_dot(f"/home/jeronimo/GIT/PeR/reports/fpga/", "placed.dot", placement, n2c)

        h, tc = cls.calc_distance(n2c, edges_idx, n_cells_sqrt, cls.graph.n_nodes)

        longest_path_cost = cls.calc_distance(n2c,
                                              cls.graph.get_edges_idx(cls.graph.longest_path),
                                              cls.graph.n_cells_sqrt, cls.graph.n_nodes)[1]

        print(f"Ending {exec_id}")

        # Write to the shared report with a lock
        with lock:
            report[exec_id] = {
                'exec_id': exec_id,
                'dot_name': cls.graph.dot_name,
                'dot_path': cls.graph.dot_path,
                'placer': 'sa',
                'edges_algorithm': "direct",
                'total_cost': tc,
                'histogram': h,
                'longest_path_cost': longest_path_cost,
                # 'longest_path': cls.graph.longest_path,
                # 'longest_path_idx': longest_path_nodes,
                # 'nodes_idx': cls.graph.nodes_to_idx,
                # 'input_nodes': cls.graph.input_nodes_idx,
                # 'output_nodes': cls.graph.output_nodes_idx,
                'placement': placement,
                'n2c': n2c,
                # 'edges': edges_idx,
            }

    def yoto_worker(cls, exec_id, report, lock, parameters: List):
        print(f"Starting YOTO {exec_id}_{cls.graph.dot_name}")
        edges_alg: EdAlgEnum = parameters[0]
        # Prepare the report
        placement = [-1] * cls.graph.n_cells
        distances_cells = cls.graph.get_mesh_distances()
        n2c = [-1] * cls.graph.n_nodes

        tries = 0
        swaps = 0

        # Getting the edges to be placed
        ed_str = []
        if edges_alg == EdAlgEnum.DEPTH_FIRST_NO_PRIORITY:
            ed_str = cls.graph.get_edges_depth_first()
        elif edges_alg == EdAlgEnum.DEPTH_FIRST_WITH_PRIORITY:
            ed_str = cls.graph.get_edges_depth_first(with_priority=True)
        elif edges_alg == EdAlgEnum.ZIG_ZAG:
            ed_str = cls.graph.get_edges_zigzag()[0]

        ed = cls.graph.get_edges_idx(ed_str)

        nodes = []
        # Input and output position placement
        if edges_alg == EdAlgEnum.DEPTH_FIRST_NO_PRIORITY or edges_alg == EdAlgEnum.DEPTH_FIRST_WITH_PRIORITY:
            nodes = cls.graph.get_nodes_idx(cls.graph.input_nodes_str)
        elif edges_alg == EdAlgEnum.ZIG_ZAG:
            nodes = [ed[0][0]]
        cls.place_nodes(n2c, placement, cls.possible_pos_in_out, nodes)

        # Prepare for faster access
        n_cells_sqrt = cls.graph.n_cells_sqrt
        possible_positions = set(cls.possible_pos_in_out)
        input_nodes_idx = set(cls.graph.input_nodes_idx)
        output_nodes_idx = set(cls.graph.output_nodes_idx)

        # Yoto algorithm logic
        for i, e in enumerate(ed):
            a = e[0]
            b = e[1]
            if n2c[b] != 1:
                continue
            if n2c[a] == -1:
                nodes = [a]
                cls.place_nodes(n2c, placement, possible_positions, nodes)

            ia = n2c[a] // n_cells_sqrt
            ja = n2c[a] % n_cells_sqrt

            for l_n, line in enumerate(distances_cells):
                placed = False
                for ij in line:
                    tries += 1
                    ib = ia + ij[0]
                    jb = ja + ij[1]
                    if (
                            ib < 0 or
                            ib >= n_cells_sqrt or
                            jb < 0 or
                            jb >= n_cells_sqrt or
                            (ib == 0 and jb == 0) or
                            (ib == (n_cells_sqrt - 1) and jb == (n_cells_sqrt - 1)) or
                            (ib == (n_cells_sqrt - 1) and jb == 0) or
                            (ib == 0 and jb >= (n_cells_sqrt - 1))
                    ):
                        continue
                    ch = ib * n_cells_sqrt + jb
                    if ch in possible_positions:
                        if b not in input_nodes_idx and b not in output_nodes_idx:
                            continue
                    else:
                        if b in input_nodes_idx or b in output_nodes_idx:
                            continue
                    if placement[ch] == -1:
                        placement[ch] = b
                        n2c[b] = ch
                        placed = True
                        swaps += 1
                        break
                if placed:
                    break

        h, tc = cls.calc_distance(n2c, cls.graph.edges_idx, n_cells_sqrt, cls.graph.n_nodes)

        tc += 0.1

        longest_path_cost = cls.calc_distance(n2c,
                                              cls.graph.get_edges_idx(cls.graph.longest_path),
                                              n_cells_sqrt, cls.graph.n_nodes)[1]

        longest_path_cost += 0.1

        print(f"Ending {exec_id}")

        # Write to the shared report with a lock
        with lock:
            report[exec_id] = {
                'exec_id': exec_id,
                'dot_name': cls.graph.dot_name,
                'dot_path': cls.graph.dot_path,
                'placer': 'yoto',
                'tries': tries,
                'swaps': swaps,
                'edges_algorithm': edges_alg.name,
                'total_cost': tc,
                'histogram': h,
                'longest_path_cost': longest_path_cost,
                'placement': placement,
                'n2c': n2c,
            }

    # Works only with one annotation
    def yott_worker(cls, exec_id, report, lock, parameters: List):
        print(f"Starting YOTT {exec_id}_{cls.graph.dot_name}")
        # Prepare the report
        n_cells = cls.graph.n_cells
        n_cells_sqrt = cls.graph.n_cells_sqrt

        tries = 0
        swaps = 0

        placement = [None for _ in range(n_cells)]
        distances_cells = cls.graph.get_mesh_distances()
        n2c = [None for _ in range(cls.graph.n_nodes)]

        # Getting the edges to be placed
        ed_str, tmp, convergences_str = cls.graph.get_edges_zigzag()
        ed = cls.graph.get_edges_idx(ed_str)
        convergences = cls.graph.get_edges_idx(convergences_str)
        annotations = cls.graph.get_graph_annotations(ed, convergences)

        # Input and output position placement
        nodes = [ed[0][0]]
        cls.place_nodes(n2c, placement, cls.possible_pos_in_out, nodes)
        # cls.write_dot(f"/home/jeronimo/GIT/PeR/reports/fpga/", "placed.dot", placement, n2c)
        # Yott algorithm logic
        for i, e in enumerate(ed):
            a = e[0]
            b = e[1]
            if n2c[b] is not None:
                continue
            if n2c[a] is None:
                nodes = [a]
                cls.place_nodes(n2c, placement, cls.possible_pos_in_out, nodes)

            ia = n2c[a] // n_cells_sqrt
            ja = n2c[a] % n_cells_sqrt

            better_cell = None
            better_cell_dist = n_cells

            for l_n, line in enumerate(distances_cells):
                placed = False
                for ij in line:
                    tries += 1
                    ib = ia + ij[0]
                    jb = ja + ij[1]
                    if (
                            ib < 0 or
                            ib >= n_cells_sqrt or
                            jb < 0 or
                            jb >= n_cells_sqrt or
                            (ib == 0 and jb == 0) or
                            (ib >= (n_cells_sqrt - 1) and jb >= (n_cells_sqrt - 1)) or
                            (ib >= (n_cells_sqrt - 1) and jb == 0) or
                            (ib == 0 and jb >= (n_cells_sqrt - 1))
                    ):
                        continue
                    ch = ib * n_cells_sqrt + jb
                    if ch in cls.possible_pos_in_out:
                        if b not in cls.graph.input_nodes_idx and b not in cls.graph.output_nodes_idx:
                            continue
                    else:
                        if b in cls.graph.input_nodes_idx or b in cls.graph.output_nodes_idx:
                            continue
                    annotation = annotations[f"{a} {b}"]
                    if len(annotation) > 0:
                        if placement[ch] is not None:
                            continue
                        target_ch: int = n2c[annotation[0][0]]
                        # TODO
                        for ann_idx, ann in annotation:
                            pass

                        dist = cls.graph.get_manhattan_distance(target_ch, ch, n_cells_sqrt)
                        if dist == annotation[0][1] + 1:
                            if placement[ch] is None:
                                placement[ch] = b
                                n2c[b] = ch
                                placed = True
                                swaps += 1
                                break

                        mod_dist = abs(annotation[0][1] + 1 - dist)

                        if mod_dist < better_cell_dist:
                            better_cell_dist = dist
                            better_cell = ch
                        continue
                    if placement[ch] is None:
                        placement[ch] = b
                        n2c[b] = ch
                        placed = True
                        swaps += 1
                        break
                if placed:
                    break
            if not placed:
                placement[better_cell] = b
                n2c[b] = better_cell
                swaps += 1
            cls.write_dot(f"/home/jeronimo/GIT/PeR/reports/fpga/", "placed.dot", placement, n2c)

        h, tc = cls.calc_distance(n2c, cls.graph.edges_idx, n_cells_sqrt, cls.graph.n_nodes)

        tc -= 0.1

        longest_path_cost = cls.calc_distance(n2c,
                                              cls.graph.get_edges_idx(cls.graph.longest_path),
                                              n_cells_sqrt, cls.graph.n_nodes)[1]
        longest_path_cost -= 0.1
        if longest_path_cost < 0:
            print(f"longest_path_cost {longest_path_cost}")

        print(f"Ending {exec_id}")

        # Write to the shared report with a lock
        with lock:
            report[exec_id] = {
                'exec_id': exec_id,
                'dot_name': cls.graph.dot_name,
                'dot_path': cls.graph.dot_path,
                'placer': 'yott',
                'tries': tries,
                'swaps': swaps,
                'edges_algorithm': EdAlgEnum.ZIG_ZAG.name,
                'total_cost': tc,
                'histogram': h,
                'longest_path_cost': longest_path_cost,
                # 'longest_path': cls.graph.longest_path,
                # 'longest_path_idx': cls.graph.get_nodes_idx(cls.graph.longest_path_nodes),
                # 'nodes_idx': cls.graph.nodes_to_idx,
                # 'input_nodes': cls.graph.input_nodes_idx,
                # 'output_nodes': cls.graph.output_nodes_idx,
                'placement': placement,
                'n2c': n2c,
                # 'edges': cls.graph.edges_idx,
                # 'neigh': cls.graph.neighbors_idx
            }

    def write_dot(self, path, file_name, placement, n2c):
        path = verify_path(path)
        output_dot_file = path + file_name
        dot_head = 'digraph layout{\n rankdir=TB;\n splines=ortho;\n node [style=filled shape=square fixedsize=true width=0.6];\n'
        dot_foot = 'edge [constraint=true, style=invis];\n'

        for i in range(self.graph.n_cells_sqrt):
            for j in range(self.graph.n_cells_sqrt):
                dot_foot = dot_foot + '%d' % (j * self.graph.n_cells_sqrt + i)
                if (j + 1) % self.graph.n_cells_sqrt == 0:
                    dot_foot = dot_foot + ';\n'
                else:
                    dot_foot = dot_foot + ' -> '

        for i in range(self.graph.n_cells):
            if i % self.graph.n_cells_sqrt == 0:
                dot_foot += 'rank = same {'
            dot_foot = dot_foot + '%d' % i
            if (i + 1) % self.graph.n_cells_sqrt == 0:
                dot_foot += '};\n'
            else:
                dot_foot += ' -> '

        dot_foot = dot_foot + '}'

        str_out = dot_head

        input_nodes = [self.graph.nodes_to_idx[node] for node in self.graph.input_nodes_str]
        output_nodes = [self.graph.nodes_to_idx[node] for node in self.graph.output_nodes_str]

        for i in range(self.graph.n_cells):
            if placement[i] == -1:
                str_out += '%d[label="%d", fontsize=8, fillcolor="%s"];\n' % (
                    i, i, '#ffffff')
            elif placement[i] in input_nodes:
                str_out += '%d[label="%d", fontsize=8, fillcolor="%s"];\n' % (
                    i, placement[i],
                    '#e3c9af')  ##ffffff')  # if int(c_content, 16) == 0 else '#d9d9d9')
            elif placement[i] in output_nodes:
                str_out += '%d[label="%d", fontsize=8, fillcolor="%s"];\n' % (
                    i, placement[i],
                    '#a9ccde')  ##ffffff')  # if int(c_content, 16) == 0 else '#d9d9d9')
            else:
                str_out += '%d[label="%d", fontsize=8, fillcolor="%s"];\n' % (
                    i, placement[i], '#91e3bb')
        str_out += 'edge [constraint=false, style=vis];'
        for ed in self.graph.get_edges_idx(self.graph.edges_str):
            a = ed[0]
            b = ed[1]
            str_out += f"{n2c[a]} -> {n2c[b]};\n"
        str_out += dot_foot

        with open(output_dot_file, 'w') as f:
            f.write(str_out)
        f.close()

    def place_nodes(self, n2c, placement, possible_pos, nodes):
        i = 0
        while i < len(nodes):
            if i < len(nodes):
                n = nodes[i]
                while True:
                    ch = self.choose_position(placement, possible_pos)
                    if placement[ch] == -1:
                        placement[ch] = n
                        n2c[n] = ch
                        break
            i += 1

    @staticmethod
    def calc_distance(n2c, edges, cells_sqrt, n_nodes):
        distance = -len(edges)
        distances = {}
        for e in edges:
            dist = Graph.get_manhattan_distance(n2c[e[0]], n2c[e[1]], cells_sqrt)
            if dist == 0:
                a = 1
            if dist not in distances.keys():
                distances[dist] = 0
            distances[dist] += 1
            distance += dist
        if distance < 0:
            a = 1
        else:
            a = 1

        return dict(sorted(distances.items())), distance

    @staticmethod
    def manhattan_distance(ia, ja, ib, jb):
        return abs(ia - ib) + abs(ja - jb)

    @staticmethod
    def choose_position(placement, choices):
        while True:
            ch = random.choice(choices)
            if placement[ch] != -1:
                continue
            return ch

    def get_in_out_pos(self):
        in_out = []
        for i in range(1, self.graph.n_cells_sqrt - 1):
            in_out.append(i)
        for i in range(1, self.graph.n_cells_sqrt - 1):
            in_out.append(i + self.graph.n_cells - self.graph.n_cells_sqrt)
        for i in range(self.graph.n_cells_sqrt, self.graph.n_cells - self.graph.n_cells_sqrt, self.graph.n_cells_sqrt):
            in_out.append(i)
        for i in range(self.graph.n_cells_sqrt * 2 - 1, self.graph.n_cells - 1, self.graph.n_cells_sqrt):
            in_out.append(i)
        self.possible_pos_in_out = in_out
