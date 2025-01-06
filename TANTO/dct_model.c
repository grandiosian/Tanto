#include "dct_model.h"

#define DEBUG_TRANSFORM 0  // Afficher les détails de transformation
#define DEBUG_CLIPPING 0  // Afficher les détails de clipping
#define DEBUG_RENDER 0    // Afficher les détails du rendu
//#define DEBUG_VERTEX 0   // Afficher les coordonnées des vertices

#define DEBUG_UPDATE 0
#define DEBUG_ANIMVTX 1
#define DEBUG_RENDERVTX 1
#define DEBUG_VTX 0



void createModel(const char *filename)
{
    // charge le fichier dct model binaire 


}

void initTextureModel(char *nameTextureKMG, dct_model_t *model)
{
    load_dct_texture( &model->texture01 , nameTextureKMG);
}

void initModel( dct_model_t *model)
{
    printf("\n\n{ START >>> [INIT-MODEL]\n");
    //model->type = DCT_MODEL;
    model->position.x = 0.0f;
    model->position.y = 0.0f;
    model->position.z = 0.0f;
    model->rotation.x = 0.0f;
    model->rotation.y = 0.0f;
    model->rotation.z = 0.0f;
    model->scale.x    = 1.0f;
    model->scale.y    = 1.0f;
    model->scale.z    = 1.0f;
    model->DEBUG_MODEL_VTX  = false;
    model->texture01.addr = NULL;
    initMeshesHeader(model);    
    printf("[INIT-MODEL] <<< END }\n\n");
    

}

void setMeshHeader(dct_mesh_t *mesh, int SETTING)
{
    pvr_poly_cxt_col(&mesh->cxt, SETTING);
    mesh->cxt.gen.culling = PVR_CULLING_NONE; 
    mesh->cxt.depth.write = PVR_DEPTHWRITE_ENABLE;
    mesh->cxt.depth.comparison = PVR_DEPTHCMP_GEQUAL;
    //esh->cxt.gen.alpha = PVR_ALPHA_DISABLE;
    mesh->cxt.blend.src = PVR_BLEND_ONE;
    mesh->cxt.blend.dst = PVR_BLEND_ZERO;
    mesh->cxt.blend.src_enable = PVR_BLEND_DISABLE;
    mesh->cxt.blend.dst_enable = PVR_BLEND_DISABLE; 
    pvr_poly_compile(&mesh->hdr, &mesh->cxt);
    
}

void initMeshesHeader(dct_model_t *model)
{
    printf("\ninit mesh header meshes count %d \n", model->meshesCount);
    for(int i_mesh=0; i_mesh<model->meshesCount; i_mesh++)
    {
        //printf("\n loop i_mesh \n");
        pvr_poly_cxt_col(&model->meshes[i_mesh].cxt, PVR_LIST_OP_POLY);
        //model->meshes[i_mesh].cxt.gen.culling = PVR_CULLING_NONE; 
        model->meshes[i_mesh].cxt.gen.culling = PVR_CULLING_NONE; 
        model->meshes[i_mesh].cxt.depth.write = PVR_DEPTHWRITE_ENABLE;
        model->meshes[i_mesh].cxt.depth.comparison = PVR_DEPTHCMP_GEQUAL;
        //model->meshes[i_mesh].cxt.gen.alpha = PVR_ALPHA_DISABLE;
        model->meshes[i_mesh].cxt.blend.src = PVR_BLEND_ONE;
        model->meshes[i_mesh].cxt.blend.dst = PVR_BLEND_ZERO;
        model->meshes[i_mesh].cxt.blend.src_enable = PVR_BLEND_DISABLE;
        model->meshes[i_mesh].cxt.blend.dst_enable = PVR_BLEND_DISABLE; 
        pvr_poly_compile(&model->meshes[i_mesh].hdr, &model->meshes[i_mesh].cxt);
        model->meshes[i_mesh].clippingTrianglesCount = model->meshes[i_mesh].vtxCount*3;
        model->meshes[i_mesh].clippingTrianglesVtx = (pvr_vertex_t*)memalign(32, model->meshes[i_mesh].clippingTrianglesCount*sizeof(pvr_vertex_t));
        
    }

    initClipTrianglesVtx();
    
}

void initClipTrianglesVtx(dct_model_t *model)
{
    for(int i_mesh=0; i_mesh<model->meshesCount; i_mesh++)
    {
        //init clipping Triangles Vtx 
        for (int i=0; i<model->meshes[i_mesh].clippingTrianglesCount; i++)
        {
            model->meshes[i_mesh].clippingTrianglesVtx[i].flags = PVR_CMD_VERTEX_EOL;
        }
    }
    
}

void setVertexColorToAllMesh(dct_mesh_t *mesh, uint32_t col)
{
    for (int i_vtx=0; i_vtx<mesh->vtxCount; i_vtx++)
    {
        mesh->animatedVtx[i_vtx].argb = col;

    }
}   

void displayModelData(dct_model_t *model)
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
    
    printf("-- POLY MESHES HDR : ");
    //if(model->hdr.cmd == PVR_CMD_MODIFIER) { printf(" PVR_CMD_MODIFIER \n");}
    //if(model->hdr.cmd == PVR_CMD_POLYHDR) { printf(" PVR_CMD_POLYHDR \n");}
    //if(model->hdr.cmd == PVR_CMD_SPRITE) { printf(" PVR_CMD_SPRITE \n");}
    //if(model->hdr.cmd == PVR_CMD_USERCLIP) { printf(" PVR_CMD_USERCLIP \n");}
    //if(model->hdr.cmd == PVR_CMD_VERTEX) { printf(" PVR_CMD_VERTEX \n");}
    //if(model->hdr.cmd == PVR_CMD_VERTEX_EOL) { printf(" PVR_CMD_VERTEX_EOL \n");}

    printf("\n\n[DISPLAY-MODEL-DATA] <<< END }\n\n");
    //printf("-------------------------------\n");
}


void __updateModel( dct_model_t *model )
{
    //---------------------------------------//
    //----   MATRIX MODEL TRANSFORMATION ----//
    //---------------------------------------//

    if(DEBUG_UPDATE){printf("\n\n{ START >>> [UPDATE-MODEL] \n\n-- name:%s ",model->name);}
    
    // for (int i=0; i<model->meshes[0].vtxCount; i++)
    // {

    //     printf("render vtx %d  x%f y%f z%f  \n",i,model->meshes[0].renderVtx[i].x,model->meshes[0].renderVtx[i].y,model->meshes[0].renderVtx[i].z);
    // }
    mat_identity();

    // CAMERA DEFORMATION ET TRANSFORMATION

    // float znear = 0.01f;
    // float zfar = 100.0f;
    // float fovy_rad = F_PI/8.0f;  // 45 degrés
    // float cot = 1.0f / dct_tan(fovy_rad); // tan est buggé et retourne 4.0 pour tan(0.785f)
    // mat_perspective(640.0f/2.0f, 480.0f/2.0f, cot, znear, zfar);
    // mat_translate(0.0f, 0.0f, -10.0f);
    dct_camera *currCam = getCurrentCamera(); 
    mat_load(currCam->transform);

    mat_translate(model->position.x, model->position.y, model->position.z);
    model->rotation.x = fmod( model->rotation.x , (2.0f*3.14159f));
    model->rotation.y = fmod( model->rotation.y , (2.0f*3.14159f));
    model->rotation.z = fmod( model->rotation.z , (2.0f*3.14159f));
    model->rotation.x = fmax(fmin(model->rotation.x, 1.5533f), -1.5533f);
    
    mat_rotate_y(model->rotation.y);
    mat_rotate_x(model->rotation.x);
    mat_rotate_z(model->rotation.z);
    mat_scale(model->scale.x,model->scale.y,model->scale.z);


    //--------------------------------------------------------------//
    //----   VERTEX TRANSFORMATION BASED ON INTERNAL SH4 MATRIX ----//
    //--------------------------------------------------------------//

    //displayModelData(model);

    for (int i_mesh=0; i_mesh<model->meshesCount; i_mesh++)
    {
        sq_cpy( model->meshes[i_mesh].renderVtx, model->meshes[i_mesh].animatedVtx, model->meshes[i_mesh].sizeOfVtx );

        for (int i_vtx=0; i_vtx<model->meshes[i_mesh].vtxCount; i_vtx++)
        {
            
            mat_trans_single3(model->meshes[i_mesh].renderVtx[i_vtx].x, 
                              model->meshes[i_mesh].renderVtx[i_vtx].y,
                              model->meshes[i_mesh].renderVtx[i_vtx].z );
            float z = model->meshes[i_mesh].renderVtx[i_vtx].z;
            model->meshes[i_mesh].renderVtx[i_vtx].z =  fmax(0.0f, fmin(0.01f, z / 1000.0f));
        }

    }

    if(DEBUG_UPDATE){printf("\n[UPDATE-MODEL] <<< END }\n\n");}

}

bool isCounterClockwise(pvr_vertex_t *v0, pvr_vertex_t *v1, pvr_vertex_t *v2) {
    float dx1 = v1->x - v0->x;
    float dy1 = v1->y - v0->y;
    float dx2 = v2->x - v0->x;
    float dy2 = v2->y - v0->y;
    
    return (dx1 * dy2 - dy1 * dx2) > 0;
}


void calculateIntersection(pvr_vertex_t* result, pvr_vertex_t* v1, pvr_vertex_t* v2, float nearZ) {
    // t = distance proportionnelle le long de l'arête
    float t = (nearZ - v1->z) / (v2->z - v1->z);
    if (fabs(v2->z - v1->z) < 0.0001f) {
    t = 0.0f;
    }
    // Position
    result->x = v1->x + t * (v2->x - v1->x);
    result->y = v1->y + t * (v2->y - v1->y);
    result->z = nearZ;
    
    // UV
    result->u = v1->u + t * (v2->u - v1->u);
    result->v = v1->v + t * (v2->v - v1->v);
    
    // Interpolation couleurs ARGB et OARGB
    // Pour chaque composante (A,R,G,B)
    for(int i = 0; i < 4; i++) {
        uint8_t c1 = (v1->argb >> (i*8)) & 0xFF;
        uint8_t c2 = (v2->argb >> (i*8)) & 0xFF;
        uint8_t c = (uint8_t)(c1 + t * (c2 - c1));
        result->argb |= (c << (i*8));
        
        c1 = (v1->oargb >> (i*8)) & 0xFF;
        c2 = (v2->oargb >> (i*8)) & 0xFF;
        c = (uint8_t)(c1 + t * (c2 - c1));
        result->oargb |= (c << (i*8));
    }
    
    // Copie des flags du premier vertex
    result->flags = v1->flags;
}

