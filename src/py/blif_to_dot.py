from src.py.util.util import get_files_list_by_extension, get_project_root


def blif_to_dot(base_path, blif_path, blif_file):
    with open(f"{blif_path}", 'r') as blif:
        lines = blif.readlines()

    with open(f"{base_path}{blif_file}.dot", 'w') as dot:
        dot.write('digraph G {\n')

        lines_tmp = []
        for i, line in enumerate(lines):
            if "\\" in line:
                line = line.replace("\\", lines[i + 1])
                lines.pop(i + 1)
            lines_tmp.append(line)

        lines = lines_tmp

        for line in lines:
            if line.startswith('.names'):
                parts = line.split()[1:]
                output = parts[-1]
                inputs = parts[1:-1]
                for input_node in inputs:
                    dot.write(f'    "{input_node}" -> "{output}";\n')

        dot.write('}\n')


if __name__ == '__main__':
    root_path = get_project_root()
    base_path = f"{root_path}/benchmarks/fpga/dot_TRETS/"
    files = get_files_list_by_extension(f"{root_path}/benchmarks/fpga/dot_TRETS/", ".blif")
    # files = Util.get_files_list_by_extension(f"{root_path}/benchmarks/fpga/dot_EPFL/", ".blif")
    for file in files:
        blif_to_dot(base_path, file[0], file[1][:-5])
