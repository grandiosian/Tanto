#ifndef DCT_UTILS_H
#define DCT_UTILS_H

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

int list_append(List* list, void* data);

int list_remove(List* list, void* data);

void list_destroy(List* list);

void test_memory_limit_binary();

void debug_pvr_vtx(pvr_vertex_t *pvr, int size);

float normalize_angle(float angle);

float dct_tan(float angle);

void debug_alignment(const char* name, void* ptr, int size);

void print_matrix(matrix_t mat);

void debug_mat(matrix_t mat);


#endif