#ifndef DCT_MODEL_H
#define DCT_MODEL_H

#include <kos.h>
#include <kmg/kmg.h>
#include <math.h>
#include "dct_utils.h"
#include "dct_camera.h"
#include "dct_types.h"
#include "dct_texture.h"




typedef struct dct_mesh_t 
{
    int          indice;
    char         *name;     

    int          vtxCount;
    int          sizeOfVtx;      // sizeof(originalVtx) pour optimiser sq_cpy et ne pas avoir à le calculer à chaque frame
    pvr_vertex_t *originalVtx;   // les vertex en "bindPose"
    pvr_vertex_t *animatedVtx;   // les vertex animés par le squelette
    pvr_vertex_t *renderVtx;     // les vertex transformés par mat_trans_single3 afin d'eviter une double transformation constante car mat_trans_single incrémente les vertex en input

    pvr_vertex_t  *clippingTrianglesVtx;
    int           clippingTrianglesCount;
    int           currentClippingTrianglesCount; // 1 ou 2 triangle a dessiné en plus pour chaque triangle

    pvr_poly_hdr_t      hdr; 
    pvr_poly_cxt_t      cxt;

} dct_mesh_t;





typedef struct dct_model_t
{
    char            *name;

    int             meshesCount;
    dct_mesh_t      *meshes;

    dct_texture_t   texture01;
    
    vec3f_t         position;
    vec3f_t         rotation;
    vec3f_t         scale;

    

    bool            DEBUG_MODEL_VTX;

} dct_model_t;



void createModel(const char *filename);
void initModel( dct_model_t *model);
void setMeshHeader(dct_mesh_t *mesh, int SETTING);
void initMeshesHeader(dct_model_t *model);
void setVertexColorToAllMesh(dct_mesh_t *mesh, uint32_t col);
void displayModelData(dct_model_t *model);
void updateModel( dct_model_t *model );
void renderMesh(dct_mesh_t *mesh);
void renderModel(dct_model_t *model);
void freeModel(dct_model_t *model);

void createPrimitiveTriangle(dct_model_t *triangle, const char *name, float x1, float y1, float z1, float x2, float y2, float z2 ,float x3, float y3, float z3, uint32_t color[3]);
void createPrimitiveRect(dct_model_t *rect, const char *name,float x, float y, float width, float height,uint32_t color);
void createPrimitiveLine(dct_model_t *line, const char *name,float x1, float y1, float x2, float y2,float thickness, uint32_t color);
void createPrimitiveCircle(dct_model_t *circle, const char *name,float posX, float posY, float radius,int segments, uint32_t color);
void createPrimitivePolygon(dct_model_t *polygon, const char *name, float *points, int numPoints, uint32_t color);
void createPrimitivePath(dct_model_t *path, const char *name, float *points, int numPoints, float thickness, uint32_t color);
void createCube(dct_model_t *cube, const char *name, float size, uint32_t color[6]);
void createPentahedre(dct_model_t *penta, const char *name, float size, uint32_t colors[5]);
void createSphere(dct_model_t *sphere, const char *name ,float size, uint32_t color);

#endif