int clipTriangle(pvr_vertex_t **triangle, pvr_vertex_t* output, float nearZ, dct_model_t *model, dct_mesh_t *mesh) {
   pvr_vertex_t *input[3] = {triangle[0], triangle[1], triangle[2]};
   int insidePoints = 0;
   int outsidePoints = 0;
   
   bool inside[3];
   bool outside[3];
   for(int i = 0; i < 3; i++) {
       if(model->DEBUG_MODEL_VTX) {
           //printf("inside i:%d z:%f \n", i, input[i]->z);
       }
       inside[i] = (input[i]->z > nearZ);
       outside[i]=(input[i]->z < nearZ);
       if(inside[i]) insidePoints++;
       else outsidePoints++;
   }


   if(model->DEBUG_MODEL_VTX)
   {

    printf("insidePoints :%d OutsidePoints :%d \n",insidePoints,outsidePoints);
    printf("check inside Bool \n");
    for(int i_insideOutside=0; i_insideOutside<3; i_insideOutside++)
    {
        printf("indice %d inside =>value:%d  outside:=>value:%d \n",i_insideOutside,inside[i_insideOutside],outside[i_insideOutside]);
    }
    printf("\n debugging outputValue ClipTriangle Start \n");
    for (int i=0; i<6; i++)
    {
        printf("output v:%d flags:%x x:%f y:%f z:%f argb:%x \n", i, output[i].flags, output[i].x, output[i].y, output[i].z, output[i].argb);
    }
   }
   
   // Cas 1: Triangle entièrement devant
   if(outsidePoints == 0) {
       //memcpy(output, input[0], sizeof(pvr_vertex_t));
       //memcpy(output+1, input[1], sizeof(pvr_vertex_t));
       //memcpy(output+2, input[2], sizeof(pvr_vertex_t));
       //return 1;
   }
   
   // Cas 2: Triangle entièrement derrière
   if(insidePoints == 3) {
       return -1;
   }

    // Cas 2: Triangle entièrement derrière
   if(insidePoints == 0) {
       return 0;
   }
   
   pvr_vertex_t intersections[2]= {0};



   
   // Cas 3: Un point inside = deux triangles
   if(insidePoints == 1) {
       int inPoint;
       int outPoints[2];
       int outPointappended = 0;
       for(int i = 0; i < 3; i++) if(inside[i]) inPoint = i;
       for(int i = 0; i < 3; i++) 
       { 
            if(outside[i]) 
            { 
                if(outPointappended<2)
                {
                    outPoints[outPointappended] = i;
                    outPointappended += 1;
                }
               
            }
        
        }


        // les cas possibles

        // CAS 01 : inpoint = 0 pointA = 0 pointB = 1 pointC = 2
        // CAS 02 : inpoint = 1 pointA = 1 pointB = 2 pointC = 0
        // CAS 03 : inpoint = 2 pointA = 2 pointB = 0 pointC = 1
        // Calcul des deux intersections depuis le point inside

        switch(inPoint)
        {
            case 0:
                calculateIntersection(&intersections[0],input[0],input[1],nearZ);
                calculateIntersection(&intersections[1],input[0],input[2],nearZ);
                output[0] = intersections[0];
                output[1] = *input[1];
                output[2] = *input[2];

                // output[0].argb = 0xFFFFFF00;
                // output[1].argb = 0xFFFFFF00;
                // output[2].argb = 0xFFFFFF00;

                output[0].flags = PVR_CMD_VERTEX;
                output[1].flags = PVR_CMD_VERTEX;
                output[2].flags = PVR_CMD_VERTEX_EOL;

                

                output[3] = intersections[0];
                output[4] = intersections[1];
                output[5] = *input[2];

                // output[3].argb = 0xFFFFFFAA;
                // output[4].argb = 0xFFFFFFAA;
                // output[5].argb = 0xFFFFFFAA;

                output[3].flags = PVR_CMD_VERTEX;
                output[4].flags = PVR_CMD_VERTEX;
                output[5].flags = PVR_CMD_VERTEX_EOL;

                if(mesh->cxt.gen.culling == 2)
                {
                    
                    output[0] = intersections[0];
                    output[1] = *input[2];
                    output[2] = *input[1];
                    
                }


                if(mesh->cxt.gen.culling == 3)
                {
                    output[3] = intersections[1];
                    output[4] = intersections[0];
                    output[5] = *input[2];
                }


                
                break;

            case 1:
                calculateIntersection(&intersections[0],input[1],input[2],nearZ);
                calculateIntersection(&intersections[1],input[1],input[0],nearZ);

                output[0] = intersections[0];
                output[1] = *input[2];
                output[2] = *input[0];

                // output[0].argb = 0xFFFFFF00;
                // output[1].argb = 0xFFFFFF00;
                // output[2].argb = 0xFFFFFF00;

                output[0].flags = PVR_CMD_VERTEX;
                output[1].flags = PVR_CMD_VERTEX;
                output[2].flags = PVR_CMD_VERTEX_EOL;

                

                output[3] = intersections[0];
                output[4] = intersections[1];
                output[5] = *input[0];

                // output[3].argb = 0xFFFFFFCC;
                // output[4].argb = 0xFFFFFFCC;
                // output[5].argb = 0xFFFFFFCC;

                output[3].flags = PVR_CMD_VERTEX;
                output[4].flags = PVR_CMD_VERTEX;
                output[5].flags = PVR_CMD_VERTEX_EOL;

                if(mesh->cxt.gen.culling == 2)
                {
                    output[0] = intersections[0];
                    output[1] = *input[0];
                    output[2] = *input[2];  

                }

                if(mesh->cxt.gen.culling == 3)
                {
                    output[3] = intersections[1];
                    output[4] = intersections[0];
                    output[5] = *input[0];
                }

                break;

            case 2:
                calculateIntersection(&intersections[0],input[2],input[0],nearZ);
                calculateIntersection(&intersections[1],input[2],input[1],nearZ);

                output[0] = intersections[0];
                output[1] = *input[0];
                output[2] = *input[1];

                // output[0].argb = 0xFFFFFF00;
                // output[1].argb = 0xFFFFFF00;
                // output[2].argb = 0xFFFFFF00;

                output[0].flags = PVR_CMD_VERTEX;
                output[1].flags = PVR_CMD_VERTEX;
                output[2].flags = PVR_CMD_VERTEX_EOL;

                

                output[3] = intersections[0];
                output[4] = intersections[1];
                output[5] = *input[1];

                // output[3].argb = 0xFFFFFFEE;
                // output[4].argb = 0xFFFFFFEE;
                // output[5].argb = 0xFFFFFFEE;

                output[3].flags = PVR_CMD_VERTEX;
                output[4].flags = PVR_CMD_VERTEX;
                output[5].flags = PVR_CMD_VERTEX_EOL;

                if(mesh->cxt.gen.culling ==2)
                {
                    
                    output[0] = intersections[0];
                    output[1] = *input[1];
                    output[2] = *input[0];
                    

                }

                if(mesh->cxt.gen.culling ==3)
                {
                    output[3] = intersections[1];
                    output[4] = intersections[0];
                    output[5] = *input[1];
                }

                break;
        }
        
                            





       
       // Calcul des deux intersections depuis le point inside
    //    calculateIntersection(&intersections[0], 
    //                        input[inPoint], 
    //                        input[(inPoint + 1) % 3], 
    //                        nearZ);
                           
    //    calculateIntersection(&intersections[1], 
    //                        input[inPoint], 
    //                        input[(inPoint + 2) % 3], 
    //                        nearZ);
                           
    //    // Premier triangle
    //    output[0] = *input[outPoints[0]];
    //    output[1] = *input[outPoints[1]];
    //    output[2] = intersections[0];
    //    output[0].argb = 0xFFFFFF00;
    //    output[1].argb = 0xFFFFFF00;
    //    output[2].argb = 0xFFFFFF00;
       
    //    // Second triangle 
       
    //    output[3] = intersections[0];
    //    output[4] = *input[outPoints[1]];
    //    output[5] = intersections[1];
    //    output[3].argb = 0xFFFFFFAA;
    //    output[4].argb = 0xFFFFFFAA;
    //    output[5].argb = 0xFFFFFFAA;

    //    output[0].flags = PVR_CMD_VERTEX;
    //    output[1].flags = PVR_CMD_VERTEX;
    //    output[3].flags = PVR_CMD_VERTEX;
    //    output[4].flags = PVR_CMD_VERTEX;
    //    output[2].flags = PVR_CMD_VERTEX_EOL;
    //    output[5].flags = PVR_CMD_VERTEX_EOL;
       
       if(model->DEBUG_MODEL_VTX)
        {
            printf("\n debugging outputValue ClipTriangle END INSIDE POINT 1 => 2 triangle a creer  \n");
            for (int i=0; i<6; i++)
            {
                printf("output v:%d flags:%x x:%f y:%f z:%f argb:%x \n", i, output[i].flags, output[i].x, output[i].y, output[i].z, output[i].argb);
            }
        }
       return 2;
   }
   
   // Cas 4: Deux points inside = un triangle
   if(insidePoints == 2) {
    int outPoint;
    for(int i = 0; i < 3; i++) 
        if(!inside[i]) outPoint = i;
    

    // 3 cas possibles outPoint 0  A 0 B 1 C 2   A 1 B 2 C 0  A 2 B 0 C 1

    switch(outPoint)
    {
        case 0:
            calculateIntersection(&intersections[0],input[0],input[1],nearZ);
            calculateIntersection(&intersections[1],input[0],input[2],nearZ);
            output[0] = intersections[0];        // Première intersection
            output[1] = intersections[1];        // Deuxième intersection
            output[2] = *input[0];        // Point de départ (outside)

            output[0].flags = PVR_CMD_VERTEX;
            output[1].flags = PVR_CMD_VERTEX;
            output[2].flags = PVR_CMD_VERTEX_EOL;

            // output[0].argb  = 0xFFFF0000;
            // output[1].argb  = 0xFF00FF00;
            // output[2].argb  = 0xFF0000FF;

            if(mesh->cxt.gen.culling == 2)
            {
                output[0] = intersections[1];        // Première intersection
                output[1] = intersections[0];        // Deuxième intersection
                output[2] = *input[0];        // Point de départ (outside)

                // output[0].argb  = 0xFFFFFFFF;
                // output[1].argb  = 0xFFFFFFFF;
                // output[2].argb  = 0xFFFFFFFF;
            }


            break;

        case 1:
            calculateIntersection(&intersections[0],input[1],input[2],nearZ);
            calculateIntersection(&intersections[1],input[1],input[0],nearZ);

            output[0] = intersections[0];        // Première intersection
            output[1] = intersections[1];        // Deuxième intersection
            output[2] = *input[1];        // Point de départ (outside)

            output[0].flags = PVR_CMD_VERTEX;
            output[1].flags = PVR_CMD_VERTEX;
            output[2].flags = PVR_CMD_VERTEX_EOL;

            // output[0].argb  = 0xFFFF0000;
            // output[1].argb  = 0xFF00FF00;
            // output[2].argb  = 0xFF0000FF;
            if(mesh->cxt.gen.culling == 2)
            {
                output[0] = intersections[1];        // Première intersection
                output[1] = intersections[0];        // Deuxième intersection
                output[2] = *input[1];        // Point de départ (outside)
                // output[0].argb  = 0xFFAAAAAA;
                // output[1].argb  = 0xFFAAAAAA;
                // output[2].argb  = 0xFFAAAAAA;
            }
            break;

        case 2:
            calculateIntersection(&intersections[0],input[2],input[0],nearZ);
            calculateIntersection(&intersections[1],input[2],input[1],nearZ);

            output[0] = intersections[0];        // Première intersection
            output[1] = intersections[1];        // Deuxième intersection
            output[2] = *input[2];        // Point de départ (outside)

            output[0].flags = PVR_CMD_VERTEX;
            output[1].flags = PVR_CMD_VERTEX;
            output[2].flags = PVR_CMD_VERTEX_EOL;

            // output[0].argb  = 0xFFFF0000;
            // output[1].argb  = 0xFF00FF00;
            // output[2].argb  = 0xFF0000FF;

            if(mesh->cxt.gen.culling == 2)
            {
                output[0] = intersections[1];        // Première intersection
                output[1] = intersections[0];        // Deuxième intersection
                output[2] = *input[2];        // Point de départ (outside)

                // output[0].argb  = 0xFF909090;
                // output[1].argb  = 0xFF909090;
                // output[2].argb  = 0xFF909090;

            }
            break;

    }

    // outPoint est le point de départ 
    // Calculer les 2 intersections à partir de ce point
    // calculateIntersection(&intersections[0],
    //                      input[outPoint],
    //                      input[(outPoint + 1) % 3],
    //                      nearZ);
                         
    // calculateIntersection(&intersections[1],
    //                      input[outPoint], 
    //                      input[(outPoint + 2) % 3],
    //                      nearZ);

    // // Former un seul triangle avec le point de départ et les intersections
   
    // output[0] = intersections[0];        // Première intersection
    // output[1] = intersections[1];        // Deuxième intersection
    // output[2] = *input[outPoint];        // Point de départ (outside)

    // // Flags
    // output[0].flags = PVR_CMD_VERTEX;
    // output[1].flags = PVR_CMD_VERTEX;
    // output[2].flags = PVR_CMD_VERTEX_EOL;

    // output[0].argb  = 0xFFFF0000;
    // output[1].argb  = 0xFF00FF00;
    // output[2].argb  = 0xFF0000FF;

    // if(!isCounterClockwise(&output[0], &output[1], &output[2])) {
    // // Swap vertices 1 et 2
    // pvr_vertex_t temp = output[1];
    // output[1] = output[2];
    // output[2] = temp;
    // }

    if(model->DEBUG_MODEL_VTX)
        {
            printf("\n debugging outputValue ClipTriangle END INSIDE POINTS 2 => 1 triangle a creer \n");
            for (int i=0; i<3; i++)
            {
                printf("output v:%d flags:%x x:%f y:%f z:%f argb:%x \n", i, output[i].flags, output[i].x, output[i].y, output[i].z, output[i].argb);
            }
        }
    
    return 1;
    }



   
   return 0;
}



