import json
import os
import matplotlib.pyplot as plt
import numpy as np
from matplotlib.patches import Patch
from matplotlib.colors import LinearSegmentedColormap, Normalize


def plot_heatmap_with_used_mask(data, used_mask, output_path, title="", leg_label=""):
    target_pixels = 1024
    dpi = 100
    figsize = (target_pixels / dpi, target_pixels / dpi)

    used_mask = np.array(used_mask)

    image = np.ones((*used_mask.shape, 3))

    image[used_mask] = [224 / 255, 224 / 255, 224 / 255]

    norm = Normalize(vmin=0, vmax=np.max(data))

    custom_cmap = LinearSegmentedColormap.from_list("custom", [
        (0.0, "#ADD8E6"),  # azul claro
        (0.5, "#800080"),  # roxo
        (1.0, "#FF0000"),  # vermelho
    ])

    # Aplica o heatmap por cima do cinza
    for y in range(data.shape[0]):
        for x in range(data.shape[1]):
            if data[y, x] > 0:
                r, g, b, _ = custom_cmap(norm(data[y, x]))
                image[y, x] = [r, g, b]

    fig, ax = plt.subplots(figsize=figsize, dpi=dpi)
    ax.imshow(image, interpolation="nearest")
    ax.set_title(title)
    ax.axis("on")  # Mostra a moldura dos eixos
    ax.set_xticks([])  # Remove os números do eixo X
    ax.set_yticks([])  # Remove os números do eixo Y

    cbar = fig.colorbar(
        plt.cm.ScalarMappable(norm=norm, cmap=custom_cmap),
        ax=ax,
        shrink=0.8,
        orientation='vertical',
        pad=0.02
    )
    cbar.set_label(leg_label)

    plt.savefig(output_path, dpi=300, bbox_inches="tight")
    plt.close()


def draw_lines_between_cells(origins_to_dests, heat_end, output_path, title="", subtitle="", threshold=10):
    n = heat_end.shape[0]
    image = np.ones((n, n, 3))  # fundo branco

    fig, ax = plt.subplots(figsize=(8, 8))
    ax.set_title(f"{title}\n{subtitle}, Threshold={threshold}")
    ax.axis("on")
    ax.set_xticks([])  # Remove os números do eixo X
    ax.set_yticks([])  # Remove os números do eixo Y

    legend_elements = [
        Patch(facecolor=(153 / 255, 153 / 255, 255 / 255), edgecolor='black', label='Origem'),
        Patch(facecolor=(255 / 255, 153 / 255, 153 / 255), edgecolor='black', label='Destino')
    ]
    ax.legend(handles=legend_elements, loc='upper right')
    ax.legend(
        handles=legend_elements,
        loc='center left',
        bbox_to_anchor=(1, 0.5),  # Fora da imagem, à direita
        borderaxespad=0.5,
        frameon=False
    )

    for origin, dests in origins_to_dests.items():
        for dest in dests:
            dy, dx = divmod(dest, n)
            if heat_end[dy, dx] > threshold:
                oy, ox = divmod(origin, n)
                image[oy, ox] = [153 / 255, 153 / 255, 255 / 255]  # azul claro
                image[dy, dx] = [255 / 255, 153 / 255, 153 / 255]  # vermelho suave

    ax.imshow(image, interpolation="nearest")

    for origin, dests in origins_to_dests.items():
        for dest in dests:
            dy, dx = divmod(dest, n)
            if heat_end[dy, dx] > threshold:
                oy, ox = divmod(origin, n)
                ax.plot([ox, dx], [oy, dy], color='black', linewidth=0.5)

    plt.savefig(output_path, dpi=300, bbox_inches="tight")
    plt.close()


