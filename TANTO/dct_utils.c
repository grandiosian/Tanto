#include "dct_utils.h"


void quat_slerp(float result[4], const float q1[4], const float q2[4], float t) {
    float cos_theta = q1[0] * q2[0] + q1[1] * q2[1] + q1[2] * q2[2] + q1[3] * q2[3];

    // Si cos_theta < 0, on inverse un des quaternions pour prendre le chemin le plus court
    if (cos_theta < 0.0f) {
        cos_theta = -cos_theta;
        result[0] = -q2[0];
        result[1] = -q2[1];
        result[2] = -q2[2];
        result[3] = -q2[3];
    } else {
        result[0] = q2[0];
        result[1] = q2[1];
        result[2] = q2[2];
        result[3] = q2[3];
    }

    // Interpolation linéaire si les quaternions sont très proches
    if (cos_theta > 0.9995f) {
        result[0] = q1[0] + t * (result[0] - q1[0]);
        result[1] = q1[1] + t * (result[1] - q1[1]);
        result[2] = q1[2] + t * (result[2] - q1[2]);
        result[3] = q1[3] + t * (result[3] - q1[3]);

        // Normaliser le résultat
        float norm = sqrtf(result[0] * result[0] + result[1] * result[1] + result[2] * result[2] + result[3] * result[3]);
        if (norm > 0.0f) {
            result[0] /= norm;
            result[1] /= norm;
            result[2] /= norm;
            result[3] /= norm;
        }
        return;
    }

    // Sinon, utiliser slerp
    float theta = acosf(cos_theta);
    float sin_theta = sinf(theta);

    float w1 = sinf((1.0f - t) * theta) / sin_theta;
    float w2 = sinf(t * theta) / sin_theta;

    result[0] = q1[0] * w1 + result[0] * w2;
    result[1] = q1[1] * w1 + result[1] * w2;
    result[2] = q1[2] * w1 + result[2] * w2;
    result[3] = q1[3] * w1 + result[3] * w2;
}

void transposeMatrix(float matrix[16]) {
    float temp;

    // Échanger les éléments hors diagonale
    for (int i = 0; i < 4; i++) {
        for (int j = i + 1; j < 4; j++) {
            // Échanger matrix[i][j] et matrix[j][i]
            temp = matrix[i * 4 + j];
            matrix[i * 4 + j] = matrix[j * 4 + i];
            matrix[j * 4 + i] = temp;
        }
    }
}

// Initialiser une nouvelle liste
List* list_create() {
    List* list = (List*)malloc(sizeof(List));
    if(list) {
        list->head = NULL;
        list->tail = NULL;
        list->size = 0;
    }
    return list;
}

// Ajouter un élément à la fin
int list_append(List* list, void* data) {
    if(!list) return 0;
    
    ListNode* node = (ListNode*)malloc(sizeof(ListNode));
    if(!node) return 0;
    
    node->data = data;
    node->next = NULL;
    
    if(list->tail) {
        list->tail->next = node;
        list->tail = node;
    } else {
        list->head = node;
        list->tail = node;
    }
    
    list->size++;
    return 1;
}

// Supprimer un élément
int list_remove(List* list, void* data) {
    if(!list || !list->head) return 0;
    
    ListNode *current = list->head;
    ListNode *prev = NULL;
    
    while(current) {
        if(current->data == data) {
            if(prev) {
                prev->next = current->next;
            } else {
                list->head = current->next;
            }
            
            if(current == list->tail) {
                list->tail = prev;
            }
            
            free(current);
            list->size--;
            return 1;
        }
        prev = current;
        current = current->next;
    }
    return 0;
}

// Libérer la liste
void list_destroy(List* list) {
    if(!list) return;
    
    ListNode* current = list->head;
    while(current) {
        ListNode* next = current->next;
        free(current);
        current = next;
    }
    free(list);
}



