#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <algorithm>
#include <cstdlib>

void merge(std::vector<int> &v, std::vector<int> &aux,
           int esquerda, int meio, int direita) {
    int i = esquerda;
    int j = meio + 1;
    int k = esquerda;

    while (i <= meio && j <= direita) {
        int a = v[i], b = v[j];
        if (v[i] <= v[j]) {
            aux[k++] = v[i++];
        } else {
            aux[k++] = v[j++];
        }
    }

    while (i <= meio) {
        aux[k++] = v[i++];
    }

    while (j <= direita) {
        aux[k++] = v[j++];
    }

    for (int p = esquerda; p <= direita; p++) {
        v[p] = aux[p];
    }
}

void mergesort_sequencial(std::vector<int> &v, std::vector<int> &aux,
                          int esquerda, int direita) {
    if (esquerda >= direita) {
        return;
    }

    int meio = esquerda + (direita - esquerda) / 2;

    mergesort_sequencial(v, aux, esquerda, meio);
    mergesort_sequencial(v, aux, meio + 1, direita);

    merge(v, aux, esquerda, meio, direita);
}

bool esta_ordenado(const std::vector<int> &v) {
    for (size_t i = 1; i < v.size(); i++) {
        if (v[i - 1] > v[i]) {
            return false;
        }
    }

    return true;
}

int main(int argc, char *argv[]) {
    int N = 20;

    if (argc >= 2) {
        N = std::atoi(argv[1]);
    }

    std::vector<int> dados(N);
    std::vector<int> auxiliar(N);

    std::mt19937 gerador(42);
    std::uniform_int_distribution<int> dist(0, 1000000000);

    for (int i = 0; i < N; i++) {
        dados[i] = dist(gerador);
    }

    auto inicio = std::chrono::high_resolution_clock::now();

    mergesort_sequencial(dados, auxiliar, 0, N - 1);

    auto fim = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> tempo = fim - inicio;

    std::cout << "N = " << N << "\n";
    std::cout << "Ordenado = " << (esta_ordenado(dados) ? "sim" : "nao") << "\n";
    std::cout << "Tempo = " << tempo.count() << " s\n";

    return 0;
}