float checkW(float x, float y, float z, matrix_t matrix) {
    // Calcul uniquement du w (dernière ligne de la matrice transposée)
    return matrix[0][3]*x + matrix[1][3]*y + matrix[2][3]*z + matrix[3][3];
}

void transform_vertex_software_nodiv(pvr_vertex_t *vertex, matrix_t matrix) {
    float x = vertex->x;
    float y = vertex->y;
    float z = vertex->z;
    float w = 1.0f;
    
    // Multiplication matrice-vecteur simple sans division perspective
    vertex->x = matrix[0][0]*x + matrix[0][1]*y + matrix[0][2]*z + matrix[0][3]*w;
    vertex->y = matrix[1][0]*x + matrix[1][1]*y + matrix[1][2]*z + matrix[1][3]*w;
    vertex->z = matrix[2][0]*x + matrix[2][1]*y + matrix[2][2]*z + matrix[2][3]*w;
}

void matrix_multiply(matrix_t result, matrix_t a, matrix_t b) {
    matrix_t temp;  // Pour permettre result = a ou result = b en paramètre

    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            temp[i][j] = a[i][0] * b[0][j] + 
                        a[i][1] * b[1][j] + 
                        a[i][2] * b[2][j] + 
                        a[i][3] * b[3][j];
        }
    }

    // Copie du résultat
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            result[i][j] = temp[i][j];
        }
    }
}


