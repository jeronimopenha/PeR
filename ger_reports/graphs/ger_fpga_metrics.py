import json
import os
import sys
import matplotlib.pyplot as plt
import numpy as np
from matplotlib.patches import Patch
from matplotlib.colors import LinearSegmentedColormap, Normalize

gray = [224 / 255] * 3
light_blue = [153 / 255, 153 / 255, 255 / 255]
light_red = [255 / 255, 153 / 255, 153 / 255]
light_purple = [204 / 255, 153 / 255, 204 / 255]
purple = [128 / 255, 0 / 255, 128 / 255]
red = [255 / 255, 0, 0]


def get_figsize(target_pixels=1024, dpi=100):
    size = target_pixels / dpi
    return (size, size), dpi


def plot_heatmap_with_used_mask(data, used_mask, output_path, title="", leg_label=""):
    (figsize, dpi) = get_figsize()

    image = np.ones((*used_mask.shape, 3))
    image[used_mask] = gray

    norm = Normalize(vmin=1, vmax=np.max(data))
    custom_cmap = LinearSegmentedColormap.from_list("custom", [
        (0.0, light_blue),
        (0.5, purple),
        (1.0, red),
    ])

    mask_nonzero = data > 0
    colored_pixels = custom_cmap(norm(data[mask_nonzero]))[:, :3]
    image[mask_nonzero] = colored_pixels

    fig, ax = plt.subplots(figsize=figsize, dpi=dpi)
    ax.imshow(image, interpolation="nearest")
    ax.set_title(title)
    ax.axis("on")
    ax.set_xticks([])
    ax.set_yticks([])

    cbar = fig.colorbar(
        plt.cm.ScalarMappable(norm=norm, cmap=custom_cmap),
        ax=ax,
        shrink=1.0,  # 0.8
        orientation='vertical',
        pad=0.02
    )
    cbar.set_label(leg_label)

    plt.savefig(output_path, dpi=300, bbox_inches="tight")
    plt.close()


def draw_lines_between_cells(origins_to_dests, used_mask, heat_end, output_path, title="", subtitle="", threshold=10):
    def is_color(pixel, target_rgb, tol=1e-2):
        return np.allclose(pixel, np.array(target_rgb), atol=1e-2)

    (figsize, dpi) = get_figsize()
    image = np.ones((*used_mask.shape, 3))
    image[used_mask] = gray

    n = heat_end.shape[0]

    fig, ax = plt.subplots(figsize=figsize)
    ax.set_title(f"{title}\n{subtitle}, Threshold={threshold}")
    ax.axis("on")
    ax.set_xticks([])  # remove the X numbers
    ax.set_yticks([])  # remove the Y numbers

    legend_elements = [
        Patch(facecolor=tuple(light_blue), edgecolor='black', label='Origem'),
        Patch(facecolor=tuple(light_red), edgecolor='black', label='Destino'),
        Patch(facecolor=tuple(light_purple), edgecolor='black', label='Origem\\\nDestino')
    ]
    ax.legend(
        handles=legend_elements,
        loc='center left',
        bbox_to_anchor=(1, 0.5),
        borderaxespad=0.5,
        frameon=False
    )

    '''lines_offset = 0.1
    for origin, dests in origins_to_dests.items():
        oy, ox = divmod(origin, n)
        for dest in dests:
            dy, dx = divmod(dest, n)
            tries = heat_end[dy, dx]
            if tries <= threshold:
                continue

            if is_color(image[oy, ox], light_red):
                image[oy, ox] = light_purple
            else:
                image[oy, ox] = light_blue
            image[dy, dx] = light_red
            ax.plot([ox, dx], [oy - lines_offset, dy + lines_offset], color='black', linewidth=0.15)'''

    for origin, dests in origins_to_dests.items():
        for dest in dests:
            dy, dx = divmod(dest, n)
            if dy == 0 or dx == 0 or (dy == n - 1) or (dx == n - 1):
                continue
            tries = heat_end[dy, dx]
            if tries > threshold:
                oy, ox = divmod(origin, n)
                image[oy, ox] = light_blue  # light blue
                image[dy, dx] = light_red  # light red

    lines_offset = 0.1
    for origin, dests in origins_to_dests.items():
        for dest in dests:
            dy, dx = divmod(dest, n)
            if dy == 0 or dx == 0 or dy == n - 1 or dx == n - 1:
                continue
            tries = heat_end[dy, dx]
            if tries > threshold:
                oy, ox = divmod(origin, n)
                if is_color(image[oy, ox], light_red):
                    image[oy, ox] = light_purple
                ax.plot([ox, dx], [oy - lines_offset, dy + lines_offset], color='black', linewidth=0.15)

    ax.imshow(image, interpolation="nearest")
    plt.savefig(output_path, dpi=300, bbox_inches="tight")
    plt.close()


