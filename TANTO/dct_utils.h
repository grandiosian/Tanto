#include <kos.h>


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

float my_tan(float angle) {
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



void debug_mat(matrix_t mat)
{
    printf("--- dbg matrice --- \n");
    printf("mat 11:%f 12:%f 13:%f 14:%f \n", mat[0][0], mat[0][1], mat[0][2], mat[0][3]);
    printf("mat 21:%f 22:%f 23:%f 24:%f \n", mat[1][0], mat[1][1], mat[1][2], mat[1][3]);
    printf("mat 31:%f 32:%f 33:%f 34:%f \n", mat[2][0], mat[2][1], mat[2][2], mat[2][3]);
    printf("mat 41:%f 42:%f 43:%f 44:%f \n", mat[3][0], mat[3][1], mat[3][2], mat[3][3]);
    printf("------------------- \n");
}