void updateModel(dct_model_t *model) {
    if(model->DEBUG_MODEL_VTX) { printf("\n\n---------------------------------------------\n---------------------------------------------\n"); printf("\n{ START >>> [UPDATE-MODEL] Model: %s\n", model->name);}
    
    
    dct_camera *currCam = getCurrentCamera(); 
    matrix_t matrix_view;
    matrix_t matrix_MVP;
    matrix_t matrix_persp;

    matrix_t matrix_modelTransform;

    mat_identity();
    //mat_load(currCam->transform);
    mat_translate(currCam->position.x, currCam->position.y, currCam->position.z);
    mat_translate(model->position.x, model->position.y, model->position.z);
    mat_rotate_y(model->rotation.y);
    mat_rotate_x(model->rotation.x);
    mat_rotate_z(model->rotation.z);
    mat_scale(model->scale.x, model->scale.y, model->scale.z);
    mat_store(&matrix_view);
    


    // RESET
    mat_identity();
    // PERSPECTIVE DEFORMATION
    mat_perspective(currCam->offsetCenterScreen[0], currCam->offsetCenterScreen[1], currCam->cot, currCam->znear, currCam->zfar);
    mat_store(&matrix_persp);
    // CAMERA TRANSFROM  Translate  lootAt 
    mat_translate(currCam->position.x, currCam->position.y, currCam->position.z);
    
    // MODEL TRANSFORM 
    mat_translate(model->position.x, model->position.y, model->position.z);
    mat_rotate_y(model->rotation.y);
    mat_rotate_x(model->rotation.x);
    mat_rotate_z(model->rotation.z);
    mat_scale(model->scale.x, model->scale.y, model->scale.z);
    mat_store(&matrix_MVP);
    
    
    pvr_vertex_t *v_test  =  malloc(sizeof(pvr_vertex_t));
    pvr_vertex_t *v_test1 =  malloc(sizeof(pvr_vertex_t));
    pvr_vertex_t *v_test2 =  malloc(sizeof(pvr_vertex_t));
    
    int indice_current_clipping_vtx = 0;
    initClipTrianglesVtx(model);

    for (int i_mesh = 0; i_mesh < model->meshesCount; i_mesh++) {
        sq_cpy(model->meshes[i_mesh].renderVtx, 
               model->meshes[i_mesh].animatedVtx, 
               model->meshes[i_mesh].sizeOfVtx);

        for (int i_vtx = 0; i_vtx < model->meshes[i_mesh].vtxCount; i_vtx+=3) {

            // on assume qu'il n'y a que des triangles qui sont dans les meshes pas de vertex Stripes ni de quad
           
            pvr_vertex_t *v = &model->meshes[i_mesh].renderVtx[i_vtx];
            pvr_vertex_t *v1 = &model->meshes[i_mesh].renderVtx[i_vtx+1];
            pvr_vertex_t *v2 = &model->meshes[i_mesh].renderVtx[i_vtx+2];

            memcpy(v_test,   v, sizeof(pvr_vertex_t));
            memcpy(v_test1, v1, sizeof(pvr_vertex_t));
            memcpy(v_test2, v2, sizeof(pvr_vertex_t));

            mat_load(&matrix_view);
            mat_trans_single3_nodiv(v_test->x,v_test->y,v_test->z);
            mat_trans_single3_nodiv(v_test1->x,v_test1->y,v_test1->z);
            mat_trans_single3_nodiv(v_test2->x,v_test2->y,v_test2->z);


            pvr_vertex_t *inputTriangle[3] = { v_test, v_test1, v_test2 };
            pvr_vertex_t OutputTriangles[6] = {0};

            float nearZ = 0.1f;

            if(model->DEBUG_MODEL_VTX) {
                printf("\n\n---------------------------------------------\n");
                printf("\nSTART >>> DEBUG UPDATE VTX %d MESH VTX COUNT :%d \n",i_vtx, model->meshes[i_mesh].vtxCount);
                
                printf("Vertex %d before transform: x:%f y:%f z:%f argb:%x\n", 
                       i_vtx, v->x, v->y, v->z, v->argb);
                printf("Vertex %d before transform: x:%f y:%f z:%f argb:%x\n", 
                       i_vtx+1, v1->x, v1->y, v1->z, v1->argb);
                printf("Vertex %d before transform: x:%f y:%f z:%f argb:%x\n", 
                       i_vtx+2, v2->x, v2->y, v2->z, v2->argb);
            }
            int numTriangles = clipTriangle(inputTriangle, OutputTriangles, 0.4f,model,&model->meshes[i_mesh]);
            
            
            float x,y,z,w,x1,y1,z1,w1,x2,y2,z2,w2 = {0.0f};
            
            


            //model->meshes[i_mesh].currentClippingTrianglesCount = numTriangles;



            switch (numTriangles)
            {
                case 0:
                    //model->meshes[i_mesh].currentClippingTrianglesCount = 0;
                    mat_load(&matrix_MVP);

                    x = v->x;
                    y = v->y;
                    z = v->z;
                    w = 1.0f;
                    // La transformation inclut déjà la perspective et les coordonnées écran
                    mat_trans_single4(x, y, z, w);

                    x1 = v1->x;
                    y1 = v1->y;
                    z1 = v1->z;
                    w1 = 1.0f;
                    // La transformation inclut déjà la perspective et les coordonnées écran
                    mat_trans_single4(x1, y1, z1, w1);

                    x2 = v2->x;
                    y2 = v2->y;
                    z2 = v2->z;
                    w2 = 1.0f;
                    // La transformation inclut déjà la perspective et les coordonnées écran
                    mat_trans_single4(x2, y2, z2, w2);

                    
                    v->x = x;
                    v->y = y;
                    v->z = w;  

                    v1->x = x1;
                    v1->y = y1;
                    v1->z = w1;

                    v2->x = x2;
                    v2->y = y2;
                    v2->z = w2;
                    break;
                case 1:


                    
                    //model->meshes[i_mesh].currentClippingTrianglesCount = 3;
                    mat_load(&matrix_persp);

                    // ------ TRIANGLE 01 ----- 

                    x = OutputTriangles[0].x;
                    y = OutputTriangles[0].y;
                    z = OutputTriangles[0].z;
                    w = 1.0f;
                    // La transformation inclut déjà la perspective et les coordonnées écran
                    mat_trans_single4(x, y, z,w);

                    x1 = OutputTriangles[1].x;
                    y1 = OutputTriangles[1].y;
                    z1 = OutputTriangles[1].z;
                    w1 = 1.0f;
                    // La transformation inclut déjà la perspective et les coordonnées écran
                    mat_trans_single4(x1, y1, z1,w1);

                    x2 = OutputTriangles[2].x;
                    y2 = OutputTriangles[2].y;
                    z2 = OutputTriangles[2].z;
                    w2 = 1.0f;
                    // La transformation inclut déjà la perspective et les coordonnées écran
                    mat_trans_single4(x2, y2, z2,w2);

                    model->meshes[i_mesh].clippingTrianglesVtx[indice_current_clipping_vtx].flags = PVR_CMD_VERTEX;
                    model->meshes[i_mesh].clippingTrianglesVtx[indice_current_clipping_vtx].x = x;
                    model->meshes[i_mesh].clippingTrianglesVtx[indice_current_clipping_vtx].y = y;
                    model->meshes[i_mesh].clippingTrianglesVtx[indice_current_clipping_vtx].z = w;
                    model->meshes[i_mesh].clippingTrianglesVtx[indice_current_clipping_vtx].u = OutputTriangles[0].u;
                    model->meshes[i_mesh].clippingTrianglesVtx[indice_current_clipping_vtx].v = OutputTriangles[0].v;  
                    model->meshes[i_mesh].clippingTrianglesVtx[indice_current_clipping_vtx].argb = OutputTriangles[0].argb;

                    model->meshes[i_mesh].clippingTrianglesVtx[indice_current_clipping_vtx+1].flags = PVR_CMD_VERTEX;
                    model->meshes[i_mesh].clippingTrianglesVtx[indice_current_clipping_vtx+1].x = x1;
                    model->meshes[i_mesh].clippingTrianglesVtx[indice_current_clipping_vtx+1].y = y1;
                    model->meshes[i_mesh].clippingTrianglesVtx[indice_current_clipping_vtx+1].z = w1;
                    model->meshes[i_mesh].clippingTrianglesVtx[indice_current_clipping_vtx+1].u = OutputTriangles[1].u;
                    model->meshes[i_mesh].clippingTrianglesVtx[indice_current_clipping_vtx+1].v = OutputTriangles[1].v;  
                    model->meshes[i_mesh].clippingTrianglesVtx[indice_current_clipping_vtx+1].argb = OutputTriangles[1].argb;

                    model->meshes[i_mesh].clippingTrianglesVtx[indice_current_clipping_vtx+2].flags = PVR_CMD_VERTEX_EOL;
                    model->meshes[i_mesh].clippingTrianglesVtx[indice_current_clipping_vtx+2].x = x2;
                    model->meshes[i_mesh].clippingTrianglesVtx[indice_current_clipping_vtx+2].y = y2;
                    model->meshes[i_mesh].clippingTrianglesVtx[indice_current_clipping_vtx+2].z = w2;
                    model->meshes[i_mesh].clippingTrianglesVtx[indice_current_clipping_vtx+2].u = OutputTriangles[2].u;
                    model->meshes[i_mesh].clippingTrianglesVtx[indice_current_clipping_vtx+2].v = OutputTriangles[2].v;  
                    model->meshes[i_mesh].clippingTrianglesVtx[indice_current_clipping_vtx+2].argb = OutputTriangles[2].argb;

                    

                    break;
                case 2:
                    //model->meshes[i_mesh].currentClippingTrianglesCount = 6;
                    mat_load(&matrix_persp);

                    // ------ TRIANGLE 01 ----- 

                    x = OutputTriangles[0].x;
                    y = OutputTriangles[0].y;
                    z = OutputTriangles[0].z;
                    w = 1.0f;
                    // La transformation inclut déjà la perspective et les coordonnées écran
                    mat_trans_single4(x, y, z,w);

                    x1 = OutputTriangles[1].x;
                    y1 = OutputTriangles[1].y;
                    z1 = OutputTriangles[1].z;
                    w1 = 1.0f;
                    // La transformation inclut déjà la perspective et les coordonnées écran
                    mat_trans_single4(x1, y1, z1,w1);

                    x2 = OutputTriangles[2].x;
                    y2 = OutputTriangles[2].y;
                    z2 = OutputTriangles[2].z;
                    w2 = 1.0f;
                    // La transformation inclut déjà la perspective et les coordonnées écran
                    mat_trans_single4(x2, y2, z2,w2);

                    model->meshes[i_mesh].clippingTrianglesVtx[indice_current_clipping_vtx].flags = PVR_CMD_VERTEX;
                    model->meshes[i_mesh].clippingTrianglesVtx[indice_current_clipping_vtx].x = x;
                    model->meshes[i_mesh].clippingTrianglesVtx[indice_current_clipping_vtx].y = y;
                    model->meshes[i_mesh].clippingTrianglesVtx[indice_current_clipping_vtx].z = w;
                    model->meshes[i_mesh].clippingTrianglesVtx[indice_current_clipping_vtx].u = OutputTriangles[0].u;
                    model->meshes[i_mesh].clippingTrianglesVtx[indice_current_clipping_vtx].v = OutputTriangles[0].v;  
                    model->meshes[i_mesh].clippingTrianglesVtx[indice_current_clipping_vtx].argb = OutputTriangles[0].argb;

                    model->meshes[i_mesh].clippingTrianglesVtx[indice_current_clipping_vtx+1].flags = PVR_CMD_VERTEX;
                    model->meshes[i_mesh].clippingTrianglesVtx[indice_current_clipping_vtx+1].x = x1;
                    model->meshes[i_mesh].clippingTrianglesVtx[indice_current_clipping_vtx+1].y = y1;
                    model->meshes[i_mesh].clippingTrianglesVtx[indice_current_clipping_vtx+1].z = w1;
                    model->meshes[i_mesh].clippingTrianglesVtx[indice_current_clipping_vtx+1].u = OutputTriangles[1].u;
                    model->meshes[i_mesh].clippingTrianglesVtx[indice_current_clipping_vtx+1].v = OutputTriangles[1].v;  
                    model->meshes[i_mesh].clippingTrianglesVtx[indice_current_clipping_vtx+1].argb = OutputTriangles[1].argb;

                    model->meshes[i_mesh].clippingTrianglesVtx[indice_current_clipping_vtx+2].flags = PVR_CMD_VERTEX_EOL;
                    model->meshes[i_mesh].clippingTrianglesVtx[indice_current_clipping_vtx+2].x = x2;
                    model->meshes[i_mesh].clippingTrianglesVtx[indice_current_clipping_vtx+2].y = y2;
                    model->meshes[i_mesh].clippingTrianglesVtx[indice_current_clipping_vtx+2].z = w2;
                    model->meshes[i_mesh].clippingTrianglesVtx[indice_current_clipping_vtx+2].u = OutputTriangles[2].u;
                    model->meshes[i_mesh].clippingTrianglesVtx[indice_current_clipping_vtx+2].v = OutputTriangles[2].v;  
                    model->meshes[i_mesh].clippingTrianglesVtx[indice_current_clipping_vtx+2].argb = OutputTriangles[2].argb;

                    // ------ TRIANGLE 02 ----- 

                    x = OutputTriangles[3].x;
                    y = OutputTriangles[3].y;
                    z = OutputTriangles[3].z;
                    w = 1.0f;
                    // La transformation inclut déjà la perspective et les coordonnées écran
                    mat_trans_single4(x, y, z,w);

                    x1 = OutputTriangles[4].x;
                    y1 = OutputTriangles[4].y;
                    z1 = OutputTriangles[4].z;
                    w1 = 1.0f;
                    // La transformation inclut déjà la perspective et les coordonnées écran
                    mat_trans_single4(x1, y1, z1,w1);

                    x2 = OutputTriangles[5].x;
                    y2 = OutputTriangles[5].y;
                    z2 = OutputTriangles[5].z;
                    w2 = 1.0f;
                    // La transformation inclut déjà la perspective et les coordonnées écran
                    mat_trans_single4(x2, y2, z2,w2);

                    model->meshes[i_mesh].clippingTrianglesVtx[indice_current_clipping_vtx+3].flags = PVR_CMD_VERTEX;
                    model->meshes[i_mesh].clippingTrianglesVtx[indice_current_clipping_vtx+3].x = x;
                    model->meshes[i_mesh].clippingTrianglesVtx[indice_current_clipping_vtx+3].y = y;
                    model->meshes[i_mesh].clippingTrianglesVtx[indice_current_clipping_vtx+3].z = w;
                    model->meshes[i_mesh].clippingTrianglesVtx[indice_current_clipping_vtx+3].u = OutputTriangles[3].u;
                    model->meshes[i_mesh].clippingTrianglesVtx[indice_current_clipping_vtx+3].v = OutputTriangles[3].v;  
                    model->meshes[i_mesh].clippingTrianglesVtx[indice_current_clipping_vtx+3].argb = OutputTriangles[3].argb;

                    model->meshes[i_mesh].clippingTrianglesVtx[indice_current_clipping_vtx+4].flags = PVR_CMD_VERTEX;
                    model->meshes[i_mesh].clippingTrianglesVtx[indice_current_clipping_vtx+4].x = x1;
                    model->meshes[i_mesh].clippingTrianglesVtx[indice_current_clipping_vtx+4].y = y1;
                    model->meshes[i_mesh].clippingTrianglesVtx[indice_current_clipping_vtx+4].z = w1;
                    model->meshes[i_mesh].clippingTrianglesVtx[indice_current_clipping_vtx+4].u = OutputTriangles[4].u;
                    model->meshes[i_mesh].clippingTrianglesVtx[indice_current_clipping_vtx+4].v = OutputTriangles[4].v;  
                    model->meshes[i_mesh].clippingTrianglesVtx[indice_current_clipping_vtx+4].argb = OutputTriangles[4].argb;

                    model->meshes[i_mesh].clippingTrianglesVtx[indice_current_clipping_vtx+5].flags = PVR_CMD_VERTEX_EOL;
                    model->meshes[i_mesh].clippingTrianglesVtx[indice_current_clipping_vtx+5].x = x2;
                    model->meshes[i_mesh].clippingTrianglesVtx[indice_current_clipping_vtx+5].y = y2;
                    model->meshes[i_mesh].clippingTrianglesVtx[indice_current_clipping_vtx+5].z = w2;
                    model->meshes[i_mesh].clippingTrianglesVtx[indice_current_clipping_vtx+5].u = OutputTriangles[5].u;
                    model->meshes[i_mesh].clippingTrianglesVtx[indice_current_clipping_vtx+5].v = OutputTriangles[5].v;  
                    model->meshes[i_mesh].clippingTrianglesVtx[indice_current_clipping_vtx+5].argb = OutputTriangles[5].argb;

                    
                    break;
            }

            if(model->DEBUG_MODEL_VTX) {
                
                matrix_t mdebug;
                mat_store(&mdebug);
                
                printf("\nnumTriangles :%d \n", numTriangles);
                printf("input Triangle %d \n x1:%f y1:%f z1:%f \n x2:%f y2:%f z2:%f \n x3:%f y3:%f z3:%f \n", i_vtx,
                inputTriangle[0]->x,inputTriangle[0]->y,inputTriangle[0]->z,
                inputTriangle[1]->x,inputTriangle[1]->y,inputTriangle[1]->z,
                inputTriangle[2]->x,inputTriangle[2]->y,inputTriangle[2]->z
                 );

                if(numTriangles == 1)
                {   
                    printf("Output Triangle %d \n flags:%x x1:%f y1:%f z1:%f argb:%x \n flags:%x  x2:%f y2:%f z2:%f argb:%x \n flags:%x  x3:%f y3:%f z3:%f argb:%x \n ", i_vtx,
                    OutputTriangles[0].flags,OutputTriangles[0].x,OutputTriangles[0].y,OutputTriangles[0].z,OutputTriangles[0].argb,
                    OutputTriangles[1].flags,OutputTriangles[1].x,OutputTriangles[1].y,OutputTriangles[1].z,OutputTriangles[1].argb,
                    OutputTriangles[2].flags,OutputTriangles[2].x,OutputTriangles[2].y,OutputTriangles[2].z,OutputTriangles[2].argb
                 );
                }
                if(numTriangles == 2)
                {   
                    printf("Output Triangle %d \n flags:%x x1:%f y1:%f z1:%f argb:%x \n flags:%x  x2:%f y2:%f z2:%f argb:%x \n flags:%x  x3:%f y3:%f z3:%f argb:%x \n flags:%x   x4:%f y4:%f z4:%f argb:%x \n flags:%x  x5:%f y5:%f z5:%f argb:%x \n flags:%x  x6:%f y6:%f z6:%f argb:%x \n ", i_vtx,
                    OutputTriangles[0].flags,OutputTriangles[0].x,OutputTriangles[0].y,OutputTriangles[0].z,OutputTriangles[0].argb,
                    OutputTriangles[1].flags,OutputTriangles[1].x,OutputTriangles[1].y,OutputTriangles[1].z,OutputTriangles[1].argb,
                    OutputTriangles[2].flags,OutputTriangles[2].x,OutputTriangles[2].y,OutputTriangles[2].z,OutputTriangles[2].argb,
                    OutputTriangles[3].flags,OutputTriangles[3].x,OutputTriangles[3].y,OutputTriangles[3].z,OutputTriangles[3].argb,
                    OutputTriangles[4].flags,OutputTriangles[4].x,OutputTriangles[4].y,OutputTriangles[4].z,OutputTriangles[4].argb,
                    OutputTriangles[5].flags,OutputTriangles[5].x,OutputTriangles[5].y,OutputTriangles[5].z,OutputTriangles[5].argb
                 );
                }
                
                printf("debug matrix view : \n");
                print_matrix(&matrix_view);
                
                
                printf("Vertex %d after transform: x=%f y=%f z=%f w=%f\n", 
                       i_vtx, v->x, v->y, v->z, w);
                printf("Vertex %d after transform: x=%f y=%f z=%f w=%f\n", 
                       i_vtx+1, v1->x, v1->y, v1->z, w);
                printf("Vertex %d after transform: x=%f y=%f z=%f w=%f\n", 
                       i_vtx+2, v2->x, v2->y, v2->z, w);

               
            }
            
            if(numTriangles>0)
            {
                indice_current_clipping_vtx += numTriangles*3;
            }
            model->meshes[i_mesh].currentClippingTrianglesCount = indice_current_clipping_vtx;
            if(model->DEBUG_MODEL_VTX) 
            {
                printf("indice_current_clipping_vtx : %d \n",indice_current_clipping_vtx);
                printf("DEBUG UPDATE VTX %d <<< END \n",i_vtx);
            }
        }
    }


    free(v_test);
    free(v_test1);
    free(v_test2);
    if(model->DEBUG_MODEL_VTX) printf("[UPDATE-MODEL] <<< END }\n\n");
}



void renderMesh(dct_mesh_t *mesh)
{
    pvr_prim(&mesh->hdr, sizeof(mesh->hdr));

    for (int i_vtx=0; i_vtx<mesh->vtxCount; i_vtx++)
    {
        pvr_prim(&mesh->renderVtx[i_vtx], sizeof(pvr_vertex_t));
    }

}


