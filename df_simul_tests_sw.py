import os
import time
from src.python.sw.df_simul.df_simul_sw import DfSimulSw
from src.python.util.per_graph import PeRGraph
from src.python.util.util import Util


def run_simul_graphs():
    root_path: str = Util.get_project_root()
    dot_input = root_path + '/dot_db/dot_simul/'

    output_base = root_path + '/reports/sw/df_simul'

    if not os.path.exists(output_base):
        os.makedirs(output_base)

    # list connected benchmarks
    dots_list = Util.get_files_list_by_extension(dot_input, '.dot')

    for dot_path, dot_name in dots_list:
        print(f'DOT: {dot_name}')
        per_graph = PeRGraph(dot_path, dot_name)
        df_simul = DfSimulSw(per_graph, output_base)
        th: list = df_simul.run_simulation()


if __name__ == '__main__':
    start_time = time.time()
    run_simul_graphs()
    end_time = time.time()
    print("Total time: " + str(end_time - start_time))