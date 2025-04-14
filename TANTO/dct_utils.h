#ifndef DCT_UTILS_H
#define DCT_UTILS_H

// Définition des codes couleur
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define ANSI_BOLD         "\x1b[1m"
#define ANSI_UNDERLINE    "\x1b[4m"
#define ANSI_BLINK        "\x1b[5m"
#define ANSI_REVERSE      "\x1b[7m"

#define COLOR_CUSTOM_RGB(r,g,b)  "\x1b[38;2;" #r ";" #g ";" #b "m"
#define BG_CUSTOM_RGB(r,g,b)     "\x1b[48;2;" #r ";" #g ";" #b "m"

#define BLEU_END_FN    "\x1b[38;2;100;120;245m"
#define BLEU_INSIDE_FN    "\x1b[38;2;122;171;246m"
#define BLEU_START_FN    "\x1b[38;2;165;122;245m"

#include <kos.h>
#include <math.h>




// Structure pour un élément de la liste
typedef struct ListNode {
    void* data;                // Données stockées (pointer générique)
    struct ListNode* next;     // Pointeur vers l'élément suivant
} ListNode;

// Structure pour gérer la liste
typedef struct {
    ListNode* head;            // Premier élément
    ListNode* tail;            // Dernier élément
    int size;                  // Taille actuelle
} List;

List* list_create();

void transposeMatrix(float matrix[16]);

int list_append(List* list, void* data);

int list_remove(List* list, void* data);

void list_destroy(List* list);

void test_memory_limit_binary();

void debug_pvr_vtx(pvr_vertex_t *pvr, int size);

float normalize_angle(float angle);

float dct_tan(float angle);

void debug_alignment(const char* name, void* ptr, int size);

void print_matrix(matrix_t mat);

void print_matrix_f16(float m[16]);

void debug_mat(matrix_t mat);


#endif