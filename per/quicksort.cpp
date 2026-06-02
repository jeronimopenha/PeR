#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <algorithm>
#include <cstdlib>

int particionar(std::vector<int>& v, int esquerda, int direita) {
    int pivo = v[direita];
    int i = esquerda - 1;

    for (int j = esquerda; j < direita; j++) {
        if (v[j] <= pivo) {
            i++;
            std::swap(v[i], v[j]);
        }
    }

    std::swap(v[i + 1], v[direita]);
    return i + 1;
}

void quicksort_sequencial(std::vector<int>& v, int esquerda, int direita) {
    if (esquerda >= direita) {
        return;
    }

    int p = particionar(v, esquerda, direita);

    quicksort_sequencial(v, esquerda, p - 1);
    quicksort_sequencial(v, p + 1, direita);
}

bool esta_ordenado(const std::vector<int>& v) {
    for (size_t i = 1; i < v.size(); i++) {
        if (v[i - 1] > v[i]) {
            return false;
        }
    }

    return true;
}

int main(int argc, char* argv[]) {
    int N = 10000000;

    if (argc >= 2) {
        N = std::atoi(argv[1]);
    }

    std::vector<int> dados(N);

    std::mt19937 gerador(42);
    std::uniform_int_distribution<int> dist(0, 1000000000);

    for (int i = 0; i < N; i++) {
        dados[i] = dist(gerador);
    }

    auto inicio = std::chrono::high_resolution_clock::now();

    quicksort_sequencial(dados, 0, N - 1);

    auto fim = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> tempo = fim - inicio;

    std::cout << "N = " << N << "\n";
    std::cout << "Ordenado = " << (esta_ordenado(dados) ? "sim" : "nao") << "\n";
    std::cout << "Tempo = " << tempo.count() << " s\n";

    return 0;
}