#include <kos.h>


#define DEBUG_INIT 1
#define DEBUG_UPDATE 0
#define DEBUG_RENDER 0

#define DEBUG_VTX 1
#define DEBUG_ANIMVTX 1
#define DEBUG_RENDERVTX 1





typedef struct pvr_mesh_t 
{
    int          vtxCount;
    int          sizeOfVtx;      // sizeof(originalVtx) pour optimiser sq_cpy et ne pas avoir à le calculer à chaque frame
    pvr_vertex_t *originalVtx;   // les vertex en "bindPose"
    pvr_vertex_t *animatedVtx;   // les vertex animés par le squelette
    pvr_vertex_t *renderVtx;     // les vertex transformés par mat_trans_single3 afin d'eviter une double transformation constante car mat_trans_single incrémente les vertex en input


} pvr_mesh_t;





typedef struct pvr_model_t
{
    char        *name;

    int         meshesCount;
    pvr_mesh_t  *meshes;
    
    vec3f_t     position;
    vec3f_t     rotation;
    vec3f_t     scale;

    pvr_poly_hdr_t      hdr; 
    pvr_poly_cxt_t      cxt;



} pvr_model_t;



void createModel(const char *filename)
{
    // charge le fichier dct model binaire 


}


void initModel( pvr_model_t *model)
{
    //printf("\n++++++++++++++++++++++++++++++++++\n");
    printf("\n\n{ START >>> [INIT-MODEL]\n");

    //model->cxt.gen.culling = PVR_CULLING_CCW;  // reglage du culling (CW clockwise sens horaire des vertex dessiné par défaut) 
    model->position.x = 0.0f;
    model->position.y = 0.0f;
    model->position.z = 0.0f;
    model->rotation.x = 0.0f;
    model->rotation.y = 0.0f;
    model->rotation.z = 0.0f;
    model->scale.x    = 1.0f;
    model->scale.y    = 1.0f;
    model->scale.z    = 1.0f;
    initHeader(model);
    printf("[INIT-MODEL] <<< END }\n\n");
    //printf("\n-------------------------------\n");

}


void initHeader(pvr_model_t *model)
{
      
    pvr_poly_cxt_col(&model->cxt, PVR_LIST_OP_POLY);
    model->cxt.gen.culling = PVR_CULLING_CW; 
    model->cxt.depth.write = PVR_DEPTHWRITE_ENABLE;
    model->cxt.depth.comparison = PVR_DEPTHCMP_ALWAYS;
    //model->cxt.gen.alpha = PVR_ALPHA_DISABLE;
    model->cxt.blend.src = PVR_BLEND_ONE;
    model->cxt.blend.dst = PVR_BLEND_ZERO;
    model->cxt.blend.src_enable = PVR_BLEND_DISABLE;
    model->cxt.blend.dst_enable = PVR_BLEND_DISABLE; 
    pvr_poly_compile(&model->hdr, &model->cxt);
    
}

void setHeader(pvr_model_t *model)
{

}

