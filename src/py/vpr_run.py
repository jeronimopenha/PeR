
from src.py.graph.graph import Graph
from src.py.util.util import verify_path, get_project_root, get_files_list_by_extension, read_json, generate_vpr_data

# quero ler todos os arquivos do diretório de posicionamentos
reports_path_sufix = "reports/fpga/outputs/"
benchmark_path_sufix = "benchmarks/fpga/bench_test"
root_path = verify_path(get_project_root())
look_word = "yoto"
reports_path = root_path + reports_path_sufix
benchmark_path = root_path + benchmark_path_sufix

files = get_files_list_by_extension(reports_path, ".json")
files_tmp = files.copy()
for file, name in files_tmp:
    if look_word not in name.lower():
        files.remove((file, name))
del files_tmp

# abrir cada arquivo
for file, name in files:
    report = read_json(file)
    dotPath = report['dotPath']
    dotName = report['dotName']
    graph = Graph(dotPath, dotName)
    generate_vpr_data(graph, report,'./', './')
    a=1
a = 1
