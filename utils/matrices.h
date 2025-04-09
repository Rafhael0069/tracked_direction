#ifndef MATRICES_H
#define MATRICES_H

#include "matrix_control.h"

#define MATRIX_SIZE 5

// Padrão central para ociosidade

int center_pattern[MATRIX_SIZE][MATRIX_SIZE] = {
    {0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0},
    {0, 0, 1, 0, 0},
    {0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0}
};

// Seta para Norte (Cima)
int north_arrow[MATRIX_SIZE][MATRIX_SIZE] = {
    {0, 0, 1, 0, 0},
    {0, 0, 1, 0, 0},
    {1, 0, 1, 0, 1},
    {0, 1, 1, 1, 0},
    {0, 0, 1, 0, 0}
};

// Seta para Noroeste (Diagonal superior direito)
int northwest_arrow[MATRIX_SIZE][MATRIX_SIZE] = {
    {0, 0, 0, 0, 1},
    {0, 1, 0, 0, 1},
    {1, 0, 1, 0, 0},
    {0, 0, 0, 1, 1},
    {1, 1, 1, 1, 0}
};

// Seta para Leste (Direita)
int east_arrow[MATRIX_SIZE][MATRIX_SIZE] = {
    {0, 0, 1, 0, 0},
    {0, 0, 0, 1, 0},
    {1, 1, 1, 1, 1},
    {0, 0, 0, 1, 0},
    {0, 0, 1, 0, 0}
};

// Seta para Sudoeste (Diagonal inferior direito)
int southwest_arrow[MATRIX_SIZE][MATRIX_SIZE] = {
    {1, 1, 1, 1, 0},
    {0, 0, 0, 1, 1},
    {1, 0, 1, 0, 0},
    {0, 1, 0, 0, 1},
    {0, 0, 0, 0, 1}
};


// Seta para Sul (Baixo)
int south_arrow[MATRIX_SIZE][MATRIX_SIZE] = {
    {0, 0, 1, 0, 0},
    {0, 1, 1, 1, 0},
    {1, 0, 1, 0, 1},
    {0, 0, 1, 0, 0},
    {0, 0, 1, 0, 0}
};

// Seta para Sudeste (Diagonal inferior esquerdo)
int southeast_arrow[MATRIX_SIZE][MATRIX_SIZE] = {
    {0, 1, 1, 1, 1},
    {1, 1, 0, 0, 0},
    {0, 0, 1, 0, 1},
    {1, 0, 0, 1, 0},
    {1, 0, 0, 0, 0}
};

// Seta para Oeste (Esquerda)
int west_arrow[MATRIX_SIZE][MATRIX_SIZE] = {
    {0, 0, 1, 0, 0},
    {0, 1, 0, 0, 0},
    {1, 1, 1, 1, 1},
    {0, 1, 0, 0, 0},
    {0, 0, 1, 0, 0}
};

// Seta para Nordeste (Diagonal superior esquerdo)
int northeast_arrow[MATRIX_SIZE][MATRIX_SIZE] = {
    {1, 0, 0, 0, 0},
    {1, 0, 0, 1, 0},
    {0, 0, 1, 0, 1},
    {1, 1, 0, 0, 0},
    {0, 1, 1, 1, 1}
};

#endif // MATRICES_H