void old_renderModel(dct_model_t *model) {
    pvr_list_begin(PVR_LIST_OP_POLY);
    
    for (int i_mesh = 0; i_mesh < model->meshesCount; i_mesh++) {
        pvr_prim(&model->meshes[i_mesh].hdr, sizeof(pvr_poly_hdr_t));
        int quadDraw = 0;
        int vertices_in_poly = 0;
        pvr_vertex_t *last_vertices[3] = {NULL, NULL, NULL};
        
        for (int i_vtx = 0; i_vtx < model->meshes[i_mesh].vtxCount; i_vtx++) {
            pvr_vertex_t *v = &model->meshes[i_mesh].renderVtx[i_vtx];
            
            // Décaler et stocker le vertex courant
            for(int i = 0; i < 2; i++) {
                last_vertices[i] = last_vertices[i + 1];
            }
            last_vertices[2] = v;
            
            if (v->z > 0.0f) {
                vertices_in_poly++;
            }
            //printf("flags:%x v %x last_vertices[0] %x last_vertices[1] %x last_vertices[2] %x \n", v->flags ,v, last_vertices[0],last_vertices[1],last_vertices[2]   );
            //printf("Vertex %d: flags=0x%x %s\n", i_vtx, v->flags, 
            //      (v->flags & PVR_CMD_VERTEX_EOL) ? "EOL" : "");
            if ( (v->flags & PVR_CMD_VERTEX_EOL)==PVR_CMD_VERTEX_EOL && vertices_in_poly > 0) {
                //printf(" FACE QUAD %d i_vtx %d \n",vertices_in_poly, i_vtx);
                
                if (last_vertices[0] && last_vertices[1] ) {
                    //printf(" lastvertices : 0:%x   1:%x  2:%x  3:%x  \n",last_vertices[0],last_vertices[1],last_vertices[2],last_vertices[3]);
                    pvr_vertex_t *v1 = last_vertices[0];
                    pvr_vertex_t *v2 = last_vertices[1];
                    pvr_vertex_t *v3 = last_vertices[2];
                    
                    
                    
                    if(v1->z > 1.0f && v2->z > 1.0f && v3->z > 1.0f )
                    {
                        

                    }else
                    {
                            // Premier triangle
                        v1->flags = PVR_CMD_VERTEX;
                        v2->flags = PVR_CMD_VERTEX;
                        v3->flags = PVR_CMD_VERTEX_EOL;
                        pvr_prim(v1, sizeof(pvr_vertex_t));
                        pvr_prim(v2, sizeof(pvr_vertex_t));
                        pvr_prim(v3, sizeof(pvr_vertex_t));
                        //printf("v1->z:%f  v2->z:%f  v3->z:%f v1->z:%f  \n",v1->z, v2->z, v3->z, v4->z);
                        quadDraw += 1;
                    }
                }
                
                // Reset pour le prochain polygone
                vertices_in_poly = 0;
                for(int i = 0; i < 3; i++) {
                    last_vertices[i] = NULL;
                }
            }
        }

        //printf("drawQuadCount :%d \n",quadDraw);
    }
}




void _renderModel(dct_model_t *model) {
    pvr_list_begin(PVR_LIST_OP_POLY);
    
    for (int i_mesh = 0; i_mesh < model->meshesCount; i_mesh++) {
        pvr_prim(&model->meshes[i_mesh].hdr, sizeof(pvr_poly_hdr_t));
        int quadDraw = 0;
        int vertices_in_poly = 0;
        pvr_vertex_t *last_vertices[3] = {NULL, NULL, NULL};
        
        for (int i_vtx = 0; i_vtx < model->meshes[i_mesh].vtxCount; i_vtx++) {
            pvr_vertex_t *v = &model->meshes[i_mesh].renderVtx[i_vtx];
            pvr_prim(v, sizeof(pvr_vertex_t));
        }

    }
}



void renderModel(dct_model_t *model) {
    pvr_list_begin(PVR_LIST_OP_POLY);
    
    for (int i_mesh = 0; i_mesh < model->meshesCount; i_mesh++) 
    {
        
        
        if(model->texture01.addr)
        {
            pvr_prim(&model->texture01.hdr ,sizeof(pvr_poly_hdr_t));
        }else
        {
            pvr_prim(&model->meshes[i_mesh].hdr, sizeof(pvr_poly_hdr_t));
        }
        
        int quadDraw = 0;
        int vertices_in_poly = 0;
        pvr_vertex_t *last_vertices[3] = {NULL, NULL, NULL};
        
        for (int i_vtx = 0; i_vtx < model->meshes[i_mesh].vtxCount; i_vtx+=3) 
        {
            pvr_vertex_t *v = &model->meshes[i_mesh].renderVtx[i_vtx];
            pvr_vertex_t *v1 = &model->meshes[i_mesh].renderVtx[i_vtx+1];
            pvr_vertex_t *v2 = &model->meshes[i_mesh].renderVtx[i_vtx+2];

            pvr_prim(v, sizeof(pvr_vertex_t));
            pvr_prim(v1, sizeof(pvr_vertex_t));
            pvr_prim(v2, sizeof(pvr_vertex_t));
            
        }

        if(model->meshes[i_mesh].currentClippingTrianglesCount>0)
        {
            //pvr_prim(&model->meshes[i_mesh].hdr, sizeof(pvr_poly_hdr_t));
            
            for(int i = 0; i < model->meshes[i_mesh].currentClippingTrianglesCount; i++) {
                pvr_prim(&model->meshes[i_mesh].clippingTrianglesVtx[i], sizeof(pvr_vertex_t));
            }
        }

        //printf("drawQuadCount :%d \n",quadDraw);
    }
}


// Fonction utilitaire pour afficher les informations d'un vertex
void debug_print_vertex(const char* prefix, pvr_vertex_t* v) {
    printf("%s: x=%f y=%f z=%f flags=%d argb=0x%08x\n",
           prefix, v->x, v->y, v->z, v->flags, v->argb);
}

// Fonction utilitaire pour afficher une matrice 4x4
void debug_print_matrix(const char* name, float* matrix) {
    printf("%s:\n", name);
    for(int i = 0; i < 4; i++) {
        printf("[%f %f %f %f]\n", 
               matrix[i*4], matrix[i*4+1], 
               matrix[i*4+2], matrix[i*4+3]);
    }
    printf("\n");
}

void freeModel(dct_model_t *model)
{
    for (int i=0; i<model->meshesCount; i++)
    {
        free(model->meshes[i].originalVtx);
        free(model->meshes[i].animatedVtx);
        free(model->meshes[i].renderVtx);
    }

}





void createPrimitiveTriangle(dct_model_t *triangle, const char *name, 
                      float x1, float y1, float z1, float x2, float y2, float z2 ,float x3, float y3, float z3, 
                      uint32_t color[3]){
    triangle->name = strdup(name);
    //sprintf(triangle->type ,"TRIANGLE");
    triangle->meshesCount = 1;
    triangle->meshes = malloc(sizeof(dct_mesh_t));
    triangle->meshes[0].vtxCount = 3;
    triangle->meshes[0].sizeOfVtx = sizeof(pvr_vertex_t)*3;

    triangle->meshes[0].originalVtx = (pvr_vertex_t*)memalign(32, triangle->meshes[0].vtxCount*sizeof(pvr_vertex_t));
    triangle->meshes[0].animatedVtx = (pvr_vertex_t*)memalign(32, triangle->meshes[0].vtxCount*sizeof(pvr_vertex_t));
    triangle->meshes[0].renderVtx   = (pvr_vertex_t*)memalign(32, triangle->meshes[0].vtxCount*sizeof(pvr_vertex_t));
    // Premier sommet
    triangle->meshes[0].originalVtx[0].x = x1;
    triangle->meshes[0].originalVtx[0].y = y1;
    triangle->meshes[0].originalVtx[0].z = z1;
    triangle->meshes[0].originalVtx[0].u = 0.0f;
    triangle->meshes[0].originalVtx[0].v = 0.0f;
    triangle->meshes[0].originalVtx[0].flags = PVR_CMD_VERTEX;
    triangle->meshes[0].originalVtx[0].argb = color[0];
    
    // Deuxième sommet
    triangle->meshes[0].originalVtx[1].x = x2;
    triangle->meshes[0].originalVtx[1].y = y2;
    triangle->meshes[0].originalVtx[1].z = z2;
    triangle->meshes[0].originalVtx[1].u = 0.0f;
    triangle->meshes[0].originalVtx[1].v = 0.0f;
    triangle->meshes[0].originalVtx[1].flags = PVR_CMD_VERTEX;
    triangle->meshes[0].originalVtx[1].argb = color[1];
    
    // Troisième sommet
    triangle->meshes[0].originalVtx[2].x = x3;
    triangle->meshes[0].originalVtx[2].y = y3;
    triangle->meshes[0].originalVtx[2].z = z3;
    triangle->meshes[0].originalVtx[2].u = 0.0f;
    triangle->meshes[0].originalVtx[2].v = 0.0f;
    triangle->meshes[0].originalVtx[2].flags = PVR_CMD_VERTEX_EOL;
    triangle->meshes[0].originalVtx[2].argb = color[2];

    

    triangle->meshes[0].animatedVtx[0].x = x1;
    triangle->meshes[0].animatedVtx[0].y = y1;
    triangle->meshes[0].animatedVtx[0].z = z1;
    triangle->meshes[0].animatedVtx[0].u = 0.0f;
    triangle->meshes[0].animatedVtx[0].v = 0.0f;
    triangle->meshes[0].animatedVtx[0].flags = PVR_CMD_VERTEX;
    triangle->meshes[0].animatedVtx[0].argb = color[0];
    
    // Deuxième sommet
    triangle->meshes[0].animatedVtx[1].x = x2;
    triangle->meshes[0].animatedVtx[1].y = y2;
    triangle->meshes[0].animatedVtx[1].z = z2;
    triangle->meshes[0].animatedVtx[1].u = 0.0f;
    triangle->meshes[0].animatedVtx[1].v = 0.0f;
    triangle->meshes[0].animatedVtx[1].flags = PVR_CMD_VERTEX;
    triangle->meshes[0].animatedVtx[1].argb = color[1];
    
    // Troisième sommet
    triangle->meshes[0].animatedVtx[2].x = x3;
    triangle->meshes[0].animatedVtx[2].y = y3;
    triangle->meshes[0].animatedVtx[2].z = z3;
    triangle->meshes[0].animatedVtx[2].u = 0.0f;
    triangle->meshes[0].animatedVtx[2].v = 0.0f;
    triangle->meshes[0].animatedVtx[2].flags = PVR_CMD_VERTEX_EOL;
    triangle->meshes[0].animatedVtx[2].argb = color[2];

    //memcpy(triangle->meshes[0].originalVtx, triangle->meshes[0].animatedVtx, 3);

    // Premier sommet
    triangle->meshes[0].renderVtx[0].x = x1;
    triangle->meshes[0].renderVtx[0].y = y1;
    triangle->meshes[0].renderVtx[0].z = z1;
    triangle->meshes[0].renderVtx[0].u = 0.0f;
    triangle->meshes[0].renderVtx[0].v = 0.0f;
    triangle->meshes[0].renderVtx[0].flags = PVR_CMD_VERTEX;
    triangle->meshes[0].renderVtx[0].argb = color[0];
    
    // Deuxième sommet
    triangle->meshes[0].renderVtx[1].x = x2;
    triangle->meshes[0].renderVtx[1].y = y2;
    triangle->meshes[0].renderVtx[1].z = z2;
    triangle->meshes[0].renderVtx[1].u = 0.0f;
    triangle->meshes[0].renderVtx[1].v = 0.0f;
    triangle->meshes[0].renderVtx[1].flags = PVR_CMD_VERTEX;
    triangle->meshes[0].renderVtx[1].argb = color[1];
    
    // Troisième sommet
    triangle->meshes[0].renderVtx[2].x = x3;
    triangle->meshes[0].renderVtx[2].y = y3;
    triangle->meshes[0].renderVtx[2].z = z3;
    triangle->meshes[0].renderVtx[2].u = 0.0f;
    triangle->meshes[0].renderVtx[2].v = 0.0f;
    triangle->meshes[0].renderVtx[2].flags = PVR_CMD_VERTEX_EOL;
    triangle->meshes[0].renderVtx[2].argb = color[2];
    
    triangle->position = (vec3f_t){0, 0, 0};
    triangle->rotation = (vec3f_t){0, 0, 0};
    triangle->scale = (vec3f_t){1, 1, 1};
    
    pvr_poly_cxt_col(&triangle->meshes[0].cxt, PVR_LIST_OP_POLY);
    triangle->meshes[0].cxt.gen.culling = PVR_CULLING_NONE;
    pvr_poly_compile(&triangle->meshes[0].hdr, &triangle->meshes[0].cxt);
    initModel(triangle);

}



