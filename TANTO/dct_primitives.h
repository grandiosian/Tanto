#include <kos.h>
#include "dct_model.h"




typedef struct primitive_cube_t
{
    pvr_model_t  model;

} primitive_cube_t;



primitive_cube_t* createCube()
{
    printf("\n{ START >>> [CREATE CUBE] \n");

    primitive_cube_t *cube;

    cube = (primitive_cube_t*)malloc(sizeof(primitive_cube_t));


    //
    //             DRAW CCW => hdr culling CW
    //                  FACE AVANT QUAD 
    //                      vertex Top 
    //                     1---------4 Y-
    //                     |         | 
    //                     |         |
    //                     |         |
    //                     2 ------- 3
    //              X-   vgauche    vdroite X+
    //     

    const pvr_vertex_t rawdata[] = {
        // FACE AVANT
        {PVR_CMD_VERTEX,    -1,   -1,     1.0f, 0.0f, 0.0f, 0xFFFF0000, 0.0f},
        {PVR_CMD_VERTEX,    -1,    1,     1.0f, 0.0f, 0.0f, 0xFF00FF00, 0.0f},
        {PVR_CMD_VERTEX_EOL, 1,    1,     1.0f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
        {PVR_CMD_VERTEX,     1.0f, 1.0f,  1.0f, 0.0f, 0.0f, 0xFF00FF00, 0.0f},
        {PVR_CMD_VERTEX,     1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
        {PVR_CMD_VERTEX_EOL, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f,0xFFFF0000, 0.0f},

        // FACE ARRIERE
        {PVR_CMD_VERTEX,      1,  -1,     -1.0f, 0.0f, 0.0f, 0xFFFFFFFF, 0.0f},
        {PVR_CMD_VERTEX,      1,   1,     -1.0f, 0.0f, 0.0f, 0xFFFFFFFF, 0.0f},
        {PVR_CMD_VERTEX_EOL, -1,   1,     -1.0f, 0.0f, 0.0f, 0xFFFFFFFF, 0.0f},
        {PVR_CMD_VERTEX,     -1.0f, 1.0f,  -1.0f, 0.0f, 0.0f, 0xFFFFFFFF, 0.0f},
        {PVR_CMD_VERTEX,     -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0xFFFFFFFF, 0.0f},
        {PVR_CMD_VERTEX_EOL, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f,0xFFFFFFFF, 0.0f},


    };
    



    cube->model.name = malloc(sizeof(char)*10);
    sprintf(cube->model.name,"CUBE");
    cube->model.meshesCount = 1;
    cube->model.meshes = (pvr_mesh_t*)malloc(sizeof(pvr_mesh_t));
    cube->model.meshes[0].vtxCount = sizeof(rawdata)/sizeof(pvr_vertex_t);
    cube->model.meshes[0].sizeOfVtx = cube->model.meshes[0].vtxCount*sizeof(pvr_vertex_t);
    cube->model.meshes[0].originalVtx = (pvr_vertex_t*)memalign(32, cube->model.meshes[0].vtxCount*sizeof(pvr_vertex_t));
    cube->model.meshes[0].animatedVtx = (pvr_vertex_t*)memalign(32, cube->model.meshes[0].vtxCount*sizeof(pvr_vertex_t));
    cube->model.meshes[0].renderVtx = (pvr_vertex_t*)memalign(32, cube->model.meshes[0].vtxCount*sizeof(pvr_vertex_t));

    memcpy(cube->model.meshes[0].originalVtx, rawdata , cube->model.meshes[0].sizeOfVtx);
    memcpy(cube->model.meshes[0].animatedVtx, rawdata , cube->model.meshes[0].sizeOfVtx);
    memcpy(cube->model.meshes[0].renderVtx, rawdata , cube->model.meshes[0].sizeOfVtx);
    //memcpy(penta.model.meshes[0].renderVtx, penta.model.meshes[0].originalVtx , sizeof(rawdata));
    
    initModel(&cube->model);
    displayModelData(&cube->model);

    printf("\n[CREATE CUBE] <<< END }\n");
    return cube;
}








typedef struct primitive_pentahedre_t
{
    pvr_model_t  model;

} primitive_pentahedre_t;


primitive_pentahedre_t* createPentahedre()
{
    printf("{ START >>> [CREATE PENTAHEDRE] \n");

    primitive_pentahedre_t *penta;

    penta = (primitive_pentahedre_t*)malloc(sizeof(primitive_pentahedre_t));


    // DRAW CCW => hdr culling CW
    //                  FACE AVANT TRIANGLE 
    //                      vertex Top 
    //                          1 Y-
    //                         / \     
    //                        /   \   
    //                       /     \ 
    //                     2 ------- 3
    //              X-   vgauche    vdroite X+
    //      


    const pvr_vertex_t rawdata[18] = {
        // FACE AV 
        {PVR_CMD_VERTEX,     0, -1,  0.0f, 0.0f, 0.0f, 0xFFFF0000, 0.0f},
        {PVR_CMD_VERTEX,    -1,  1,  1.0f, 0.0f, 0.0f, 0xFFFF0000, 0.0f},
        {PVR_CMD_VERTEX_EOL, 1, 1,   1.0f, 0.0f, 0.0f, 0xFFFF0000, 0.0f},
        // FACE RIGHT
        {PVR_CMD_VERTEX,     0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0xFF00FF00, 0.0f},
        {PVR_CMD_VERTEX,     1.0f, 1.0f,  1.0f, 0.0f, 0.0f, 0xFF00FF00, 0.0f},
        {PVR_CMD_VERTEX_EOL, 1.0f, 1.0f,  -1.0f, 0.0f, 0.0f, 0xFF00FF00, 0.0f},
        // FACE BACK
        {PVR_CMD_VERTEX,     0.0f,-1.0f, 0.0f, 0.0f, 0.0f, 0xFFFF0000, 0.0f},
        {PVR_CMD_VERTEX,     1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0xFF00FF00, 0.0f},
        {PVR_CMD_VERTEX_EOL, -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
        // FACE LEFT
        {PVR_CMD_VERTEX,     0.0f,-1.0f,  0.0f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
        {PVR_CMD_VERTEX,    -1.0f, 1.0f,  -1.0f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
        {PVR_CMD_VERTEX_EOL, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
        // FACE BOTTOM 2 FACES 1 SQUARE
        {PVR_CMD_VERTEX,      -1.0f, 1.0f,  1.0f, 0.0f, 0.0f, 0xFFFFFFFF, 0.0f},
        {PVR_CMD_VERTEX,      -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0xFFFFFFFF, 0.0f},
        {PVR_CMD_VERTEX_EOL,   1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0xFFFFFFFF, 0.0f},
        {PVR_CMD_VERTEX,       1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0xFFFFFFFF, 0.0f},
        {PVR_CMD_VERTEX,      -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0xFFFFFFFF, 0.0f},
        {PVR_CMD_VERTEX_EOL,   1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0xFFFFFFFF, 0.0f}
    };
    



    penta->model.name = malloc(sizeof(char)*10);
    sprintf(penta->model.name,"PENTAHEDRE");
    penta->model.meshesCount = 1;
    penta->model.meshes = (pvr_mesh_t*)malloc(sizeof(pvr_mesh_t));
    penta->model.meshes[0].vtxCount = 18;
    penta->model.meshes[0].sizeOfVtx = penta->model.meshes[0].vtxCount*sizeof(pvr_vertex_t);
    penta->model.meshes[0].originalVtx = (pvr_vertex_t*)memalign(32, penta->model.meshes[0].vtxCount*sizeof(pvr_vertex_t));
    penta->model.meshes[0].animatedVtx = (pvr_vertex_t*)memalign(32, penta->model.meshes[0].vtxCount*sizeof(pvr_vertex_t));
    penta->model.meshes[0].renderVtx = (pvr_vertex_t*)memalign(32, penta->model.meshes[0].vtxCount*sizeof(pvr_vertex_t));

    memcpy(penta->model.meshes[0].originalVtx, rawdata , penta->model.meshes[0].sizeOfVtx);
    memcpy(penta->model.meshes[0].animatedVtx, rawdata , penta->model.meshes[0].sizeOfVtx);
    memcpy(penta->model.meshes[0].renderVtx, rawdata , penta->model.meshes[0].sizeOfVtx);


    initModel(&penta->model);
    displayModelData(&penta->model);

    printf("\n[CREATE PENTAHEDRE] <<< END }\n");
    return penta;
}





typedef struct primitive_sphere_t
{
    pvr_model_t model;

} primitive_sphere_t;

primitive_sphere_t* createSphere()
{
    printf("\n{ START >>> [CREATE SPHERE] \n");

    primitive_sphere_t *sphere;

    sphere = (primitive_sphere_t*)malloc(sizeof(primitive_sphere_t));

    const pvr_vertex_t sphere_data[] = {
    {PVR_CMD_VERTEX, 0.0000f, 1.0000f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.0000f, 1.0000f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, 0.3827f, 0.9239f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.0000f, 1.0000f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.2706f, 0.9239f, 0.2706f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, 0.3827f, 0.9239f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.0000f, 1.0000f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.0000f, 1.0000f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, 0.2706f, 0.9239f, 0.2706f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.0000f, 1.0000f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.0000f, 0.9239f, 0.3827f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, 0.2706f, 0.9239f, 0.2706f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.0000f, 1.0000f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.0000f, 1.0000f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, 0.0000f, 0.9239f, 0.3827f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.0000f, 1.0000f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.2706f, 0.9239f, 0.2706f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, 0.0000f, 0.9239f, 0.3827f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.0000f, 1.0000f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.0000f, 1.0000f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, -0.2706f, 0.9239f, 0.2706f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.0000f, 1.0000f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.3827f, 0.9239f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, -0.2706f, 0.9239f, 0.2706f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.0000f, 1.0000f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.0000f, 1.0000f, -0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, -0.3827f, 0.9239f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.0000f, 1.0000f, -0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.2706f, 0.9239f, -0.2706f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, -0.3827f, 0.9239f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.0000f, 1.0000f, -0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.0000f, 1.0000f, -0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, -0.2706f, 0.9239f, -0.2706f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.0000f, 1.0000f, -0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.0000f, 0.9239f, -0.3827f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, -0.2706f, 0.9239f, -0.2706f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.0000f, 1.0000f, -0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.0000f, 1.0000f, -0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, -0.0000f, 0.9239f, -0.3827f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.0000f, 1.0000f, -0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.2706f, 0.9239f, -0.2706f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, -0.0000f, 0.9239f, -0.3827f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.0000f, 1.0000f, -0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.0000f, 1.0000f, -0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, 0.2706f, 0.9239f, -0.2706f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.0000f, 1.0000f, -0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.3827f, 0.9239f, -0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, 0.2706f, 0.9239f, -0.2706f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.3827f, 0.9239f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.2706f, 0.9239f, 0.2706f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, 0.7071f, 0.7071f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.2706f, 0.9239f, 0.2706f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.5000f, 0.7071f, 0.5000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, 0.7071f, 0.7071f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.2706f, 0.9239f, 0.2706f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.0000f, 0.9239f, 0.3827f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, 0.5000f, 0.7071f, 0.5000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.0000f, 0.9239f, 0.3827f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.0000f, 0.7071f, 0.7071f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, 0.5000f, 0.7071f, 0.5000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.0000f, 0.9239f, 0.3827f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.2706f, 0.9239f, 0.2706f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, 0.0000f, 0.7071f, 0.7071f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.2706f, 0.9239f, 0.2706f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.5000f, 0.7071f, 0.5000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, 0.0000f, 0.7071f, 0.7071f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.2706f, 0.9239f, 0.2706f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.3827f, 0.9239f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, -0.5000f, 0.7071f, 0.5000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.3827f, 0.9239f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.7071f, 0.7071f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, -0.5000f, 0.7071f, 0.5000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.3827f, 0.9239f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.2706f, 0.9239f, -0.2706f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, -0.7071f, 0.7071f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.2706f, 0.9239f, -0.2706f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.5000f, 0.7071f, -0.5000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, -0.7071f, 0.7071f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.2706f, 0.9239f, -0.2706f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.0000f, 0.9239f, -0.3827f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, -0.5000f, 0.7071f, -0.5000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.0000f, 0.9239f, -0.3827f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.0000f, 0.7071f, -0.7071f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, -0.5000f, 0.7071f, -0.5000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.0000f, 0.9239f, -0.3827f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.2706f, 0.9239f, -0.2706f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, -0.0000f, 0.7071f, -0.7071f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.2706f, 0.9239f, -0.2706f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.5000f, 0.7071f, -0.5000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, -0.0000f, 0.7071f, -0.7071f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.2706f, 0.9239f, -0.2706f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.3827f, 0.9239f, -0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, 0.5000f, 0.7071f, -0.5000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.3827f, 0.9239f, -0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.7071f, 0.7071f, -0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, 0.5000f, 0.7071f, -0.5000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.7071f, 0.7071f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.5000f, 0.7071f, 0.5000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, 0.9239f, 0.3827f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.5000f, 0.7071f, 0.5000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.6533f, 0.3827f, 0.6533f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, 0.9239f, 0.3827f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.5000f, 0.7071f, 0.5000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.0000f, 0.7071f, 0.7071f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, 0.6533f, 0.3827f, 0.6533f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.0000f, 0.7071f, 0.7071f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.0000f, 0.3827f, 0.9239f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, 0.6533f, 0.3827f, 0.6533f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.0000f, 0.7071f, 0.7071f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.5000f, 0.7071f, 0.5000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, 0.0000f, 0.3827f, 0.9239f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.5000f, 0.7071f, 0.5000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.6533f, 0.3827f, 0.6533f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, 0.0000f, 0.3827f, 0.9239f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.5000f, 0.7071f, 0.5000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.7071f, 0.7071f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, -0.6533f, 0.3827f, 0.6533f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.7071f, 0.7071f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.9239f, 0.3827f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, -0.6533f, 0.3827f, 0.6533f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.7071f, 0.7071f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.5000f, 0.7071f, -0.5000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, -0.9239f, 0.3827f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.5000f, 0.7071f, -0.5000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.6533f, 0.3827f, -0.6533f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, -0.9239f, 0.3827f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.5000f, 0.7071f, -0.5000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.0000f, 0.7071f, -0.7071f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, -0.6533f, 0.3827f, -0.6533f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.0000f, 0.7071f, -0.7071f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.0000f, 0.3827f, -0.9239f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, -0.6533f, 0.3827f, -0.6533f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.0000f, 0.7071f, -0.7071f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.5000f, 0.7071f, -0.5000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, -0.0000f, 0.3827f, -0.9239f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.5000f, 0.7071f, -0.5000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.6533f, 0.3827f, -0.6533f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, -0.0000f, 0.3827f, -0.9239f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.5000f, 0.7071f, -0.5000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.7071f, 0.7071f, -0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, 0.6533f, 0.3827f, -0.6533f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.7071f, 0.7071f, -0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.9239f, 0.3827f, -0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, 0.6533f, 0.3827f, -0.6533f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.9239f, 0.3827f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.6533f, 0.3827f, 0.6533f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, 1.0000f, 0.0000f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.6533f, 0.3827f, 0.6533f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.7071f, 0.0000f, 0.7071f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, 1.0000f, 0.0000f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.6533f, 0.3827f, 0.6533f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.0000f, 0.3827f, 0.9239f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, 0.7071f, 0.0000f, 0.7071f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.0000f, 0.3827f, 0.9239f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.0000f, 0.0000f, 1.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, 0.7071f, 0.0000f, 0.7071f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.0000f, 0.3827f, 0.9239f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.6533f, 0.3827f, 0.6533f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, 0.0000f, 0.0000f, 1.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.6533f, 0.3827f, 0.6533f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.7071f, 0.0000f, 0.7071f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, 0.0000f, 0.0000f, 1.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.6533f, 0.3827f, 0.6533f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.9239f, 0.3827f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, -0.7071f, 0.0000f, 0.7071f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.9239f, 0.3827f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -1.0000f, 0.0000f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, -0.7071f, 0.0000f, 0.7071f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.9239f, 0.3827f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.6533f, 0.3827f, -0.6533f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, -1.0000f, 0.0000f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.6533f, 0.3827f, -0.6533f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.7071f, 0.0000f, -0.7071f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, -1.0000f, 0.0000f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.6533f, 0.3827f, -0.6533f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.0000f, 0.3827f, -0.9239f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, -0.7071f, 0.0000f, -0.7071f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.0000f, 0.3827f, -0.9239f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.0000f, 0.0000f, -1.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, -0.7071f, 0.0000f, -0.7071f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.0000f, 0.3827f, -0.9239f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.6533f, 0.3827f, -0.6533f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, -0.0000f, 0.0000f, -1.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.6533f, 0.3827f, -0.6533f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.7071f, 0.0000f, -0.7071f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, -0.0000f, 0.0000f, -1.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.6533f, 0.3827f, -0.6533f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.9239f, 0.3827f, -0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, 0.7071f, 0.0000f, -0.7071f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.9239f, 0.3827f, -0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 1.0000f, 0.0000f, -0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, 0.7071f, 0.0000f, -0.7071f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 1.0000f, 0.0000f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.7071f, 0.0000f, 0.7071f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, 0.9239f, -0.3827f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.7071f, 0.0000f, 0.7071f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.6533f, -0.3827f, 0.6533f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, 0.9239f, -0.3827f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.7071f, 0.0000f, 0.7071f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.0000f, 0.0000f, 1.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, 0.6533f, -0.3827f, 0.6533f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.0000f, 0.0000f, 1.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.0000f, -0.3827f, 0.9239f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, 0.6533f, -0.3827f, 0.6533f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.0000f, 0.0000f, 1.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.7071f, 0.0000f, 0.7071f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, 0.0000f, -0.3827f, 0.9239f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.7071f, 0.0000f, 0.7071f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.6533f, -0.3827f, 0.6533f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, 0.0000f, -0.3827f, 0.9239f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.7071f, 0.0000f, 0.7071f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -1.0000f, 0.0000f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, -0.6533f, -0.3827f, 0.6533f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -1.0000f, 0.0000f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.9239f, -0.3827f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, -0.6533f, -0.3827f, 0.6533f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -1.0000f, 0.0000f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.7071f, 0.0000f, -0.7071f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, -0.9239f, -0.3827f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.7071f, 0.0000f, -0.7071f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.6533f, -0.3827f, -0.6533f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, -0.9239f, -0.3827f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.7071f, 0.0000f, -0.7071f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.0000f, 0.0000f, -1.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, -0.6533f, -0.3827f, -0.6533f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.0000f, 0.0000f, -1.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.0000f, -0.3827f, -0.9239f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, -0.6533f, -0.3827f, -0.6533f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.0000f, 0.0000f, -1.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.7071f, 0.0000f, -0.7071f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, -0.0000f, -0.3827f, -0.9239f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.7071f, 0.0000f, -0.7071f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.6533f, -0.3827f, -0.6533f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, -0.0000f, -0.3827f, -0.9239f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.7071f, 0.0000f, -0.7071f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 1.0000f, 0.0000f, -0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, 0.6533f, -0.3827f, -0.6533f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 1.0000f, 0.0000f, -0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.9239f, -0.3827f, -0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, 0.6533f, -0.3827f, -0.6533f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.9239f, -0.3827f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.6533f, -0.3827f, 0.6533f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, 0.7071f, -0.7071f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.6533f, -0.3827f, 0.6533f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.5000f, -0.7071f, 0.5000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, 0.7071f, -0.7071f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.6533f, -0.3827f, 0.6533f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.0000f, -0.3827f, 0.9239f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, 0.5000f, -0.7071f, 0.5000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.0000f, -0.3827f, 0.9239f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.0000f, -0.7071f, 0.7071f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, 0.5000f, -0.7071f, 0.5000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.0000f, -0.3827f, 0.9239f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.6533f, -0.3827f, 0.6533f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, 0.0000f, -0.7071f, 0.7071f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.6533f, -0.3827f, 0.6533f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.5000f, -0.7071f, 0.5000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, 0.0000f, -0.7071f, 0.7071f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.6533f, -0.3827f, 0.6533f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.9239f, -0.3827f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, -0.5000f, -0.7071f, 0.5000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.9239f, -0.3827f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.7071f, -0.7071f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, -0.5000f, -0.7071f, 0.5000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.9239f, -0.3827f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.6533f, -0.3827f, -0.6533f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, -0.7071f, -0.7071f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.6533f, -0.3827f, -0.6533f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.5000f, -0.7071f, -0.5000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, -0.7071f, -0.7071f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.6533f, -0.3827f, -0.6533f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.0000f, -0.3827f, -0.9239f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, -0.5000f, -0.7071f, -0.5000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.0000f, -0.3827f, -0.9239f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.0000f, -0.7071f, -0.7071f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, -0.5000f, -0.7071f, -0.5000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.0000f, -0.3827f, -0.9239f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.6533f, -0.3827f, -0.6533f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, -0.0000f, -0.7071f, -0.7071f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.6533f, -0.3827f, -0.6533f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.5000f, -0.7071f, -0.5000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, -0.0000f, -0.7071f, -0.7071f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.6533f, -0.3827f, -0.6533f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.9239f, -0.3827f, -0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, 0.5000f, -0.7071f, -0.5000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.9239f, -0.3827f, -0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.7071f, -0.7071f, -0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, 0.5000f, -0.7071f, -0.5000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.7071f, -0.7071f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.5000f, -0.7071f, 0.5000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, 0.3827f, -0.9239f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.5000f, -0.7071f, 0.5000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.2706f, -0.9239f, 0.2706f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, 0.3827f, -0.9239f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.5000f, -0.7071f, 0.5000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.0000f, -0.7071f, 0.7071f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, 0.2706f, -0.9239f, 0.2706f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.0000f, -0.7071f, 0.7071f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.0000f, -0.9239f, 0.3827f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, 0.2706f, -0.9239f, 0.2706f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.0000f, -0.7071f, 0.7071f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.5000f, -0.7071f, 0.5000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, 0.0000f, -0.9239f, 0.3827f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.5000f, -0.7071f, 0.5000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.2706f, -0.9239f, 0.2706f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, 0.0000f, -0.9239f, 0.3827f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.5000f, -0.7071f, 0.5000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.7071f, -0.7071f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, -0.2706f, -0.9239f, 0.2706f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.7071f, -0.7071f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.3827f, -0.9239f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, -0.2706f, -0.9239f, 0.2706f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.7071f, -0.7071f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.5000f, -0.7071f, -0.5000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, -0.3827f, -0.9239f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.5000f, -0.7071f, -0.5000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.2706f, -0.9239f, -0.2706f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, -0.3827f, -0.9239f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.5000f, -0.7071f, -0.5000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.0000f, -0.7071f, -0.7071f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, -0.2706f, -0.9239f, -0.2706f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.0000f, -0.7071f, -0.7071f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.0000f, -0.9239f, -0.3827f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, -0.2706f, -0.9239f, -0.2706f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.0000f, -0.7071f, -0.7071f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.5000f, -0.7071f, -0.5000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, -0.0000f, -0.9239f, -0.3827f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.5000f, -0.7071f, -0.5000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.2706f, -0.9239f, -0.2706f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, -0.0000f, -0.9239f, -0.3827f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.5000f, -0.7071f, -0.5000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.7071f, -0.7071f, -0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, 0.2706f, -0.9239f, -0.2706f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.7071f, -0.7071f, -0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.3827f, -0.9239f, -0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, 0.2706f, -0.9239f, -0.2706f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.3827f, -0.9239f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.2706f, -0.9239f, 0.2706f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, 0.0000f, -1.0000f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.2706f, -0.9239f, 0.2706f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.0000f, -1.0000f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, 0.0000f, -1.0000f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.2706f, -0.9239f, 0.2706f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.0000f, -0.9239f, 0.3827f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, 0.0000f, -1.0000f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.0000f, -0.9239f, 0.3827f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.0000f, -1.0000f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, 0.0000f, -1.0000f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.0000f, -0.9239f, 0.3827f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.2706f, -0.9239f, 0.2706f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, 0.0000f, -1.0000f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.2706f, -0.9239f, 0.2706f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.0000f, -1.0000f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, 0.0000f, -1.0000f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.2706f, -0.9239f, 0.2706f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.3827f, -0.9239f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, -0.0000f, -1.0000f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.3827f, -0.9239f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.0000f, -1.0000f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, -0.0000f, -1.0000f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.3827f, -0.9239f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.2706f, -0.9239f, -0.2706f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, -0.0000f, -1.0000f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.2706f, -0.9239f, -0.2706f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.0000f, -1.0000f, -0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, -0.0000f, -1.0000f, 0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.2706f, -0.9239f, -0.2706f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.0000f, -0.9239f, -0.3827f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, -0.0000f, -1.0000f, -0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.0000f, -0.9239f, -0.3827f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.0000f, -1.0000f, -0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, -0.0000f, -1.0000f, -0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, -0.0000f, -0.9239f, -0.3827f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.2706f, -0.9239f, -0.2706f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, -0.0000f, -1.0000f, -0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.2706f, -0.9239f, -0.2706f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.0000f, -1.0000f, -0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, -0.0000f, -1.0000f, -0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.2706f, -0.9239f, -0.2706f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.3827f, -0.9239f, -0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, 0.0000f, -1.0000f, -0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.3827f, -0.9239f, -0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX, 0.0000f, -1.0000f, -0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
    {PVR_CMD_VERTEX_EOL, 0.0000f, -1.0000f, -0.0000f, 0.0f, 0.0f, 0xFF0000FF, 0.0f},
};

    sphere->model.name = malloc(sizeof(char)*10);
    sprintf(sphere->model.name,"sphere");
    sphere->model.meshesCount = 1;
    sphere->model.meshes = (pvr_mesh_t*)malloc(sizeof(pvr_mesh_t));
    sphere->model.meshes[0].vtxCount = sizeof(sphere_data)/sizeof(pvr_vertex_t);
    sphere->model.meshes[0].sizeOfVtx = sphere->model.meshes[0].vtxCount*sizeof(pvr_vertex_t);
    sphere->model.meshes[0].originalVtx = (pvr_vertex_t*)memalign(32, sphere->model.meshes[0].vtxCount*sizeof(pvr_vertex_t));
    sphere->model.meshes[0].animatedVtx = (pvr_vertex_t*)memalign(32, sphere->model.meshes[0].vtxCount*sizeof(pvr_vertex_t));
    sphere->model.meshes[0].renderVtx = (pvr_vertex_t*)memalign(32, sphere->model.meshes[0].vtxCount*sizeof(pvr_vertex_t));

    memcpy(sphere->model.meshes[0].originalVtx, sphere_data , sphere->model.meshes[0].sizeOfVtx);
    memcpy(sphere->model.meshes[0].animatedVtx, sphere_data , sphere->model.meshes[0].sizeOfVtx);
    memcpy(sphere->model.meshes[0].renderVtx, sphere_data , sphere->model.meshes[0].sizeOfVtx);
    //memcpy(penta.model.meshes[0].renderVtx, penta.model.meshes[0].originalVtx , sizeof(rawdata));
    
    initModel(&sphere->model);
    displayModelData(&sphere->model);

    printf("\n[CREATE SPHERE] <<< END }\n");
    return sphere;

}