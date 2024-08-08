#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

#define GRID_SIZE 20
#define NUM_TICKS 1

typedef enum {
    EMPTY,
    PLANT,
    HERBIVORE,
    CARNIVORE
} CellType;

typedef struct {
    CellType type;
    int energy;
} Cell;

Cell grid[GRID_SIZE][GRID_SIZE];

void initialize_ecosystem() {
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            int random = rand() % 100;
            if (random < 60) {
                grid[i][j].type = EMPTY;
            } else if (random < 85) {
                grid[i][j].type = PLANT;
                grid[i][j].energy = 1;
            } else if (random < 95) {
                grid[i][j].type = HERBIVORE;
                grid[i][j].energy = 5;
            } else {
                grid[i][j].type = CARNIVORE;
                grid[i][j].energy = 10;
            }
        }
    }
}

void simulate_tick() {
    Cell new_grid[GRID_SIZE][GRID_SIZE];
    
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            new_grid[i][j] = grid[i][j];
            
            switch (grid[i][j].type) {
                case PLANT:
                    // Lógica para plantas
                    if (grid[i][j].energy < 3) {
                        grid[i][j].energy++;
                    } else if (rand() % 100 < 30) {  // 30% de probabilidad de reproducción
                        // Buscar una celda vacía adyacente
                        int di = rand() % 3 - 1;  // -1, 0, o 1
                        int dj = rand() % 3 - 1;  // -1, 0, o 1
                        int ni = i + di, nj = j + dj;
                        if (ni >= 0 && ni < GRID_SIZE && nj >= 0 && nj < GRID_SIZE && grid[ni][nj].type == EMPTY) {
                            new_grid[ni][nj].type = PLANT;
                            new_grid[ni][nj].energy = 1;
                        }
                    }
                    break;
                case HERBIVORE:
                    // Lógica para herbívoros
                    new_grid[i][j].energy--;
                    if (new_grid[i][j].energy <= 0) {
                        new_grid[i][j].type = EMPTY;
                    } else {
                        // Movimiento y alimentación
                        int di = rand() % 3 - 1;
                        int dj = rand() % 3 - 1;
                        int ni = i + di, nj = j + dj;
                        if (ni >= 0 && ni < GRID_SIZE && nj >= 0 && nj < GRID_SIZE) {
                            if (grid[ni][nj].type == PLANT) {
                                new_grid[i][j].energy += 2;
                                new_grid[ni][nj].type = EMPTY;
                            } else if (grid[ni][nj].type == EMPTY) {
                                new_grid[ni][nj] = new_grid[i][j];
                                new_grid[i][j].type = EMPTY;
                            }
                        }
                    }
                    break;
                case CARNIVORE:
                    // Lógica para carnívoros (similar a herbívoros, pero cazan herbívoros)
                    new_grid[i][j].energy--;
                    if (new_grid[i][j].energy <= 0) {
                        new_grid[i][j].type = EMPTY;
                    } else {
                        int di = rand() % 3 - 1;
                        int dj = rand() % 3 - 1;
                        int ni = i + di, nj = j + dj;
                        if (ni >= 0 && ni < GRID_SIZE && nj >= 0 && nj < GRID_SIZE) {
                            if (grid[ni][nj].type == HERBIVORE) {
                                new_grid[i][j].energy += 4;
                                new_grid[ni][nj] = new_grid[i][j];
                                new_grid[i][j].type = EMPTY;
                            } else if (grid[ni][nj].type == EMPTY) {
                                new_grid[ni][nj] = new_grid[i][j];
                                new_grid[i][j].type = EMPTY;
                            }
                        }
                    }
                    break;
                default:
                    break;
            }
        }
    }
    
    // Actualizar el grid principal
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            grid[i][j] = new_grid[i][j];
        }
    }
}

void print_ecosystem_state(int tick) {
    int plants = 0, herbivores = 0, carnivores = 0;
    
    // Contar la población de cada especie
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            switch (grid[i][j].type) {
                case PLANT: plants++; break;
                case HERBIVORE: herbivores++; break;
                case CARNIVORE: carnivores++; break;
                default: break;
            }
        }
    }
    
    // Imprimir resumen
    printf("Tick %d\n", tick);
    printf("Plantas: %d\n", plants);
    printf("Herbívoros: %d\n", herbivores);
    printf("Carnívoros: %d\n", carnivores);
    printf("Distribución:\n");
    
    // Imprimir la cuadrícula
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            switch (grid[i][j].type) {
                case EMPTY: printf(". "); break;
                case PLANT: printf("P "); break;
                case HERBIVORE: printf("H "); break;
                case CARNIVORE: printf("C "); break;
            }
        }
        printf("\n");
    }
    printf("\n");
}

int main() {
    srand(time(NULL));
    initialize_ecosystem();
    
    for (int tick = 0; tick < NUM_TICKS; tick++) {
        simulate_tick();
        print_ecosystem_state(tick);
    }
    
    return 0;
}