void createPrimitiveRect(dct_model_t *rect, const char *name,
                       float x, float y, float width, float height,
                       uint32_t color){
    rect->name = strdup(name);
    //sprintf(rect->type ,"RECTANGLE");
    rect->meshesCount = 1;
    
    rect->meshes = malloc(sizeof(dct_mesh_t));
    rect->meshes[0].vtxCount = 6;
    rect->meshes[0].sizeOfVtx = rect->meshes[0].vtxCount*sizeof(pvr_vertex_t);
    rect->meshes[0].originalVtx = (pvr_vertex_t*)memalign(32, rect->meshes[0].vtxCount*sizeof(pvr_vertex_t));
    rect->meshes[0].animatedVtx = (pvr_vertex_t*)memalign(32, rect->meshes[0].vtxCount*sizeof(pvr_vertex_t));
    rect->meshes[0].renderVtx   = (pvr_vertex_t*)memalign(32, rect->meshes[0].vtxCount*sizeof(pvr_vertex_t));
    

    // Coin supérieur gauche
    rect->meshes[0].originalVtx[0].x = x;
    rect->meshes[0].originalVtx[0].y = y;
    rect->meshes[0].originalVtx[0].z = 1.0f;
    rect->meshes[0].originalVtx[0].flags = PVR_CMD_VERTEX;
    rect->meshes[0].originalVtx[0].argb = color;
    
    // Coin supérieur droit
    rect->meshes[0].originalVtx[1].x = x + width;
    rect->meshes[0].originalVtx[1].y = y;
    rect->meshes[0].originalVtx[1].z = 1.0f;
    rect->meshes[0].originalVtx[1].flags = PVR_CMD_VERTEX;
    rect->meshes[0].originalVtx[1].argb = color;
    
    // Coin inférieur gauche
    rect->meshes[0].originalVtx[2].x = x;
    rect->meshes[0].originalVtx[2].y = y + height;
    rect->meshes[0].originalVtx[2].z = 1.0f;
    rect->meshes[0].originalVtx[2].flags = PVR_CMD_VERTEX_EOL;
    rect->meshes[0].originalVtx[2].argb = color;

     // Coin supérieur gauche
    rect->meshes[0].originalVtx[3].x = x;
    rect->meshes[0].originalVtx[3].y = y + height;
    rect->meshes[0].originalVtx[3].z = 1.0f;
    rect->meshes[0].originalVtx[3].flags = PVR_CMD_VERTEX;
    rect->meshes[0].originalVtx[3].argb = color;
    
    // Coin inférieur gauche
    rect->meshes[0].originalVtx[4].x = x + width;
    rect->meshes[0].originalVtx[4].y = y ;
    rect->meshes[0].originalVtx[4].z = 1.0f;
    rect->meshes[0].originalVtx[4].flags = PVR_CMD_VERTEX;
    rect->meshes[0].originalVtx[4].argb = color;
    
    // Coin inférieur droit
    rect->meshes[0].originalVtx[5].x = x + width;
    rect->meshes[0].originalVtx[5].y = y + height;
    rect->meshes[0].originalVtx[5].z = 1.0f;
    rect->meshes[0].originalVtx[5].flags = PVR_CMD_VERTEX_EOL;
    rect->meshes[0].originalVtx[5].argb = color;


    memcpy( rect->meshes[0].animatedVtx, rect->meshes[0].originalVtx, rect->meshes[0].sizeOfVtx);
    memcpy( rect->meshes[0].renderVtx, rect->meshes[0].originalVtx, rect->meshes[0].sizeOfVtx);

    rect->position = (vec3f_t){0, 0, 0};
    rect->rotation = (vec3f_t){0, 0, 0};
    rect->scale = (vec3f_t){1, 1, 1};
    
    pvr_poly_cxt_col(&rect->meshes[0].cxt, PVR_LIST_OP_POLY);
    rect->meshes[0].cxt.gen.culling = PVR_CULLING_NONE;
    pvr_poly_compile(&rect->meshes[0].hdr, &rect->meshes[0].cxt);

    initModel(rect);
}



void createPrimitiveLine(dct_model_t *line, const char *name,
                   float x1, float y1, float x2, float y2,
                   float thickness, uint32_t color){
    
    line->meshes = malloc(sizeof(dct_mesh_t));
    
    line->meshes[0].vtxCount = 6;
    line->meshes[0].sizeOfVtx = line->meshes[0].vtxCount*sizeof(pvr_vertex_t);
    line->meshes[0].originalVtx = (pvr_vertex_t*)memalign(32, line->meshes[0].vtxCount*sizeof(pvr_vertex_t));
    line->meshes[0].animatedVtx = (pvr_vertex_t*)memalign(32, line->meshes[0].vtxCount*sizeof(pvr_vertex_t));
    line->meshes[0].renderVtx   = (pvr_vertex_t*)memalign(32, line->meshes[0].vtxCount*sizeof(pvr_vertex_t));
    

    line->meshesCount = 1;
    line->name = strdup(name);
    // Calculer la direction de la ligne
    float dx = x2 - x1;
    float dy = y2 - y1;
    float length = sqrtf(dx * dx + dy * dy);
    
    // Calculer les offsets perpendiculaires pour l'épaisseur
    float nx = (-dy / length) * (thickness / 2);
    float ny = (dx / length) * (thickness / 2);
    
    // Premier triangle (vertices 0,1,2)
    line->meshes[0].originalVtx[0].x = x1 + nx;
    line->meshes[0].originalVtx[0].y = y1 + ny;
    line->meshes[0].originalVtx[0].z = 1.0f;
    line->meshes[0].originalVtx[0].flags = PVR_CMD_VERTEX;
    line->meshes[0].originalVtx[0].argb = color;
    
    line->meshes[0].originalVtx[1].x = x2 + nx;
    line->meshes[0].originalVtx[1].y = y2 + ny;
    line->meshes[0].originalVtx[1].z = 1.0f;
    line->meshes[0].originalVtx[1].flags = PVR_CMD_VERTEX;
    line->meshes[0].originalVtx[1].argb = color;
    
    line->meshes[0].originalVtx[2].x = x1 - nx;
    line->meshes[0].originalVtx[2].y = y1 - ny;
    line->meshes[0].originalVtx[2].z = 1.0f;
    line->meshes[0].originalVtx[2].flags = PVR_CMD_VERTEX_EOL;
    line->meshes[0].originalVtx[2].argb = color;

    line->meshes[0].originalVtx[3].x = x1 - nx;
    line->meshes[0].originalVtx[3].y = y1 - ny;
    line->meshes[0].originalVtx[3].z = 1.0f;
    line->meshes[0].originalVtx[3].flags = PVR_CMD_VERTEX;
    line->meshes[0].originalVtx[3].argb = color;

    line->meshes[0].originalVtx[4].x = x2 + nx;
    line->meshes[0].originalVtx[4].y = y2 + ny;
    line->meshes[0].originalVtx[4].z = 1.0f;
    line->meshes[0].originalVtx[4].flags = PVR_CMD_VERTEX;
    line->meshes[0].originalVtx[4].argb = color;
    
    // Second triangle (vertices 1,2,3)
    line->meshes[0].originalVtx[5].x = x2 - nx;
    line->meshes[0].originalVtx[5].y = y2 - ny;
    line->meshes[0].originalVtx[5].z = 1.0f;
    line->meshes[0].originalVtx[5].flags = PVR_CMD_VERTEX_EOL;
    line->meshes[0].originalVtx[5].argb = color;

    memcpy( line->meshes[0].animatedVtx, line->meshes[0].originalVtx, line->meshes[0].sizeOfVtx);
    memcpy( line->meshes[0].renderVtx, line->meshes[0].originalVtx, line->meshes[0].sizeOfVtx);

    
    line->position = (vec3f_t){0, 0, 0};
    line->rotation = (vec3f_t){0, 0, 0};
    line->scale = (vec3f_t){1, 1, 1};
    
    pvr_poly_cxt_col(&line->meshes[0].cxt, PVR_LIST_OP_POLY);
    line->meshes[0].cxt.gen.culling = PVR_CULLING_NONE;
    pvr_poly_compile(&line->meshes[0].hdr, &line->meshes[0].cxt);

    initModel(line);
    
}



void createPrimitiveCircle(dct_model_t *circle, const char *name,
                    float posX, float posY, float radius,
                    int segments, uint32_t color){
    
    circle->meshes = malloc(sizeof(dct_mesh_t));
    circle->meshesCount = 1;
    circle->name = strdup(name);
    // Allouer la mémoire pour les vertices (3 vertices par triangle)
    float centerX,centerY = 0;
    circle->meshes[0].vtxCount = 3*segments;
    circle->meshes[0].sizeOfVtx = circle->meshes[0].vtxCount*sizeof(pvr_vertex_t);
    circle->meshes[0].originalVtx = (pvr_vertex_t*)memalign(32, circle->meshes[0].vtxCount*sizeof(pvr_vertex_t));
    circle->meshes[0].animatedVtx = (pvr_vertex_t*)memalign(32, circle->meshes[0].vtxCount*sizeof(pvr_vertex_t));
    circle->meshes[0].renderVtx   = (pvr_vertex_t*)memalign(32, circle->meshes[0].vtxCount*sizeof(pvr_vertex_t));
    
    // Générer les vertices pour chaque triangle du cercle
    for(int i = 0; i < segments; i++) {
        float angle1 = (2.0f * M_PI * i) / segments;
        float angle2 = (2.0f * M_PI * (i + 1)) / segments;
        
        // Centre du cercle
        circle->meshes[0].originalVtx[i*3].x = centerX;
        circle->meshes[0].originalVtx[i*3].y = centerY;
        circle->meshes[0].originalVtx[i*3].z = 1.0f;
        circle->meshes[0].originalVtx[i*3].flags = PVR_CMD_VERTEX;
        circle->meshes[0].originalVtx[i*3].argb = color;
        
        // Point sur le cercle
        circle->meshes[0].originalVtx[i*3+1].x = centerX + radius * cosf(angle1);
        circle->meshes[0].originalVtx[i*3+1].y = centerY + radius * sinf(angle1);
        circle->meshes[0].originalVtx[i*3+1].z = 1.0f;
        circle->meshes[0].originalVtx[i*3+1].flags = PVR_CMD_VERTEX;
        circle->meshes[0].originalVtx[i*3+1].argb = color;
        
        // Point suivant sur le cercle
        circle->meshes[0].originalVtx[i*3+2].x = centerX + radius * cosf(angle2);
        circle->meshes[0].originalVtx[i*3+2].y = centerY + radius * sinf(angle2);
        circle->meshes[0].originalVtx[i*3+2].z = 1.0f;
        circle->meshes[0].originalVtx[i*3+2].flags = PVR_CMD_VERTEX_EOL;
        circle->meshes[0].originalVtx[i*3+2].argb = color;

    }

    memcpy( circle->meshes[0].animatedVtx, circle->meshes[0].originalVtx, circle->meshes[0].sizeOfVtx);
    memcpy( circle->meshes[0].renderVtx, circle->meshes[0].originalVtx, circle->meshes[0].sizeOfVtx);

    
    circle->position = (vec3f_t){posX, posY, 0};
    circle->rotation = (vec3f_t){0, 0, 0};
    circle->scale = (vec3f_t){1, 1, 1};
    
    pvr_poly_cxt_col(&circle->meshes[0].cxt, PVR_LIST_OP_POLY);
    circle->meshes[0].cxt.gen.culling = PVR_CULLING_NONE;
    pvr_poly_compile(&circle->meshes[0].hdr, &circle->meshes[0].cxt);

    initModel(circle);
    
}



