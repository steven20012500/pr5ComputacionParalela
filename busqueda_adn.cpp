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

// Función para simular una secuencia grande
void generate_sequence(vector<char>& seq) {
    // Rellenamos con bases aleatorias para simular una carga de trabajo real
    // Insertamos el patrón cerca del final para forzar la búsqueda larga
    for (int i = 0; i < N - P_LEN; ++i) {
        seq[i] = "ATGC"[rand() % 4];
    }

    // Aseguramos que el patrón exista cerca del final
    for (int i = 0; i < P_LEN; ++i) {
        seq[N - 10000 + i] = PATTERN[i];
    }
}

// num_threads: cantidad de hilos
// schedule_type: "static", "dynamic", "guided" o "auto"
// chunk_size: tamaño de bloque para el schedule
void run_search(int num_threads, const char* schedule_type, int chunk_size) {
    vector<char> dna_sequence(N);
    generate_sequence(dna_sequence);

    long long first_index = -1;

    omp_set_num_threads(num_threads);

    // Configuramos el tipo de schedule en tiempo de ejecución
    omp_sched_t kind;

    if (strcmp(schedule_type, "static") == 0) {
        kind = omp_sched_static;
    } else if (strcmp(schedule_type, "dynamic") == 0) {
        kind = omp_sched_dynamic;
    } else if (strcmp(schedule_type, "guided") == 0) {
        kind = omp_sched_guided;
    } else { 
        // "auto" u otro valor por defecto
        kind = omp_sched_auto;
    }

    // Asignar schedule en runtime
    omp_set_schedule(kind, chunk_size);

    auto start = chrono::high_resolution_clock::now();

    // PCAM: Partitioning & Agglomeration: cada hilo procesa un rango de índices
    #pragma omp parallel for schedule(runtime)
    for (int i = 0; i < N - P_LEN; ++i) {
        // Si ya se encontró el índice, intentamos salir rápido
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

    int chunk = 10000; // puedes probar otros tamaños (1000, 50000, etc.)

    // ########### EJECUCIONES DE PRUEBA ############
    // STATIC
    run_search(2,  "static", chunk);
    run_search(4,  "static", chunk);
    run_search(8,  "static", chunk);

    // DYNAMIC
    run_search(2,  "dynamic", chunk);
    run_search(4,  "dynamic", chunk);
    run_search(8,  "dynamic", chunk);

    // GUIDED
    run_search(2,  "guided", chunk);
    run_search(4,  "guided", chunk);
    run_search(8,  "guided", chunk);

    // AUTO
    run_search(2,  "auto", chunk);
    run_search(4,  "auto", chunk);
    run_search(8,  "auto", chunk);
    // ##############################################

    return 0;
}
