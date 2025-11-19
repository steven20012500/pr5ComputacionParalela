#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <cstdlib>
#include <cstring>   // para strcmp
#include <omp.h>

#define N 100000000 // Tamaño de la secuencia
const std::string PATTERN = "ATGC"; // Patrón a buscar
const int P_LEN = PATTERN.length();

using namespace std;

// Función para simular la secuencia grande
void generate_sequence(vector<char>& seq) {
    for (int i = 0; i < N - P_LEN; ++i) {
        seq[i] = "ATGC"[rand() % 4];
    }
    // Asegurar patrón cerca del final
    for (int i = 0; i < P_LEN; ++i) {
        seq[N - 10000 + i] = PATTERN[i];
    }
}

void run_search(int num_threads, const char* schedule_type, int chunk_size) {
    vector<char> dna_sequence(N);
    generate_sequence(dna_sequence);

    long long first_index = -1;

    omp_set_num_threads(num_threads);

    // Configurar schedule
    omp_sched_t kind;

    if (strcmp(schedule_type, "static") == 0) {
        kind = omp_sched_static;
    } else if (strcmp(schedule_type, "dynamic") == 0) {
        kind = omp_sched_dynamic;
    } else if (strcmp(schedule_type, "guided") == 0) {
        kind = omp_sched_guided;
    } else {
        kind = omp_sched_auto;
    }

    omp_set_schedule(kind, chunk_size);

    auto start = chrono::high_resolution_clock::now();

    #pragma omp parallel for schedule(runtime)
    for (int i = 0; i < N - P_LEN; ++i) {
        if (first_index != -1) continue;

        bool match = true;
        for (int j = 0; j < P_LEN; ++j) {
            if (dna_sequence[i + j] != PATTERN[j]) {
                match = false;
                break;
            }
        }

        if (match) {
            #pragma omp critical
            {
                if (first_index == -1 || i < first_index) {
                    first_index = i;
                }
            }
        }
    }

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed = end - start;

    cout << "Hilos: " << num_threads
         << ", Schedule: " << schedule_type
         << " (chunk=" << chunk_size << ")"
         << ", Tiempo: " << elapsed.count() << " s"
         << ", Posición encontrada: " << first_index
         << endl;
}

int main() {
    cout << "--- Búsqueda de Patrón en ADN (" << PATTERN << ") ---" << endl;

    int chunk = 10000;

    // Solo 1 y 2 hilos
    int threads_to_test[] = {1, 2};
    const char* schedules[] = {"static", "dynamic", "guided", "auto"};

    for (int t : threads_to_test) {
        for (const char* sch : schedules) {
            run_search(t, sch, chunk);
        }
    }

    return 0;
}