void createPrimitivePolygon(dct_model_t *polygon, const char *name, 
                     float *points, int numPoints, uint32_t color){
    
    polygon->meshes = malloc(sizeof(dct_mesh_t));
    polygon->meshesCount = 1;
    polygon->name = strdup(name);
    // Pour un polygone de N points, nous avons besoin de N-2 triangles
    int totalVertices = (numPoints - 2) * 3;
    polygon->meshes[0].vtxCount = totalVertices;
    polygon->meshes[0].sizeOfVtx = polygon->meshes[0].vtxCount*sizeof(pvr_vertex_t);
    polygon->meshes[0].originalVtx = (pvr_vertex_t*)memalign(32, polygon->meshes[0].vtxCount*sizeof(pvr_vertex_t));
    polygon->meshes[0].animatedVtx = (pvr_vertex_t*)memalign(32, polygon->meshes[0].vtxCount*sizeof(pvr_vertex_t));
    polygon->meshes[0].renderVtx   = (pvr_vertex_t*)memalign(32, polygon->meshes[0].vtxCount*sizeof(pvr_vertex_t));
    
    
    // Créer les triangles
    int vertexIndex = 0;
    for(int i = 1; i < numPoints-1; i++) {
        // Premier point de chaque triangle (point 0)
        polygon->meshes[0].originalVtx[vertexIndex].x = points[0];
        polygon->meshes[0].originalVtx[vertexIndex].y = points[1];
        polygon->meshes[0].originalVtx[vertexIndex].z = 1.0f;
        polygon->meshes[0].originalVtx[vertexIndex].u = 0.0f;
        polygon->meshes[0].originalVtx[vertexIndex].v = 0.0f;
        polygon->meshes[0].originalVtx[vertexIndex].argb = color;
        polygon->meshes[0].originalVtx[vertexIndex].flags = PVR_CMD_VERTEX;
        vertexIndex++;
        
        // Deuxième point
        polygon->meshes[0].originalVtx[vertexIndex].x = points[i*2];
        polygon->meshes[0].originalVtx[vertexIndex].y = points[i*2+1];
        polygon->meshes[0].originalVtx[vertexIndex].z = 1.0f;
        polygon->meshes[0].originalVtx[vertexIndex].u = 0.0f;
        polygon->meshes[0].originalVtx[vertexIndex].v = 0.0f;
        polygon->meshes[0].originalVtx[vertexIndex].argb = color;
        polygon->meshes[0].originalVtx[vertexIndex].flags = PVR_CMD_VERTEX;

        vertexIndex++;
        
        // Troisième point
        polygon->meshes[0].originalVtx[vertexIndex].x = points[(i+1)*2];
        polygon->meshes[0].originalVtx[vertexIndex].y = points[(i+1)*2+1];
        polygon->meshes[0].originalVtx[vertexIndex].z = 1.0f;
        polygon->meshes[0].originalVtx[vertexIndex].u = 0.0f;
        polygon->meshes[0].originalVtx[vertexIndex].v = 0.0f;
        polygon->meshes[0].originalVtx[vertexIndex].argb = color;
        polygon->meshes[0].originalVtx[vertexIndex].flags = PVR_CMD_VERTEX_EOL;

        vertexIndex++;
    }

    memcpy( polygon->meshes[0].animatedVtx, polygon->meshes[0].originalVtx, polygon->meshes[0].sizeOfVtx);
    memcpy( polygon->meshes[0].renderVtx, polygon->meshes[0].originalVtx, polygon->meshes[0].sizeOfVtx);


    
    polygon->position = (vec3f_t){0.0f, 0.0f, 0.0f};
    polygon->rotation = (vec3f_t){0.0f, 0.0f, 0.0f};
    polygon->scale = (vec3f_t){1.0f, 1.0f, 1.0f};
    
    pvr_poly_cxt_col(&polygon->meshes[0].cxt, PVR_LIST_OP_POLY);
    polygon->meshes[0].cxt.gen.culling = PVR_CULLING_NONE;
    pvr_poly_compile(&polygon->meshes[0].hdr, &polygon->meshes[0].cxt);

    initModel(polygon);
    
}





void createPrimitivePath(dct_model_t *path, const char *name, 
                  float *points, int numPoints, 
                  float thickness, uint32_t color){
    
    path->name = strdup(name);
    path->meshesCount = 1;
    path->meshes = malloc(sizeof(dct_mesh_t));
    // Pour chaque segment:
    // - 6 vertices pour le segment lui-même (2 triangles)
    // - 6 vertices pour la jonction (2 triangles, sauf dernier point)
    int verticesPerSegment = 6;
    int verticesPerJunction = 6;
    path->meshes[0].vtxCount = ( (numPoints - 1) * verticesPerSegment) + ((numPoints - 2) * verticesPerJunction);
    int vertexIndex = 0;  // Déclaration de vertexIndex
    path->meshes[0].sizeOfVtx = path->meshes[0].vtxCount*sizeof(pvr_vertex_t);
    printf("Debug - Total vertices: %d, segments: %d\n", path->meshes[0].vtxCount, numPoints - 1 );

    path->meshes[0].originalVtx = (pvr_vertex_t*)memalign(32, path->meshes[0].vtxCount*sizeof(pvr_vertex_t));
    path->meshes[0].animatedVtx = (pvr_vertex_t*)memalign(32, path->meshes[0].vtxCount*sizeof(pvr_vertex_t));
    path->meshes[0].renderVtx   = (pvr_vertex_t*)memalign(32, path->meshes[0].vtxCount*sizeof(pvr_vertex_t));
    
    
    for(int i = 0; i < (numPoints - 1); i++) {  // Maintenant va jusqu'au dernier segment
        float x1 = points[i*2];
        float y1 = points[i*2+1];
        float x2 = points[(i+1)*2];
        float y2 = points[(i+1)*2+1];
        
        // Direction et normale du segment actuel
        float dx = x2 - x1;
        float dy = y2 - y1;
        float length = sqrtf(dx * dx + dy * dy);
        float nx = (-dy / length) * (thickness / 2);
        float ny = (dx / length) * (thickness / 2);

        // Premier triangle du segment
        path->meshes[0].originalVtx[vertexIndex++] = (pvr_vertex_t){
            .flags = PVR_CMD_VERTEX,
            .x = x1 + nx, .y = y1 + ny, .z = 1.0f,
            .argb = color
        };
        path->meshes[0].originalVtx[vertexIndex++] = (pvr_vertex_t){
            .flags = PVR_CMD_VERTEX,
            .x = x2 + nx, .y = y2 + ny, .z = 1.0f,
            .argb = color
        };
        path->meshes[0].originalVtx[vertexIndex++] = (pvr_vertex_t){
            .flags = PVR_CMD_VERTEX_EOL,
            .x = x1 - nx, .y = y1 - ny, .z = 1.0f,
            .argb = color
        };

        // Second triangle du segment
        path->meshes[0].originalVtx[vertexIndex++] = (pvr_vertex_t){
            .flags = PVR_CMD_VERTEX,
            .x = x2 + nx, .y = y2 + ny, .z = 1.0f,
            .argb = color
        };
        path->meshes[0].originalVtx[vertexIndex++] = (pvr_vertex_t){
            .flags = PVR_CMD_VERTEX,
            .x = x2 - nx, .y = y2 - ny, .z = 1.0f,
            .argb = color
        };
        path->meshes[0].originalVtx[vertexIndex++] = (pvr_vertex_t){
            .flags = PVR_CMD_VERTEX_EOL,
            .x = x1 - nx, .y = y1 - ny, .z = 1.0f,
            .argb = color
        };
        // Ajouter les triangles de jonction aux points intérieurs, sauf pour le dernier segment
        if (i < (numPoints - 2 )) {
            float x3 = points[(i+2)*2];
            float y3 = points[(i+2)*2+1];
            
            // Direction et normale du segment suivant
            float dx2 = x3 - x2;
            float dy2 = y3 - y2;
            float length2 = sqrtf(dx2 * dx2 + dy2 * dy2);
            float nx2 = (-dy2 / length2) * (thickness / 2);
            float ny2 = (dx2 / length2) * (thickness / 2);

            // Premier triangle de jonction (côté extérieur)
            path->meshes[0].originalVtx[vertexIndex++] = (pvr_vertex_t){
                .flags = PVR_CMD_VERTEX,
                .x = x2 + nx, .y = y2 + ny, .z = 1.0f,
                .argb = color
            };
            path->meshes[0].originalVtx[vertexIndex++] = (pvr_vertex_t){
                .flags = PVR_CMD_VERTEX,
                .x = x2 + nx2, .y = y2 + ny2, .z = 1.0f,
                .argb = color
            };
            path->meshes[0].originalVtx[vertexIndex++] = (pvr_vertex_t){
                .flags = PVR_CMD_VERTEX_EOL,
                .x = x2, .y = y2, .z = 1.0f,
                .argb = color
            };

            // Second triangle de jonction (côté intérieur)
            path->meshes[0].originalVtx[vertexIndex++] = (pvr_vertex_t){
                .flags = PVR_CMD_VERTEX,
                .x = x2 - nx, .y = y2 - ny, .z = 1.0f,
                .argb = color
            };
            path->meshes[0].originalVtx[vertexIndex++] = (pvr_vertex_t){
                .flags = PVR_CMD_VERTEX,
                .x = x2 - nx2, .y = y2 - ny2, .z = 1.0f,
                .argb = color
            };
            path->meshes[0].originalVtx[vertexIndex++] = (pvr_vertex_t){
                .flags = PVR_CMD_VERTEX_EOL,
                .x = x2, .y = y2, .z = 1.0f,
                .argb = color
            };

        }
    }

    memcpy( path->meshes[0].animatedVtx, path->meshes[0].originalVtx, path->meshes[0].sizeOfVtx);
    memcpy( path->meshes[0].renderVtx, path->meshes[0].originalVtx, path->meshes[0].sizeOfVtx);

    
    path->position = (vec3f_t){0.0f, 0.0f, 0.0f};
    path->rotation = (vec3f_t){0.0f, 0.0f, 0.0f};
    path->scale = (vec3f_t){1.0f, 1.0f, 1.0f};
    
    pvr_poly_cxt_col(&path->meshes[0].cxt, PVR_LIST_OP_POLY);
    path->meshes[0].cxt.gen.culling = PVR_CULLING_NONE;
    pvr_poly_compile(&path->meshes[0].hdr, &path->meshes[0].cxt);

    initModel(path);

}