def plot_histograms(hist_list, output_prefix, folder_path, baseName=""):
    for idx, hist in enumerate(hist_list):
        keys = list(map(int, hist.keys()))
        values = list(map(int, hist.values()))

        if not keys:
            continue  # pular histograma vazio

        min_k, max_k = min(keys), max(keys)

        # Organizar dados
        sorted_items = sorted(zip(keys, values))
        sorted_keys, sorted_vals = zip(*sorted_items)

        fig, ax = plt.subplots(figsize=(10, 4))
        ax.bar(sorted_keys, sorted_vals, color="black", width=0.8)

        ax.set_xlim(min_k - 1, max_k + 1)
        ax.set_title(f"{baseName} Histograma {idx}")
        ax.set_xlabel("Tentativas")
        ax.set_ylabel("Frequência")
        ax.set_yscale("log")
        ax.grid(axis='y', linestyle='--', alpha=0.5)

        output_file = os.path.join(folder_path, f"{output_prefix}_Hist{idx}.jpg")
        plt.savefig(output_file, bbox_inches="tight", dpi=300)
        plt.close()


def plot_boxplots(hist_list, output_prefix, folder_path):
    data_for_boxplot = []

    for hist in hist_list:
        samples = []
        for k, v in hist.items():
            samples.extend([int(k)] * int(v))  # repete o valor pela frequência
        data_for_boxplot.append(samples)

    if not data_for_boxplot:
        print("Nenhum histograma com dados.")
        return

    fig, ax = plt.subplots(figsize=(max(8, len(data_for_boxplot)), 5))
    ax.boxplot(data_for_boxplot, patch_artist=True,
               boxprops=dict(facecolor='lightblue', color='blue'),
               medianprops=dict(color='red'))

    ax.set_title(f"{output_prefix}")
    ax.set_xlabel("Histograma")
    ax.set_ylabel("Tentativas")
    ax.set_yscale("log")
    ax.set_xticks(range(1, len(data_for_boxplot) + 1))
    ax.set_xticklabels([str(i) for i in range(len(data_for_boxplot))])

    output_file = os.path.join(folder_path, f"{output_prefix}_Boxplots.jpg")
    plt.savefig(output_file, bbox_inches="tight", dpi=300)
    plt.close()


def process_json_metrics(folder_path):
    for file_name in os.listdir(folder_path):
        if not file_name.endswith(".json"):
            continue
        with open(os.path.join(folder_path, file_name)) as f:
            data = json.load(f)

        base_name = os.path.splitext(file_name)[0]

        # Converte vetores para matriz quadrada
        def vector_to_matrix(vec):
            n = int(len(vec) ** 0.5)
            return np.array(vec).reshape((n, n))

        heat_end = vector_to_matrix(data["heatEnd"])
        heat_begin = vector_to_matrix(data["heatBegin"])

        # Cria a máscara de células com valor > 0
        used_mask = heat_end > 0

        # Plot heatEnd com máscara
        output2 = os.path.join(folder_path, base_name + "_End.jpg")
        #plot_heatmap_with_used_mask(heat_end, used_mask, output2, base_name + " Custo de Destino", "Tentativas")

        # Plot heatBegin com máscara
        output1 = os.path.join(folder_path, base_name + "_Begin.jpg")
        #plot_heatmap_with_used_mask(heat_begin, used_mask, output1, base_name + " Pontos de Origem", "Posicionamentos")

        origins_to_dests = {int(k): v for k, v in data["orDest"].items()}
        output3 = os.path.join(folder_path, base_name + "_Lines.jpg")
        #draw_lines_between_cells(origins_to_dests, heat_end, output3, base_name, "Origens e Destinos com Linhas")

        hist_list = list(data["hist"].values())  # pega todos os dicionários num vetor
        #plot_histograms(hist_list, base_name, folder_path, base_name)
        plot_boxplots(hist_list, base_name, folder_path)


# Uso direto
if __name__ == "__main__":
    folder = "/home/jeronimo/GIT/PeR/reports/fpga/EPFL/yoto_df_x1_debug/metrics/"
    folder_path = os.path.dirname(os.path.abspath(folder))  # Pasta atual
    process_json_metrics(folder)
