from src.py.util.util import get_files_list_by_extension, get_project_root


def blif_to_dot(base_path, blif_path, blif_file):
    with open(f"{blif_path}", 'r') as blif:
        lines = blif.readlines()

    with open(f"{base_path}{blif_file}.dot", 'w') as dot:
        dot.write('digraph G {\n')

        slash_flag = True
        while slash_flag:
            slash_flag = False
            lines_tmp = []
            for i, line in enumerate(lines):
                if "\\" in line:
                    line = line.replace("\\", lines[i + 1])
                    lines.pop(i + 1)
                    if "\\" in line:
                        slash_flag = True
                lines_tmp.append(line)

            lines = lines_tmp

        nodes = {}

        for line in lines:
            if line.startswith('.names'):
                parts = line.split()[1:]
                output = parts[-1]
                inputs = parts[:-1]
                for input_node in inputs:
                    dot.write(f'    "{input_node}" -> "{output}";\n')
                    if input_node not in nodes.keys():
                        nodes[input_node] = {'in': 0, 'out': 0}
                    nodes[input_node]['out'] += 1
                if output not in nodes.keys():
                    nodes[output] = {'in': 0, 'out': 0}
                nodes[output]['in']+=len(inputs)
        for node in nodes:
            if nodes[node]['out'] == 0 and nodes[node]['in'] > 1:
                dot.write(f'    "{node}" -> "out_{node}";\n')

        dot.write('}\n')


if __name__ == '__main__':
    root_path = get_project_root()
    base_path = f"{root_path}/benchmarks/fpga/dot_TRETS/"
    files = get_files_list_by_extension(f"{root_path}/benchmarks/fpga/dot_TRETS/", ".blif")
    # files = Util.get_files_list_by_extension(f"{root_path}/benchmarks/fpga/dot_EPFL/", ".blif")
    for file in files:
        blif_to_dot(base_path, file[0], file[1][:-5])