void createCube(dct_model_t *cube, const char *name, 
                  float size, uint32_t color[6]){

    printf("\n{ START >>> [CREATE CUBE] \n");


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
    //
    //             DRAW CW => hdr culling CCW
    //                  FACE AVANT QUAD 
    //                      vertex Top 
    //                     1---------2 Y-
    //                     |         | 
    //                     |         |
    //                     |         |
    //                     4 ------- 3
    //              X-   vgauche    vdroite X+
    //     
  const pvr_vertex_t rawdata[] = {
   // Face gauche (-X) - triangles {1,2,0} et {1,3,2}
   {PVR_CMD_VERTEX,    -1*size,   1*size,   1*size,  0.0f, 0.0f, color[0], 0.0f},  // 1
   {PVR_CMD_VERTEX,    -1*size,  -1*size,  -1*size,  1.0f, 0.0f, color[0], 0.0f},  // 2
   {PVR_CMD_VERTEX_EOL,-1*size,  -1*size,   1*size,  0.0f, 1.0f, color[0], 0.0f},  // 0

   {PVR_CMD_VERTEX,    -1*size,   1*size,   1*size,  0.0f, 0.0f, color[0], 0.0f},  // 1
   {PVR_CMD_VERTEX,    -1*size,   1*size,  -1*size,  1.0f, 0.0f, color[0], 0.0f},  // 3
   {PVR_CMD_VERTEX_EOL,-1*size,  -1*size,  -1*size,  1.0f, 1.0f, color[0], 0.0f},  // 2

   // Face arrière (-Z) - triangles {3,6,2} et {3,7,6}
   {PVR_CMD_VERTEX,    -1*size,   1*size,  -1*size,  0.0f, 0.0f, color[0], 0.0f},  // 3
   {PVR_CMD_VERTEX,     1*size,  -1*size,  -1*size,  1.0f, 0.0f, color[0], 0.0f},  // 6
   {PVR_CMD_VERTEX_EOL,-1*size,  -1*size,  -1*size,  0.0f, 1.0f, color[0], 0.0f},  // 2

   {PVR_CMD_VERTEX,    -1*size,   1*size,  -1*size,  0.0f, 0.0f, color[0], 0.0f},  // 3
   {PVR_CMD_VERTEX,     1*size,   1*size,  -1*size,  1.0f, 0.0f, color[0], 0.0f},  // 7
   {PVR_CMD_VERTEX_EOL, 1*size,  -1*size,  -1*size,  1.0f, 1.0f, color[0], 0.0f},  // 6

   // Face droite (+X) - triangles {7,4,6} et {7,5,4}
   {PVR_CMD_VERTEX,     1*size,   1*size,  -1*size,  0.0f, 0.0f, color[2], 0.0f},  // 7
   {PVR_CMD_VERTEX,     1*size,  -1*size,   1*size,  1.0f, 0.0f, color[2], 0.0f},  // 4
   {PVR_CMD_VERTEX_EOL, 1*size,  -1*size,  -1*size,  0.0f, 1.0f, color[2], 0.0f},  // 6

   {PVR_CMD_VERTEX,     1*size,   1*size,  -1*size,  0.0f, 0.0f, color[0], 0.0f},  // 7
   {PVR_CMD_VERTEX,     1*size,   1*size,   1*size,  1.0f, 0.0f, color[0], 0.0f},  // 5
   {PVR_CMD_VERTEX_EOL, 1*size,  -1*size,   1*size,  1.0f, 1.0f, color[0], 0.0f},  // 4

   // Face avant (+Z) - triangles {5,0,4} et {5,1,0}
   {PVR_CMD_VERTEX,     1*size,   1*size,   1*size,  0.0f, 0.0f, color[0], 0.0f},  // 5
   {PVR_CMD_VERTEX,    -1*size,  -1*size,   1*size,  1.0f, 0.0f, color[0], 0.0f},  // 0
   {PVR_CMD_VERTEX_EOL, 1*size,  -1*size,   1*size,  0.0f, 1.0f, color[0], 0.0f},  // 4

   {PVR_CMD_VERTEX,     1*size,   1*size,   1*size,  0.0f, 0.0f, color[0], 0.0f},  // 5
   {PVR_CMD_VERTEX,    -1*size,   1*size,   1*size,  1.0f, 0.0f, color[0], 0.0f},  // 1
   {PVR_CMD_VERTEX_EOL,-1*size,  -1*size,   1*size,  1.0f, 1.0f, color[0], 0.0f},  // 0

   // Face bas (-Y) - triangles {6,0,2} et {6,4,0}
   {PVR_CMD_VERTEX,     1*size,  -1*size,  -1*size,  0.0f, 0.0f, color[4], 0.0f},  // 6
   {PVR_CMD_VERTEX,    -1*size,  -1*size,   1*size,  1.0f, 0.0f, color[4], 0.0f},  // 0
   {PVR_CMD_VERTEX_EOL,-1*size,  -1*size,  -1*size,  0.0f, 1.0f, color[4], 0.0f},  // 2

   {PVR_CMD_VERTEX,     1*size,  -1*size,  -1*size,  0.0f, 0.0f, color[4], 0.0f},  // 6
   {PVR_CMD_VERTEX,     1*size,  -1*size,   1*size,  1.0f, 0.0f, color[4], 0.0f},  // 4
   {PVR_CMD_VERTEX_EOL,-1*size,  -1*size,   1*size,  1.0f, 1.0f, color[4], 0.0f},  // 0

   // Face haut (+Y) - triangles {3,5,7} et {3,1,5}
   {PVR_CMD_VERTEX,    -1*size,   1*size,  -1*size,  0.0f, 0.0f, color[5], 0.0f},  // 3
   {PVR_CMD_VERTEX,     1*size,   1*size,   1*size,  1.0f, 0.0f, color[5], 0.0f},  // 5
   {PVR_CMD_VERTEX_EOL, 1*size,   1*size,  -1*size,  0.0f, 1.0f, color[5], 0.0f},  // 7

   {PVR_CMD_VERTEX,    -1*size,   1*size,  -1*size,  0.0f, 0.0f, color[5], 0.0f},  // 3
   {PVR_CMD_VERTEX,    -1*size,   1*size,   1*size,  1.0f, 0.0f, color[5], 0.0f},  // 1
   {PVR_CMD_VERTEX_EOL, 1*size,   1*size,   1*size,  1.0f, 1.0f, color[5], 0.0f},  // 5
        };
    



    cube->name = strdup(name);
    cube->meshesCount = 1;
    cube->meshes = (dct_mesh_t*)malloc(sizeof(dct_mesh_t));
    cube->meshes[0].vtxCount = sizeof(rawdata)/sizeof(pvr_vertex_t);
    cube->meshes[0].sizeOfVtx = cube->meshes[0].vtxCount*sizeof(pvr_vertex_t);
    cube->meshes[0].originalVtx = (pvr_vertex_t*)memalign(32, cube->meshes[0].vtxCount*sizeof(pvr_vertex_t));
    cube->meshes[0].animatedVtx = (pvr_vertex_t*)memalign(32, cube->meshes[0].vtxCount*sizeof(pvr_vertex_t));
    cube->meshes[0].renderVtx = (pvr_vertex_t*)memalign(32, cube->meshes[0].vtxCount*sizeof(pvr_vertex_t));

    memcpy(cube->meshes[0].originalVtx, rawdata , cube->meshes[0].sizeOfVtx);
    memcpy(cube->meshes[0].animatedVtx, rawdata , cube->meshes[0].sizeOfVtx);
    memcpy(cube->meshes[0].renderVtx, rawdata , cube->meshes[0].sizeOfVtx);
    //memcpy(penta.model.meshes[0].renderVtx, penta.model.meshes[0].originalVtx , sizeof(rawdata));
    
    initModel(cube);
    //displayModelData(&cube->model);

    printf("\n[CREATE CUBE] <<< END }\n");



}



void createPentahedre(dct_model_t *penta, const char *name, 
                  float size, uint32_t colors[5]){
    printf("{ START >>> [CREATE PENTAHEDRE] \n");


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

    // DRAW CW => hdr culling CCW
    //                  FACE AVANT TRIANGLE 
    //                      vertex Top 
    //                          1 Y-
    //                         / \     
    //                        /   \   
    //                       /     \ 
    //                     3 ------- 2
    //              X-   vgauche    vdroite X+
    //      

    const pvr_vertex_t rawdata[18] = {
        // FACE AV 
        {PVR_CMD_VERTEX,     0*size, -1*size,  0.0f*size, 0.0f, 0.0f, colors[0], 0.0f},
        {PVR_CMD_VERTEX,     1*size, 1*size,   1.0f*size, 0.0f, 0.0f, colors[0], 0.0f},
        {PVR_CMD_VERTEX_EOL,    -1*size,  1*size,  1.0f*size, 0.0f, 0.0f, colors[0], 0.0f},

        // FACE RIGHT
        {PVR_CMD_VERTEX,     0.0f*size, -1.0f*size, 0.0f*size, 0.0f, 0.0f, colors[1], 0.0f},
        
        {PVR_CMD_VERTEX, 1.0f*size, 1.0f*size,  -1.0f*size, 0.0f, 0.0f, colors[1], 0.0f},
        {PVR_CMD_VERTEX_EOL,     1.0f*size, 1.0f*size,  1.0f*size, 0.0f, 0.0f, colors[1], 0.0f},
        // FACE BACK
        {PVR_CMD_VERTEX,     0.0f*size,-1.0f*size, 0.0f*size, 0.0f, 0.0f, colors[2], 0.0f},
        
        {PVR_CMD_VERTEX, -1.0f*size, 1.0f*size, -1.0f*size, 0.0f, 0.0f, colors[2], 0.0f},
        {PVR_CMD_VERTEX_EOL,     1.0f*size, 1.0f*size, -1.0f*size, 0.0f, 0.0f, colors[2], 0.0f},
        // FACE LEFT
        {PVR_CMD_VERTEX,     0.0f*size,-1.0f*size,  0.0f*size, 0.0f, 0.0f, colors[3], 0.0f},
        
        {PVR_CMD_VERTEX, -1.0f*size, 1.0f*size, 1.0f*size, 0.0f, 0.0f, colors[3], 0.0f},
        {PVR_CMD_VERTEX_EOL,    -1.0f*size, 1.0f*size,  -1.0f*size, 0.0f, 0.0f, colors[3], 0.0f},
        // FACE BOTTOM 2 FACES 1 SQUARE
        {PVR_CMD_VERTEX,      -1.0f*size, 1.0f*size,  1.0f*size, 0.0f, 0.0f, colors[4], 0.0f},
        
        {PVR_CMD_VERTEX,   1.0f*size, 1.0f*size, -1.0f*size, 0.0f, 0.0f, colors[4], 0.0f},
        {PVR_CMD_VERTEX_EOL,      -1.0f*size, 1.0f*size, -1.0f*size, 0.0f, 0.0f, colors[4], 0.0f},
        {PVR_CMD_VERTEX,       1.0f*size, 1.0f*size, 1.0f*size, 0.0f, 0.0f, colors[4], 0.0f},
        
        {PVR_CMD_VERTEX,   1.0f*size, 1.0f*size, -1.0f*size, 0.0f, 0.0f, colors[4], 0.0f},
        {PVR_CMD_VERTEX_EOL,      -1.0f*size, 1.0f*size, 1.0f*size, 0.0f, 0.0f, colors[4], 0.0f}
    };
    



    penta->name = strdup(name);
    // penta->name = (char)malloc(sizeof(name));
    // penta->name = strcpy(name,sizeof(name));
    // penta->name[sizeof(name)-1] = '\0';
    
    penta->meshesCount = 1;
    penta->meshes = (dct_mesh_t*)malloc(sizeof(dct_mesh_t));
    penta->meshes[0].indice = 0;
    char meshName[20];
    sprintf(meshName, "mesh-%d %s", penta->meshes[0].indice, name);
    //penta->meshes[0].name = (char)malloc(sizeof(meshName));
    penta->meshes[0].name = strdup(meshName);
    penta->meshes[0].vtxCount = 18;
    penta->meshes[0].sizeOfVtx = penta->meshes[0].vtxCount*sizeof(pvr_vertex_t);
    penta->meshes[0].originalVtx = (pvr_vertex_t*)memalign(32, penta->meshes[0].vtxCount*sizeof(pvr_vertex_t));
    penta->meshes[0].animatedVtx = (pvr_vertex_t*)memalign(32, penta->meshes[0].vtxCount*sizeof(pvr_vertex_t));
    penta->meshes[0].renderVtx = (pvr_vertex_t*)memalign(32, penta->meshes[0].vtxCount*sizeof(pvr_vertex_t));

    memcpy(penta->meshes[0].originalVtx, rawdata , penta->meshes[0].sizeOfVtx);
    memcpy(penta->meshes[0].animatedVtx, rawdata , penta->meshes[0].sizeOfVtx);
    memcpy(penta->meshes[0].renderVtx, rawdata , penta->meshes[0].sizeOfVtx);


    initModel(penta);
    //displayModelData(&penta->model);

    printf("\n[CREATE PENTAHEDRE] <<< END }\n");


}


void createSphere(dct_model_t *sphere, const char *name ,float size, uint32_t color)
{

    printf("\n{ START >>> [CREATE SPHERE] \n");


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


    

    sphere->name = strdup(name);
    sphere->meshesCount = 1;
    sphere->meshes = (dct_mesh_t*)malloc(sizeof(dct_mesh_t));
    sphere->meshes[0].vtxCount = sizeof(sphere_data)/sizeof(pvr_vertex_t);

    for(int i=0; i<sphere->meshes[0].vtxCount; i++ )
    {
        pvr_vertex_t v = (pvr_vertex_t)sphere_data[i];
        v.x = v.x*size;
        v.y = v.y*size;
        v.z = v.z*size;
        v.argb = color;
    }
    sphere->meshes[0].sizeOfVtx = sphere->meshes[0].vtxCount*sizeof(pvr_vertex_t);
    sphere->meshes[0].originalVtx = (pvr_vertex_t*)memalign(32, sphere->meshes[0].vtxCount*sizeof(pvr_vertex_t));
    sphere->meshes[0].animatedVtx = (pvr_vertex_t*)memalign(32, sphere->meshes[0].vtxCount*sizeof(pvr_vertex_t));
    sphere->meshes[0].renderVtx = (pvr_vertex_t*)memalign(32, sphere->meshes[0].vtxCount*sizeof(pvr_vertex_t));

    memcpy(sphere->meshes[0].originalVtx, sphere_data , sphere->meshes[0].sizeOfVtx);
    memcpy(sphere->meshes[0].animatedVtx, sphere_data , sphere->meshes[0].sizeOfVtx);
    memcpy(sphere->meshes[0].renderVtx, sphere_data , sphere->meshes[0].sizeOfVtx);
    //memcpy(penta.model.meshes[0].renderVtx, penta.model.meshes[0].originalVtx , sizeof(rawdata));
    
    initModel(sphere);
    //displayModelData(&sphere->model);

    printf("\n[CREATE SPHERE] <<< END }\n");

}


