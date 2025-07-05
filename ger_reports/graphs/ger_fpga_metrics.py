
import json
import os
import matplotlib.pyplot as plt
import numpy as np
from matplotlib.colors import Normalize
from matplotlib.colors import LinearSegmentedColormap, Normalize



def plot_heatmap_with_used_mask(data, used_mask, output_path, title="", leg_label =""):

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
        plt.cm.ScalarMappable(norm=norm,cmap=custom_cmap),
        ax=ax,
        shrink=0.8,
        orientation='vertical',
        pad=0.02
    )
    cbar.set_label(leg_label)

    plt.savefig(output_path, dpi=300, bbox_inches="tight")
    plt.close()



def process_json_heatmaps_with_mask(folder_path):
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
        plot_heatmap_with_used_mask(heat_end, used_mask, output2, base_name + " Custo de Destino", "Tentativas")

        # Plot heatBegin com máscara
        output1 = os.path.join(folder_path, base_name + "_Begin.jpg")
        plot_heatmap_with_used_mask(heat_begin, used_mask, output1, base_name + " Pontos de Origem", "Posicionamentos")



def generate_heatmaps_from_json_folder(folder_path):
    json_files = [f for f in os.listdir(folder_path) if f.endswith(".json")]

    for filename in json_files:
        filepath = os.path.join(folder_path, filename)
        with open(filepath, "r") as f:
            data = json.load(f)

        for key in ["heatBegin", "heatEnd"]:
            if key in data:
                vector = data[key]
                n = int(len(vector) ** 0.5)
                if n * n != len(vector):
                    print(f"Aviso: vetor {key} em {filename} não forma uma matriz quadrada.")
                    continue

                matrix = np.array(vector).reshape((n, n))
                vmax = np.max(matrix)

                fig, ax = plt.subplots(figsize=(6, 6))
                im = ax.imshow(matrix, cmap="Reds", interpolation="nearest", vmin=0, vmax=vmax)

                # Grade branca fina
                ax.set_xticks(np.arange(-0.5, n, 1), minor=True)
                ax.set_yticks(np.arange(-0.5, n, 1), minor=True)
                ax.grid(which="minor", color="white", linewidth=0.5)
                ax.tick_params(which="both", bottom=False, left=False, labelbottom=False, labelleft=False)

                plt.colorbar(im, ax=ax, fraction=0.046, pad=0.04)
                plt.title(f"{key} - {filename}", fontsize=10)

                output_name = os.path.splitext(filename)[0] + f"_{'Begin' if key == 'heatBegin' else 'End'}.jpg"
                output_path = os.path.join(folder_path, output_name)
                plt.savefig(output_path, bbox_inches="tight", dpi=300)
                plt.close()

        print(f"Imagens geradas para {filename}.")


# Uso direto
if __name__ == "__main__":
    import sys

    folder = "/home/jeronimo/GIT/PeR/reports/fpga/EPFL/yoto_df_x1_debug/metrics/"
    # folder_path = os.path.dirname(os.path.abspath(__file__))  # Pasta atual
    folder_path = os.path.dirname(os.path.abspath(folder))  # Pasta atual
    process_json_heatmaps_with_mask(folder)
    # generate_heatmaps_from_json_folder(folder)
