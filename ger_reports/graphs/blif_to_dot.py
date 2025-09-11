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
        blif_inputs = []

        max_parts = 0
        n_names = 0

        for line in lines:
            if "outputs" in line:
                blif_outputs.extend(line.split()[1:])
            if "inputs" in line:
                blif_inputs.extend(line.split()[1:])
            if line.startswith('.names'):
                n_names+=1
                parts = line.split()[1:]
                for part in parts:
                    if part in blif_inputs:
                        blif_inputs.remove(part)
                len_parts = len(parts)
                if len_parts > max_parts:
                    max_parts = len_parts
                output = parts[-1]
                inputs = parts[:-1]
                for input_node in inputs:
                    dot.write(f'    "{input_node}" -> "{output}";\n')
        for output in blif_outputs:
            dot.write(f'    "{output}" -> "out:{output}";\n')
        #for inp in blif_inputs:
        #    dot.write(f'    "{inp}";\n')

        dot.write('}\n')


if __name__ == '__main__':
    root_path = get_project_root()
    base_path = f"{root_path}/benchmarks/fpga/JSC/"
    files = get_files_list_by_extension(base_path, ".blif")
    # files = Util.get_files_list_by_extension(f"{root_path}/benchmarks/fpga/dot_EPFL/", ".blif")
    for file in files:
        blif_to_dot(base_path, file[0], file[1][:-5])