void displayModelData(pvr_model_t *model)
{
    //printf("\n++++++++++++++++++++++++++++++++++\n");
    printf("\n\n{ START >>> [DISPLAY-MODEL-DATA] \n\n");
    printf("-- ADDRESS PTR : 0x%X \n",model);
    printf("-- LEN MODEL MEM SIZE : %d \n", sizeof(*model));
    printf("-- name        : %s \n",model->name);
    printf("-- meshesCount : %d \n",model->meshesCount);
    printf("-- meshes     \n");
    for (int i_mesh=0; i_mesh<model->meshesCount; i_mesh++)
    {
    printf("--- mesh %d \n",i_mesh);
    printf("--- vtxCount: %d \n",model->meshes[i_mesh].vtxCount);
    printf("--- sizeOfVtxData: %d \n",model->meshes[i_mesh].sizeOfVtx);

    if(DEBUG_VTX)
    {
        for (int i_vtx=0; i_vtx<model->meshes[i_mesh].vtxCount; i_vtx++)
        {
            printf("----- originalVtx %d %f %f %f \n", i_vtx,model->meshes[i_mesh].originalVtx[i_vtx].x,model->meshes[i_mesh].originalVtx[i_vtx].y,model->meshes[i_mesh].originalVtx[i_vtx].z );
        }
    }else
    {printf("----- DISPLAY ORIGINAL VTX DISABLE \n");}
    
    if(DEBUG_ANIMVTX)
    {
        for (int i_vtx=0; i_vtx<model->meshes[i_mesh].vtxCount; i_vtx++)
        {
            printf("----- animVtx %d %f %f %f \n", i_vtx,model->meshes[i_mesh].animatedVtx[i_vtx].x,model->meshes[i_mesh].animatedVtx[i_vtx].y,model->meshes[i_mesh].animatedVtx[i_vtx].z );
        }
    }else
    {printf("----- DISPLAY ANIMATED VTX DISABLE \n");}

    if(DEBUG_RENDERVTX)
    {
        for (int i_vtx=0; i_vtx<model->meshes[i_mesh].vtxCount; i_vtx++)
        {
            printf("----- renderVtx %d %f %f %f \n", i_vtx,model->meshes[i_mesh].renderVtx[i_vtx].x,model->meshes[i_mesh].renderVtx[i_vtx].y,model->meshes[i_mesh].renderVtx[i_vtx].z );
        }

    }else
    {printf("----- DISPLAY RENDER VTX DISABLE \n");}
    

    }

    printf("-- position : %f %f %f \n",model->position.x, model->position.y, model->position.z);
    printf("-- rotation : %f %f %f \n",model->rotation.x, model->rotation.y, model->rotation.z);
    printf("-- scale    : %f %f %f \n",model->scale.x   , model->scale.y   , model->scale.z   );
    
    printf("-- POLY HDR : ");
    if(model->hdr.cmd == PVR_CMD_MODIFIER) { printf(" PVR_CMD_MODIFIER \n");}
    if(model->hdr.cmd == PVR_CMD_POLYHDR) { printf(" PVR_CMD_POLYHDR \n");}
    if(model->hdr.cmd == PVR_CMD_SPRITE) { printf(" PVR_CMD_SPRITE \n");}
    if(model->hdr.cmd == PVR_CMD_USERCLIP) { printf(" PVR_CMD_USERCLIP \n");}
    if(model->hdr.cmd == PVR_CMD_VERTEX) { printf(" PVR_CMD_VERTEX \n");}
    if(model->hdr.cmd == PVR_CMD_VERTEX_EOL) { printf(" PVR_CMD_VERTEX_EOL \n");}

    printf("\n\n[DISPLAY-MODEL-DATA] <<< END }\n\n");
    //printf("-------------------------------\n");
}


void updateModel( pvr_model_t *model )
{
    //---------------------------------------//
    //----   MATRIX MODEL TRANSFORMATION ----//
    //---------------------------------------//

    if(DEBUG_UPDATE){printf("\n\n{ START >>> [UPDATE-MODEL] \n\n-- name:%s ",model->name);}


    mat_translate(model->position.x, model->position.y, model->position.z);
    mat_rotate_x(model->rotation.x);
    mat_rotate_y(model->rotation.y);
    mat_rotate_z(model->rotation.z);
    mat_scale(model->scale.x,model->scale.y,model->scale.z);


    //--------------------------------------------------------------//
    //----   VERTEX TRANSFORMATION BASED ON INTERNAL SH4 MATRIX ----//
    //--------------------------------------------------------------//
    //displayModelData(model);
    for (int i_mesh=0; i_mesh<model->meshesCount; i_mesh++)
    {
        sq_cpy( model->meshes[i_mesh].renderVtx, model->meshes[i_mesh].originalVtx, model->meshes[i_mesh].sizeOfVtx );

        for (int i_vtx=0; i_vtx<model->meshes[i_mesh].vtxCount; i_vtx++)
        {
            
            mat_trans_single3(model->meshes[i_mesh].renderVtx[i_vtx].x, 
                              model->meshes[i_mesh].renderVtx[i_vtx].y,
                              model->meshes[i_mesh].renderVtx[i_vtx].z );
        }

    }

    if(DEBUG_UPDATE){printf("\n[UPDATE-MODEL] <<< END }\n\n");}

}


void renderModel(pvr_model_t *model)
{
    //----------------------------------//
    //----   VERTEX TRANSFER TO PVR ----//
    //----------------------------------//
    //printf("\n{\n");
    if(DEBUG_RENDER){printf("\n\n{ START >>> [RENDER-MODEL] \n\n-- Rendering %s \n",model->name);}

    // pvr_wait_ready();
    // pvr_scene_begin();
    
    pvr_list_begin(PVR_LIST_OP_POLY);

    pvr_prim(&model->hdr, sizeof(model->hdr));

    for (int i_mesh=0; i_mesh<model->meshesCount; i_mesh++)
    {
        for (int i_vtx=0; i_vtx<model->meshes[i_mesh].vtxCount; i_vtx++)
        {
            pvr_prim(&model->meshes[i_mesh].renderVtx[i_vtx], sizeof(pvr_vertex_t));
        }
    }
    
    // pvr_list_finish();
    // pvr_scene_finish();

    if(DEBUG_RENDER){printf("\n[RENDER-MODEL] <<< END }\n\n");}
    //printf("}\n");

}