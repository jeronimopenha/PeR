from graphs.util import get_project_root, get_files_list_by_extension


def blif_to_dot(base_path, blif_path, blif_file: str):
    with open(f"{blif_path}", 'r') as blif:
        lines = blif.readlines()

    if "ctrl_k6" in blif_file:
        a = 1
    with open(f"{base_path}{blif_file}.dot", 'w') as dot:
        dot.write('digraph G {\n')
        lines_tmp = []

        for i, line in enumerate(lines):
            while "\\" in line:
                line = line.replace("\\", lines[i + 1])
                lines.pop(i + 1)
            while "\n" in line:
                line = line.replace("\n", "")
            lines_tmp.append(line)

        lines = lines_tmp

        blif_outputs = []

        for line in lines:
            if "outputs" in line:
                blif_outputs.extend(line.split()[1:])
            if line.startswith('.names'):
                parts = line.split()[1:]
                output = parts[-1]
                inputs = parts[:-1]
                for input_node in inputs:
                    dot.write(f'    "{input_node}" -> "{output}";\n')
        for output in blif_outputs:
            dot.write(f'    "{output}" -> "out:{output}";\n')

        dot.write('}\n')


if __name__ == '__main__':
    root_path = get_project_root()
    base_path = f"{root_path}/benchmarks/fpga/eval/EPFL_blif/"
    files = get_files_list_by_extension(f"{root_path}/benchmarks/fpga/eval/EPFL_blif/", ".blif")
    # files = Util.get_files_list_by_extension(f"{root_path}/benchmarks/fpga/dot_EPFL/", ".blif")
    for file in files:
        blif_to_dot(base_path, file[0], file[1][:-5])