def plot_histograms(hist_list, output_prefix, folder_path, baseName=""):
    for idx, hist in enumerate(hist_list):
        if not hist:
            continue
        sorted_items = sorted((int(k), int(v)) for k, v in hist.items())
        sorted_keys, sorted_vals = zip(*sorted_items)

        fig, ax = plt.subplots(figsize=(10, 4))
        ax.bar(sorted_keys, sorted_vals, color="black", width=0.8)
        ax.set_xlim(min(sorted_keys) - 1, max(sorted_keys) + 1)
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
            samples.extend([int(k)] * int(v))
        data_for_boxplot.append(samples)

    if not data_for_boxplot:
        print("Nenhum histograma com dados.")
        return

    fig, ax = plt.subplots(figsize=(max(8, len(data_for_boxplot)), 5))
    ax.boxplot(data_for_boxplot, patch_artist=True,
               boxprops=dict(facecolor='lightblue', color='blue'),
               medianprops=dict(color='red'))

    '''for i, data in enumerate(data_for_boxplot):
        if not data:
            continue
        mean_val = np.mean(data)
        ax.plot([i + 1 - 0.3, i + 1 + 0.3], [mean_val, mean_val], color='green', linestyle='--', linewidth=1)'''

    means = [np.mean(data) if data else 0 for data in data_for_boxplot]
    x = range(1, len(means) + 1)
    ax.plot(x, means, color='green', linestyle='--', marker='o', label='Média')
    ax.legend()

    ax.set_title(f"{output_prefix}")
    ax.set_xlabel("Histograma")
    ax.set_ylabel("Tentativas")
    ax.set_yscale("log")
    ax.set_xticks(range(1, len(data_for_boxplot) + 1))
    ax.set_xticklabels([str(i) for i in range(len(data_for_boxplot))])

    #ax.axhline(mean_val, color='green', linestyle='--', linewidth=1, label=f'Média: {mean_val:.2f}')
    ax.legend(loc='lower right',bbox_to_anchor=(1, 1))

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
        print("working on {}".format(base_name))

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
        plot_heatmap_with_used_mask(heat_end, used_mask, output2, base_name + " Custo de Destino", "Tentativas")

        # Plot heatBegin com máscara
        output1 = os.path.join(folder_path, base_name + "_Begin.jpg")
        plot_heatmap_with_used_mask(heat_begin, used_mask, output1, base_name + " Pontos de Origem", "Posicionamentos")

        origins_to_dests = {int(k): v for k, v in data["orDest"].items()}
        output3 = os.path.join(folder_path, base_name + "_Lines.jpg")
        draw_lines_between_cells(origins_to_dests, used_mask, heat_end, output3, base_name,
                                 "Origens e Destinos com Linhas")

        hist_list = list(data["hist"].values())
        plot_histograms(hist_list, base_name, folder_path, base_name)
        plot_boxplots(hist_list, base_name, folder_path)


'''if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Uso: python script.py <caminho_da_pasta>")
        sys.exit(1)
    folder = sys.argv[1]
    process_json_metrics(folder)'''

if __name__ == "__main__":
    folder = "/home/jeronimo/GIT/PeR/reports/fpga/EPFL/yoto_df_x1_debug/metrics/"
    folder = "/home/jeronimo/GIT/PeR/reports/fpga/TEST/yoto_df_x1_debug/metrics"
    folder_path = os.path.dirname(os.path.abspath(folder))  # Pasta atual
    process_json_metrics(folder)