void test_memory_limit_binary() {
    size_t total_allocated = 0;
    size_t current_size = 8*1024*1024;  // Commencer par 8MB
    void** blocks = malloc(1000 * sizeof(void*));
    int block_count = 0;
    
    printf("Test allocation mémoire avec recherche binaire...\n");
    
    while(current_size >= 1) {  // Jusqu'à 1 octet
        void* block = malloc(current_size);
        if(block) {
            blocks[block_count++] = block;
            total_allocated += current_size;
            printf("Succès allocation de %lu octets (Total: %lu)\n", current_size, total_allocated);
        } else {
            current_size /= 2;  // Diviser par 2 si échec
        }
    }
    
    printf("\nTotal alloué: %lu octets\n", total_allocated);
    printf("Mémoire système: %lu octets\n", 16*1024*1024 - total_allocated);
    
    for(int i = 0; i < block_count; i++) {
        free(blocks[i]);
    }
    free(blocks);
}


void debug_pvr_vtx(pvr_vertex_t *pvr, int size)
{
    printf("--- debug pvr vtx ---\n");
    for (int i=0; i<size; i++ )
    {
        printf("pvr vtx :%d x%f y%f z%f \n",i,pvr[i].x, pvr[i].y, pvr[i].z);
    }
    printf("--- END DBG PVR VTX --- \n");

}


float normalize_angle(float angle) {
    // Ramène l'angle dans [-2π, 2π]
    const float TWO_PI = 2.0f * F_PI;
    while(angle > TWO_PI)
        angle -= TWO_PI;
    while(angle < -TWO_PI)
        angle += TWO_PI;
    return angle;
}

float dct_tan(float angle) {
    angle = normalize_angle(angle);
    float s = sinf(angle);
    float c = cosf(angle);
    if(c != 0.0f)
        return s/c;
    return 4.0f;  // valeur par défaut
}


void debug_alignment(const char* name, void* ptr, int size) {
    printf("%s:\n", name);
    printf("Address: %p\n", ptr);
    printf("Aligned 32? %s\n", ((uintptr_t)ptr & 31) == 0 ? "yes" : "no");
    printf("Size: %d\n", size);
    
    // Affiche les premières valeurs pour vérifier
    pvr_vertex_t* vtx = (pvr_vertex_t*)ptr;
    for(int i = 0; i < 3 && i < size; i++) {
        printf("Vertex %d: x=%f y=%f z=%f flags=%x\n", 
               i, vtx[i].x, vtx[i].y, vtx[i].z, vtx[i].flags);
    }
    printf("\n");
}


void print_matrix(matrix_t mat) {
    printf("Matrix:\n");
    for(int i = 0; i < 4; i++) {
        printf("[%f %f %f %f]\n", 
            mat[i][0], mat[i][1], mat[i][2], mat[i][3]);
    }
    printf("\n");
}

void print_matrix_f16(float m[16]) {
    // Parcours des lignes de la matrice
    for (int i = 0; i < 4; i++) {
        // Parcours des colonnes de la matrice
        for (int j = 0; j < 4; j++) {
            // Affichage de l'élément à la position (i, j)
            printf("%8.6f ", m[i * 4 + j]);
        }
        // Nouvelle ligne après chaque ligne de la matrice
        printf("\n");
    }
}

void debug_mat(matrix_t mat)
{
    printf("--- dbg matrice --- \n");
    printf("mat 11:%f 12:%f 13:%f 14:%f \n", mat[0][0], mat[0][1], mat[0][2], mat[0][3]);
    printf("mat 21:%f 22:%f 23:%f 24:%f \n", mat[1][0], mat[1][1], mat[1][2], mat[1][3]);
    printf("mat 31:%f 32:%f 33:%f 34:%f \n", mat[2][0], mat[2][1], mat[2][2], mat[2][3]);
    printf("mat 41:%f 42:%f 43:%f 44:%f \n", mat[3][0], mat[3][1], mat[3][2], mat[3][3]);
    printf("------------------- \n");
}
