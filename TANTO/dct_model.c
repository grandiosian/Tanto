#include "dct_model.h"
#include <math.h>

#define DEBUG_TRANSFORM 0  // Afficher les détails de transformation
#define DEBUG_CLIPPING 0  // Afficher les détails de clipping
#define DEBUG_RENDER 0    // Afficher les détails du rendu
//#define DEBUG_VERTEX 0   // Afficher les coordonnées des vertices

#define DEBUG_UPDATE 0
#define DEBUG_ANIMVTX 1
#define DEBUG_RENDERVTX 1
#define DEBUG_VTX 0

#define MAX_BONE_DEPTH 10

#define VERTEX_BATCH_SIZE 32
#define MAX_CLIPPING_TRIANGLES 1024
#define VERTEX_BUFFER_SIZE 512 

static matrix_t matdbg = {0};

void manipulate_path_buffer(const char* original_path, const char* texture_name, 
                          const char* sub_directory, char* output_buffer, size_t buffer_size) {
    char directory[256] = {0};
    char new_path[256] = {0};
    
    // Copie du chemin original
    char path_copy[256];
    strncpy(path_copy, original_path, sizeof(path_copy) - 1);
    
    // Trouver le dernier '/'
    char* last_slash = strrchr(path_copy, '/');
    
    if (last_slash != NULL) {
        // Extraire le répertoire
        size_t dir_length = last_slash - path_copy;
        strncpy(directory, path_copy, dir_length);
        directory[dir_length] = '\0';
        
        // Construire le nouveau chemin avec le sous-dossier
        if (sub_directory != NULL && strlen(sub_directory) > 0) {
            if (sub_directory[0] == '/') {
                snprintf(new_path, sizeof(new_path), "%s%s", directory, sub_directory);
            } else {
                snprintf(new_path, sizeof(new_path), "%s/%s", directory, sub_directory);
            }
        } else {
            strncpy(new_path, directory, sizeof(new_path) - 1);
        }
        
        // Écrire le résultat final dans le buffer fourni
        snprintf(output_buffer, buffer_size, "%s/%s.kmg", new_path, texture_name);
    } else {
        output_buffer[0] = '\0'; // En cas d'erreur, retourner une chaîne vide
    }
}


dct_model_t* load_pvr(const char* filename) {
   FILE* f = fopen(filename, "rb");
   if(!f) return NULL;
   
   dct_model_t* model = malloc(sizeof(dct_model_t));
   
   int texturesCount = 0;
   char **texturesNames;
   char model_name[32];
   
   // Header
   uint32_t magic, version, mesh_count, bone_count, anim_count;
   fread(&magic, 4, 1, f);
   fread(&version, 4, 1, f);
   fread(model_name, 1, 32, f);
   //model->name = strdup(model_name);
   strcpy(model->name,model_name); 
   fread(&mesh_count, 4, 1, f);
   fread(&bone_count, 4, 1, f);
   fread(&anim_count, 4, 1, f);

   printf("header magic:%x modelname:%s version:%d meshcount:%d bonecount:%d animcount:%d \n", 
          magic, model_name, version, mesh_count, bone_count, anim_count);

   // Initialisation
   model->meshesCount = mesh_count;
   model->meshes = malloc(sizeof(dct_mesh_t) * mesh_count);
   model->anim_count = anim_count;
   model->animations = malloc(sizeof(dct_animation_t) * anim_count);
   
   // Meshes
    for(int i = 0; i < mesh_count; i++) {
        dct_mesh_t* mesh = &model->meshes[i];
        mesh->indice = i;
        
        char tex_name[32];
        // Lire d'abord le nom de la texture
        fread(tex_name, 1, 32, f);
        strcpy(mesh->name, tex_name);

        printf("size of texture name : %d \n",sizeof(mesh->name));
        if(tex_name[0]=='\0')
        {
            printf("le nom de texture est vide donc pas de texture \n");
        }
        if(tex_name[0]!='\0')
        {
            texturesCount++;
        }
        

        // Nombre de triangles
        fread(&mesh->triCount, 4, 1, f);
        //printf("mesh tricount %d \n", mesh->triCount);
        mesh->vtxCount = mesh->triCount * 3;
        
        // Allocation de la mémoire
        mesh->sizeOfVtx = sizeof(pvr_vertex_t) * mesh->vtxCount;
        mesh->originalVtx = (pvr_vertex_t*)memalign(32, mesh->sizeOfVtx);
        mesh->animatedVtx = (pvr_vertex_t*)memalign(32, mesh->sizeOfVtx);
        mesh->renderVtx = (pvr_vertex_t*)memalign(32, mesh->sizeOfVtx);
        mesh->vertexNormals = (vec3f_t*)malloc(sizeof(vec3f_t)*mesh->vtxCount);
        mesh->vertexWeights = (dct_vertexWeight_t*)malloc(sizeof(dct_vertexWeight_t)*mesh->vtxCount);

        mesh->type = MESH_STD_OP;
        mesh->ident = DCT_MESH;
        mesh->renderVtx_PCM = NULL;
        mesh->renderVtx_TPCM = NULL;


        // Lire chaque vertex
        for(int v = 0; v < mesh->vtxCount; v++) {
            // Structure d'un vertex
            fread(&mesh->originalVtx[v].flags, 4, 1, f);     // flags
            fread(&mesh->originalVtx[v].x, 4, 3, f);         // x,y,z
            fread(&mesh->originalVtx[v].u, 4, 2, f);         // u,v
            fread(&mesh->originalVtx[v].argb, 4, 1, f);      // argb
            fread(&mesh->originalVtx[v].oargb, 4, 1, f);     // oargb

            // Lire la normale
            //vec3f_t normal;
            fread(&mesh->vertexNormals[v], 4, 3, f);
            
            // Lire les poids
            //uint32_t weight_count;
            fread(&mesh->vertexWeights[v].weightCount, 4, 1, f);
            
            // Lire les 4 paires (indice, poids)
            for(int w = 0; w < 4; w++) {
                //int bone_idx;
                //float weight;
                fread(&mesh->vertexWeights[v].indiceBone[w], 4, 1, f);
                fread(&mesh->vertexWeights[v].weight[w], 4, 1, f);
                // Stocker les poids si nécessaire
            }
        }
       
       // Copier les vertices pour l'animation et le rendu
       memcpy(mesh->animatedVtx, mesh->originalVtx, mesh->sizeOfVtx);
       memcpy(mesh->renderVtx, mesh->originalVtx, mesh->sizeOfVtx);
   }

   // Armature
   if(bone_count > 0) {
       model->armature = malloc(sizeof(dct_armature_t));
       model->armature->bone_count = bone_count;
       model->armature->bones = malloc(sizeof(dct_bone_t) * bone_count);
       
       for(int i = 0; i < bone_count; i++) {
           dct_bone_t* bone = &model->armature->bones[i];
           model->armature->bones[i].children_count = 0;
           model->armature->bones[i].children_indices = NULL;
           fread(bone->name, 1, 32, f);
           fread(&bone->parent_index, 4, 1, f);
           fread(bone->matrix, sizeof(float), 16, f);
           memcpy(bone->originalMatrix,bone->matrix, sizeof(bone->matrix));
           memcpy(bone->animatedMatrix,bone->matrix, sizeof(bone->matrix));
           memcpy(bone->finalMatrix,bone->matrix, sizeof(bone->matrix));
           memcpy(bone->localMatrix,bone->matrix, sizeof(bone->matrix));
           memcpy(bone->initLocalMatrix,bone->matrix, sizeof(bone->matrix));
           matrix_t bind;
           fread(&bone->bindMatrix, sizeof(float), 16, f);
           matrix_t inv;
           fread(&bone->invBindMatrix, sizeof(float), 16, f);
           
           
       }
   }

   // Animations
   if(anim_count > 0) {
       for(int i = 0; i < anim_count; i++) {
           dct_animation_t* anim = &model->animations[i];
           fread(anim->name, 1, 32, f);
           fread(&anim->start_frame, 4, 1, f);
           fread(&anim->end_frame, 4, 1, f);
           fread(&anim->frame_count, 4, 1, f);
           
           anim->frames.frames = malloc(sizeof(*anim->frames.frames) * anim->frame_count);
           
           for(int frame = 0; frame < anim->frame_count; frame++) {
               fread(&anim->frames.frames[frame].time, 4, 1, f);
               fread(&anim->frames.frames[frame].bone_count, 4, 1, f);
               
               anim->frames.frames[frame].bones = malloc(sizeof(*anim->frames.frames[frame].bones) 
                                                       * anim->frames.frames[frame].bone_count);
               
               for(int bone = 0; bone < anim->frames.frames[frame].bone_count; bone++) {
                   fread(&anim->frames.frames[frame].bones[bone].name, 1, 32, f);
                   fread(&anim->frames.frames[frame].bones[bone].location, 4, 3, f);
                   fread(&anim->frames.frames[frame].bones[bone].rotation, 4, 4, f);
                   fread(&anim->frames.frames[frame].bones[bone].scale, 4, 3, f);
               }
           }
       }
   }

   fclose(f);

   // Chargement des textures
   model->texturesCount = texturesCount;
   model->textures = malloc(sizeof(dct_texture_t)*model->texturesCount);
   for(int i=0; i<texturesCount; i++) {
       char final_path1[512];
       manipulate_path_buffer(filename, model->meshes[i].name, "", final_path1, sizeof(final_path1));
       load_dct_texture(&model->textures[i],&model->meshes[i].cxtMesh , &model->meshes[i].hdr  , final_path1); 
       model->meshes[i].currentCxtSelected = i;
       model->meshes[i].type = MESH_STD_OP_TXR;

   }

    
    model->modifier.triCount = 0;
    model->modifier.modifierTrianglesVolFinal = NULL;
    model->modifier.modifierTrianglesVolOriginal = NULL;
   


   initModel(model);
   return model;
}

void calculateVertexLighting(dct_mesh_t *mesh, vec3f_t lightDir) {
    // Normaliser le vecteur de lumière
    float lightLength = sqrtf(
        lightDir.x * lightDir.x + 
        lightDir.y * lightDir.y + 
        lightDir.z * lightDir.z
    );
    lightDir.x /= lightLength;
    lightDir.y /= lightLength;
    lightDir.z /= lightLength;

    // printf("\nDEBUG Shading:\n");
    // printf("Light direction normalized: x:%f y:%f z:%f\n", lightDir.x, lightDir.y, lightDir.z);
    // printf("Contrast: %f\n", mesh->shadingContrast);

    for(int i = 0; i < mesh->vtxCount; i++) {
        // Les normales sont déjà normalisées dans notre cas (cube)
        float dot = (mesh->vertexNormals[i].x * lightDir.x +
                    mesh->vertexNormals[i].y * lightDir.y +
                    mesh->vertexNormals[i].z * lightDir.z);
        
        // printf("\nVertex %d:\n", i);
        // printf("Normal: x:%f y:%f z:%f\n", 
        //       mesh->vertexNormals[i].x,
        //       mesh->vertexNormals[i].y,
        //       mesh->vertexNormals[i].z);
        // printf("Raw dot: %f\n", dot);

        // Remap dot de [-1,1] à [0,1]
        dot = (dot + 1.0f) * 0.5f;
        //printf("Remapped dot [0,1]: %f\n", dot);

        // Appliquer le contraste
        float finalDot = powf(dot, mesh->shadingContrast);
        //printf("After contrast: %f\n", finalDot);

        uint32_t baseColor = mesh->renderVtx[i].argb;
        uint32_t alpha = baseColor & 0xFF000000;
        uint8_t baseR = (baseColor >> 16) & 0xFF;
        uint8_t baseG = (baseColor >> 8) & 0xFF;
        uint8_t baseB = baseColor & 0xFF;

        uint8_t shadeR = (mesh->shadingColor >> 16) & 0xFF;
        uint8_t shadeG = (mesh->shadingColor >> 8) & 0xFF;
        uint8_t shadeB = mesh->shadingColor & 0xFF;

        uint8_t r = baseR * finalDot + shadeR * (1.0f - finalDot);
        uint8_t g = baseG * finalDot + shadeG * (1.0f - finalDot);
        uint8_t b = baseB * finalDot + shadeB * (1.0f - finalDot);

        mesh->renderVtx[i].argb = alpha | (r << 16) | (g << 8) | b;
    }
}




void initTextureModel(char *nameTextureKMG, dct_model_t *model)
{
    
    //load_dct_texture( &model->textures , nameTextureKMG);
}

void recompileAllTexturesModelHeader(dct_model_t *model)
{
    for (int i=0; i<model->texturesCount; i++ )
    {
        pvr_poly_compile(&model->textures[i].hdr, &model->textures[i].cxt);
    }
}


void printAllArmature(dct_bone_t *bone)
{
    if(bone->parent_index==-1)
    {
        //first bone in armature
        printf(">>> [START PRINT ARMATURE] \n--------------------------\n");
    }
    printf("-- current Bone %s  children count %d \n",bone->name,bone->children_count);
    printf("localMatrix : \n");
    print_matrix(bone->localMatrix);
    printf("bindMatrix : \n");
    print_matrix(bone->bindMatrix);
    printf("invBindMatrix : \n");
    print_matrix(bone->invBindMatrix);
    printf("finalMatrix : \n");
    print_matrix(bone->finalMatrix);

    if( bone->children_count>0)
    {
        for(int i=0; i<bone->children_count; i++)
        {
            dct_bone_t *boneChild = bone->children[i];
            //printf("FNprint all armature before recursion target bone name %s \n",boneChild->name);
            printAllArmature(boneChild);
        }
    }
}

void setMeshTexture(dct_model_t *model, int indiceMeshTexture, const char *fileTextureName)
{
    printf(BLEU_START_FN"\n\n{ >>> START >>> [SET-TEXTURE-MODEL]\n"BLEU_INSIDE_FN);

    if(indiceMeshTexture < model->texturesCount)
    {
        load_dct_texture(&model->textures[indiceMeshTexture], &model->meshes[indiceMeshTexture].cxtMesh, &model->meshes[indiceMeshTexture].hdr ,fileTextureName); 
        model->meshes[indiceMeshTexture].currentCxtSelected = indiceMeshTexture; 
    }
    


    printf(BLEU_END_FN"\n\n  [SET-TEXTURE-MODEL] <<< END <<< }\n"ANSI_COLOR_RESET);
}


void initModel( dct_model_t *model)
{
    printf(BLEU_START_FN "\n\n{ >>> START >>> [INIT-MODEL]\n"BLEU_INSIDE_FN);
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
    model->DEBUG_RENDER_VTX = false;
    
    if(model->texturesCount==0)
    {
        model->texturesCount = model->meshesCount;
        model->textures = (dct_texture_t*)malloc(sizeof(dct_texture_t)*model->texturesCount);
        for(int i=0; i<model->texturesCount; i++)
        {
            model->textures[i].addrA = NULL;
            model->textures[i].addrB = NULL;
        }
    }


    if(model->armature != NULL)
    {
        
            // init armature childs
            //printf("model bonecount %d \n", model->armature->bone_count );
            
            for (uint32_t i=0; i<model->armature->bone_count; i++)
            {
                //printf("i %d\n",i);
                model->armature->bones[i].children_count = 0;
                model->armature->bones[i].children_indices = NULL;
                model->armature->bones[i].children = NULL;
                transposeMatrix(model->armature->bones[i].originalMatrix);
                transposeMatrix(model->armature->bones[i].animatedMatrix);
                transposeMatrix(model->armature->bones[i].finalMatrix);
                transposeMatrix(model->armature->bones[i].localMatrix);
                transposeMatrix(model->armature->bones[i].bindMatrix);
                transposeMatrix(model->armature->bones[i].invBindMatrix);
                transposeMatrix(model->armature->bones[i].initLocalMatrix);
            }

            

            for (int i=0; i<model->armature->bone_count; i++)
            {
                dct_bone_t bone= model->armature->bones[i];
                if( model->armature->bones[i].parent_index >=0)
                {
                    model->armature->bones[i].parent =  &model->armature->bones[bone.parent_index];
                    dct_bone_t boneParent = model->armature->bones[bone.parent_index];
                    model->armature->bones[bone.parent_index].children_count += 1;
                    //printf("bone parent %d childCount %d \n",i,model->armature->bones[bone.parent_index].children_count);
                }

            }

            

            for (int i=0; i<model->armature->bone_count; i++)
            {
                dct_bone_t bone= model->armature->bones[i];
                if( model->armature->bones[i].children_count>0)
                {
                    model->armature->bones[i].children_indices = malloc(sizeof(int)* model->armature->bones[i].children_count);
                    model->armature->bones[i].children = malloc(sizeof(dct_bone_t*)* model->armature->bones[i].children_count);
                    for(int j=0; j< model->armature->bones[i].children_count; j++)
                    {
                        model->armature->bones[i].children_indices[j] = -1;
                        model->armature->bones[i].children[j]=NULL;
                    }
                }
            }


            

            for (int i=0; i<model->armature->bone_count; i++)
            {
                dct_bone_t bone= model->armature->bones[i];
                if(bone.parent_index>=0)
                {
                    dct_bone_t boneParent = model->armature->bones[bone.parent_index];
                    for(int j=0; j<boneParent.children_count; j++)
                    {
                        if( boneParent.children_indices[j] == -1 )
                        {
                            model->armature->bones[bone.parent_index].children_indices[j] = i;
                            model->armature->bones[bone.parent_index].children[j]=&model->armature->bones[i];
                            break;
                        }
                    }
                    
                }
            
            }


            // for (uint32_t i=0; i<model->armature->bone_count; i++)
            // {
            //     printf("bone %d name %s children count %d\n",i, model->armature->bones[i].name ,model->armature->bones[i].children_count);
            
            // }


            //printf("\n PRINT ARMATURE BEFORE UPDATE BONES \n");
            for (int i=0; i<model->armature->bone_count; i++)
            {
                dct_bone_t bone= model->armature->bones[i];
                if(bone.parent_index<0)
                {
                    //printAllArmature(&model->armature->bones[i]);
                }
            }


            // for (int i=0; i<model->armature->bone_count; i++)
            // {
            //     dct_bone_t bone= model->armature->bones[i];
            //     if(bone.parent_index<0)
            //     {
            //         mat_identity();
            //         dct_armature_bones_update(&model->armature->bones[i]);
            //     }
            
            // }

            //printf("\n PRINT ARMATURE AFTER UPDATE BONES \n");
            for (int i=0; i<model->armature->bone_count; i++)
            {
                dct_bone_t bone= model->armature->bones[i];
                //printf("print all Armature   bone %d parentCount %d boneCount %d \n", i, model->armature->bones[i].parent_index , model->armature->bones[i].children_count);
                if(bone.parent_index<0)
                {
                    //printAllArmature(&model->armature->bones[i]);
                }
            }


    }
    

    
    initMeshesHeader(model);

    dct_camera_t *currentCam = getCurrentCamera();
    initDefaultCurrentCamera(currentCam);
    setCurrentCameraPosition(currentCam,0.0f,0.5f,-5.0f);

    updateModel(model);

    //dct_armature_update(model);

    printf(BLEU_END_FN"\n  [INIT-MODEL] <<< END <<< }\n\n"ANSI_COLOR_RESET);
    

}



void debugRenderModifier(dct_model_t *model, int switchPvrList)
{
    if(switchPvrList == 0)
    {
        pvr_list_begin(PVR_LIST_TR_POLY);
    }
    if(switchPvrList == 1)
    {
        pvr_list_begin(PVR_LIST_TR_POLY);
    }
    
    pvr_poly_cxt_t cxt;
    pvr_poly_hdr_t hdr;
    
    // Créer un nouveau contexte pour les triangles transparents
    pvr_poly_cxt_col(&cxt, PVR_LIST_TR_POLY);
    cxt.gen.shading = PVR_SHADE_FLAT;
    cxt.blend.src = PVR_BLEND_SRCALPHA;
    cxt.blend.dst = PVR_BLEND_INVSRCALPHA;
    cxt.depth.comparison = PVR_DEPTHCMP_GEQUAL;
    cxt.depth.write = PVR_DEPTHWRITE_DISABLE;
    pvr_poly_compile(&hdr, &cxt);

    pvr_list_begin(PVR_LIST_TR_POLY);
    pvr_prim(&hdr, sizeof(pvr_poly_hdr_t));  // Utiliser le nouveau header
    //pvr_prim(&cube.meshes[0].hdr, sizeof(pvr_poly_hdr_t));
    for (int tri=0; tri<model->modifier.triCount; tri++)
    {
        pvr_modifier_vol_t *m = &model->modifier.modifierTrianglesVolFinal[tri];
        
        pvr_vertex_t      vconv1 = {PVR_CMD_VERTEX, m->ax,m->ay,m->az,0.0f,0.0f   ,0x80101010,0xFF000000};
        pvr_vertex_t      vconv2 = {PVR_CMD_VERTEX,m->bx,m->by,m->bz,0.0f,0.0f    ,0x80101010,0xFF000000};
        pvr_vertex_t      vconv3 = {PVR_CMD_VERTEX_EOL,m->cx,m->cy,m->cz,0.0f,0.0f,0x80101010,0xFF000000};
        if(vconv1.z > 0 && vconv2.z > 0 && vconv2.z > 0 )
        {
            pvr_prim(&vconv1, sizeof(pvr_vertex_t));
            pvr_prim(&vconv2, sizeof(pvr_vertex_t));
            pvr_prim(&vconv3, sizeof(pvr_vertex_t));
        }
        
        
    }

    if(switchPvrList == 0)
    {
        pvr_list_finish();
    }
    if(switchPvrList == 2)
    {
        pvr_list_finish();
    }
    
}


void applyModifier(dct_mesh_modifier_vol_t *mod)
{
    pvr_mod_hdr_t mod_hdr;
    pvr_mod_hdr_t mod_hdrEnd;
    // Configuration pour un volume modifieur normal (pas un rendu)
    pvr_mod_compile(&mod_hdr, PVR_LIST_OP_MOD,
                    PVR_MODIFIER_CHEAP_SHADOW,
                    PVR_CULLING_NONE);
    
    pvr_prim(&mod_hdr, sizeof(pvr_mod_hdr_t));

    for (int tri=0; tri<mod->triCount-1; tri++)
    {
        pvr_modifier_vol_t m = mod->modifierTrianglesVolFinal[tri];
        pvr_prim(&mod->modifierTrianglesVolFinal[tri],sizeof(pvr_modifier_vol_t));

    }

    pvr_mod_compile(&mod_hdrEnd, PVR_LIST_OP_MOD,
                    PVR_MODIFIER_INCLUDE_LAST_POLY,
                    PVR_CULLING_NONE);
    pvr_prim(&mod_hdrEnd,sizeof(pvr_mod_hdr_t));
    pvr_prim(&mod->modifierTrianglesVolFinal[mod->triCount-1],sizeof(pvr_modifier_vol_t));

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
    //printf("\ninit mesh header meshes count %d \n", model->meshesCount);
    for(int i_mesh=0; i_mesh<model->meshesCount; i_mesh++)
    {
        model->meshes[i_mesh].currentCxtSelected = -1;
        
        model->meshes[i_mesh].shadingContrast = 1.0f;  // Contraste normal
        model->meshes[i_mesh].shadingColor = 0xFF000000;  // Noir par défaut pour les ombres

        //printf("\n loop i_mesh \n");
        pvr_poly_cxt_col(&model->meshes[i_mesh].cxt, PVR_LIST_OP_POLY);
        model->meshes[i_mesh].cxt.gen.alpha = PVR_ALPHA_DISABLE;
        model->meshes[i_mesh].cxt.gen.shading = PVR_SHADE_GOURAUD; 
        model->meshes[i_mesh].cxt.gen.fog_type = PVR_FOG_DISABLE;
        model->meshes[i_mesh].cxt.gen.culling = PVR_CULLING_NONE;
        model->meshes[i_mesh].cxt.gen.color_clamp = PVR_CLRCLAMP_DISABLE;
        model->meshes[i_mesh].cxt.gen.clip_mode = PVR_USERCLIP_DISABLE;
        model->meshes[i_mesh].cxt.depth.write = PVR_DEPTHWRITE_ENABLE;
        model->meshes[i_mesh].cxt.depth.comparison = PVR_DEPTHCMP_GEQUAL;
        model->meshes[i_mesh].cxt.blend.src = PVR_BLEND_ONE;
        model->meshes[i_mesh].cxt.blend.dst = PVR_BLEND_ZERO;
        model->meshes[i_mesh].cxt.blend.src_enable = PVR_BLEND_DISABLE;
        model->meshes[i_mesh].cxt.blend.dst_enable = PVR_BLEND_DISABLE;
        model->meshes[i_mesh].cxt.txr.enable = 0; 
        model->meshes[i_mesh].cxt.txr2.enable = 0; 
        
        pvr_poly_compile(&model->meshes[i_mesh].hdr, &model->meshes[i_mesh].cxt);


        model->meshes[i_mesh].cxtMesh.ident = DCT_CXT_T;
        pvr_poly_cxt_col(&model->meshes[i_mesh].cxtMesh.cxt, PVR_LIST_OP_POLY);
        model->meshes[i_mesh].cxtMesh.cxt.gen.alpha = PVR_ALPHA_DISABLE;
        model->meshes[i_mesh].cxtMesh.cxt.gen.shading = PVR_SHADE_GOURAUD; 
        model->meshes[i_mesh].cxtMesh.cxt.gen.fog_type = PVR_FOG_DISABLE;
        model->meshes[i_mesh].cxtMesh.cxt.gen.culling = PVR_CULLING_NONE;
        model->meshes[i_mesh].cxtMesh.cxt.gen.color_clamp = PVR_CLRCLAMP_DISABLE;
        model->meshes[i_mesh].cxtMesh.cxt.gen.clip_mode = PVR_USERCLIP_DISABLE;
        model->meshes[i_mesh].cxtMesh.cxt.depth.write = PVR_DEPTHWRITE_ENABLE;
        model->meshes[i_mesh].cxtMesh.cxt.depth.comparison = PVR_DEPTHCMP_GEQUAL;
        model->meshes[i_mesh].cxtMesh.cxt.blend.src = PVR_BLEND_ONE;
        model->meshes[i_mesh].cxtMesh.cxt.blend.dst = PVR_BLEND_ZERO;
        model->meshes[i_mesh].cxtMesh.cxt.blend.src_enable = PVR_BLEND_DISABLE;
        model->meshes[i_mesh].cxtMesh.cxt.blend.dst_enable = PVR_BLEND_DISABLE;
        model->meshes[i_mesh].cxtMesh.cxt.txr.enable = 0; 
        model->meshes[i_mesh].cxtMesh.cxt.txr2.enable = 0; 


        model->meshes[i_mesh].clippingTrianglesCount = model->meshes[i_mesh].vtxCount*3;
        model->meshes[i_mesh].clippingTrianglesVtx = (pvr_vertex_t*)memalign(32, model->meshes[i_mesh].clippingTrianglesCount*sizeof(pvr_vertex_t));
        //calculateNormals(&model->meshes[i_mesh]);
        model->meshes[i_mesh].currentCxtSelected = -1;
        model->meshes[i_mesh].shadingContrast = 1.0f;
        model->meshes[i_mesh].shadingColor = 0xFF000000;

        // Allocation et initialisation des normales si pas déjà fait
        if (model->meshes[i_mesh].vertexNormals == NULL) {
            model->meshes[i_mesh].vertexNormals = (vec3f_t*)malloc(model->meshes[i_mesh].vtxCount * sizeof(vec3f_t));
            // Initialiser à des normales par défaut si nécessaire
            for(int i = 0; i < model->meshes[i_mesh].vtxCount; i++) {
                model->meshes[i_mesh].vertexNormals[i] = (vec3f_t){0.0f, 1.0f, 0.0f};  // Normal vers le haut par défaut
            }
        }
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

void debug_print_model(dct_model_t* model) {
   printf("\n=== Model: %s ===\n", model->name);
   printf("Position: (%.2f, %.2f, %.2f)\n", model->position.x, model->position.y, model->position.z);
   printf("Rotation: (%.2f, %.2f, %.2f)\n", model->rotation.x, model->rotation.y, model->rotation.z);
   printf("Scale: (%.2f, %.2f, %.2f)\n", model->scale.x, model->scale.y, model->scale.z);
   
   if(model->textures != NULL)
   {
    if(model->textures[0].addrA!=NULL)
    {
        for(int i=0; i<model->texturesCount; i++)
        {
            printf("\nTexture: %d\n",i);
            printf("Width: %d Height: %d\n", model->textures[i].textureA.w, model->textures[i].textureB.h);
        }
        
    }
    if(model->textures[0].addrB!=NULL)
    {
        for(int i=0; i<model->texturesCount; i++)
        {
            printf("\nTexture: %d\n",i);
            printf("Width: %d Height: %d\n", model->textures[i].textureA.w, model->textures[i].textureB.h);
        }
        
    }
   }
   
   
   printf("\n=== Meshes (%d) ===\n", model->meshesCount);
   for(int i = 0; i < model->meshesCount; i++) {
       dct_mesh_t* mesh = &model->meshes[i];
       printf("\nMesh %d: %s\n", mesh->indice, mesh->name);
       printf("Vertices: %d (Size: %d bytes)\n", mesh->vtxCount, mesh->sizeOfVtx);
       printf("Clipping Triangles: %d/%d\n", mesh->currentClippingTrianglesCount, mesh->clippingTrianglesCount);
       
       for(int v = 0; v < 10; v++) {
           printf("\nVertex %d:\n", v);
           printf("  CMD: 0x%08X %s\n", mesh->originalVtx[v].flags, 
                  mesh->originalVtx[v].flags == 0xf0000000 ? "EOL" : "VERTEX");
           printf("  Pos: (%.3f, %.3f, %.3f)\n", 
                  mesh->originalVtx[v].x, mesh->originalVtx[v].y, mesh->originalVtx[v].z);
           printf("  UV: (%.3f, %.3f)\n", mesh->originalVtx[v].u, mesh->originalVtx[v].v);
           printf("  Color: 0x%08X\n", mesh->originalVtx[v].argb);
           for(int j=0;j<4;j++)
           {
                printf("  WeightBone indice:%d weight:%f \n", mesh->vertexWeights[v].indiceBone[j],  mesh->vertexWeights[v].weight[j]);
           }
           
       }
   }

   if(model->armature) {
       printf("\n=== Armature ===\n");
       for(int i = 0; i < model->armature->bone_count; i++) {
           dct_bone_t* bone = &model->armature->bones[i];
           printf("\nBone %d: %s\n", i, bone->name);
           printf("Parent: %d\n", bone->parent_index);
           printf("Matrix:\n");
           for(int m = 0; m < 4; m++) {
               printf("  %.3f %.3f %.3f %.3f\n", 
                   bone->matrix[m*4], bone->matrix[m*4+1], 
                   bone->matrix[m*4+2], bone->matrix[m*4+3]);
           }
       }
   }

   if(model->anim_count > 0) {
       printf("\n=== Animations (%d) ===\n", model->anim_count);
       /*
       for(int i = 0; i < model->anim_count; i++) {
           dct_animation_t* anim = &model->animations[i];
           printf("\nAnimation: %s\n", anim->name);
           printf("Frames: %d-%d (Total: %d)\n", 
                  anim->start_frame, anim->end_frame, anim->frame_count);
           
           
           for(int f = 0; f < anim->frame_count; f++) {
               dct_keyframe_t* frame = &anim->frames.frames[f];
               printf("\nFrame %d (Time: %.3f) Bones: %d\n", 
                      f, frame->time, frame->bone_count);
                      
               
               for(int b = 0; b < frame->bone_count; b++) {
                   dct_bone_keyframe_t* bframe = &frame->bones[b];
                   printf("  Bone: %s\n", bframe->name);
                   printf("    Pos: (%.3f, %.3f, %.3f)\n", 
                          bframe->location[0], bframe->location[1], bframe->location[2]);
                   printf("    Rot: (%.3f, %.3f, %.3f, %.3f)\n",
                          bframe->rotation[0], bframe->rotation[1],
                          bframe->rotation[2], bframe->rotation[3]);
                   printf("    Scale: (%.3f, %.3f, %.3f)\n",
                          bframe->scale[0], bframe->scale[1], bframe->scale[2]);
               }
               
           }
           
       }
       */
   }

   // Affichage de l'état de l'animation (si disponible)
   /*
    if (&model->anim_state) {
        dct_animation_state_t* state = &model->anim_state;
        printf("\n=== Animation State ===\n");
        printf("Current Time: %.3f\n", state->current_time);
        printf("Playback Speed: %.3f\n", state->playback_speed);
        printf("Is Playing: %s\n", state->is_playing ? "Oui" : "Non");
        printf("Is Looping: %s\n", state->is_looping ? "Oui" : "Non");
        printf("Current Frame: %d\n", state->current_frame);

        if (state->current_animation) {
            printf("\n=== Current Animation ===\n");
            printf("Name: %s\n", state->current_animation->name);
            printf("Start Frame: %d\n", state->current_animation->start_frame);
            printf("End Frame: %d\n", state->current_animation->end_frame);
            printf("Frame Count: %d\n", state->current_animation->frame_count);
        } else {
            printf("\nErreur : current_animation est un pointeur nul.\n");
        }
    } else {
        printf("\nAucun état d'animation trouvé.\n");
    }
    if(model->armature) 
    {
        printAllArmature(model->armature->bones);
    }

    */
   



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

void dct_debug_animation_state(dct_animation_state_t* state, dct_model_t* model) {
    printf("\n=== ANIMATION DEBUG INFO ===\n");
    
    // État de l'animation
    printf("Animation State:\n");
    printf("  Playing: %s\n", state->is_playing ? "Yes" : "No");
    printf("  Looping: %s\n", state->is_looping ? "Yes" : "No");
    printf("  Current Time: %.2f\n", state->current_time);
    printf("  Playback Speed: %.2f\n", state->playback_speed);
    
    if (state->current_animation) {
        printf("\nCurrent Animation:\n");
        printf("  Name: %s\n", state->current_animation->name);
        printf("  Frame Range: %d - %d\n", 
               state->current_animation->start_frame,
               state->current_animation->end_frame);
        printf("  Total Frames: %d\n", state->current_animation->frame_count);
    } else {
        printf("\nNo animation currently selected.\n");
    }
    
    // État de l'armature
    if (model && model->armature) {
        printf("\nArmature Status:\n");
        printf("Total Bones: %d\n", model->armature->bone_count);
        
        for (uint32_t i = 0; i < model->armature->bone_count; i++) {
            dct_bone_t* bone = &model->armature->bones[i];
            printf("\nBone[%d]: %s\n", i, bone->name);
            printf("  Parent Index: %d\n", bone->parent_index);
            
            // Afficher la matrice de transformation actuelle
            printf("  Current Transform Matrix:\n");
            for (int row = 0; row < 4; row++) {
                printf("    ");
                for (int col = 0; col < 4; col++) {
                    printf("%.3f ", bone->matrix[row * 4 + col]);
                }
                printf("\n");
            }

            // Si une animation est en cours, afficher la frame courante pour cet os
            if (state->current_animation && state->is_playing) {
                // Trouver la keyframe courante pour cet os
                for (uint32_t f = 0; f < state->current_animation->frame_count; f++) {
                    dct_keyframe_t* frame = &state->current_animation->frames.frames[f];
                    for (uint32_t b = 0; b < frame->bone_count; b++) {
                        if (strcmp(frame->bones[b].name, bone->name) == 0) {
                            printf("  Current Animation Data:\n");
                            printf("    Location: %.2f, %.2f, %.2f\n",
                                   frame->bones[b].location[0],
                                   frame->bones[b].location[1],
                                   frame->bones[b].location[2]);
                            printf("    Rotation: %.2f, %.2f, %.2f, %.2f\n",
                                   frame->bones[b].rotation[0],
                                   frame->bones[b].rotation[1],
                                   frame->bones[b].rotation[2],
                                   frame->bones[b].rotation[3]);
                            printf("    Scale: %.2f, %.2f, %.2f\n",
                                   frame->bones[b].scale[0],
                                   frame->bones[b].scale[1],
                                   frame->bones[b].scale[2]);
                            break;
                        }
                    }
                    break;
                }
            }
        }
    } else {
        printf("\nNo armature found in model.\n");
    }

    printf("\n=== END DEBUG INFO ===\n\n");
}


void testInternalMatrix()
{
        matrix_t mat_trans = {
            1, 0, 0, 5,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1
        };

        // Rotation 90° sur Y
        matrix_t mat_rot = {
            0,  0, -1, 0,
            0,  1,  0, 0,
            1,  0,  0, 0,
            0,  0,  0, 1
        };
        matrix_t mat_internal;
        // Test
        mat_identity();
        mat_apply(&mat_rot);    // Première opération
        mat_store(&mat_internal);
        print_matrix(&mat_internal);
        mat_apply(&mat_trans);  // Deuxième opération
        mat_store(&mat_internal);
        print_matrix(&mat_internal);

        // Rotation 90° sur Y
        matrix_t mat_rot1 = {
            0,  0, -1, 0,
            0,  1,  0, 0,
            1,  0,  0, 0,
            0,  0,  0, 1
        };

        // Rotation 90° sur X
        matrix_t mat_rot2 = {
            1,  0,  0, 0,
            0,  0, -1, 0,
            0,  1,  0, 0,
            0,  0,  0, 1
        };



        // Test 1 : Y puis X
        mat_identity();
        mat_apply(&mat_rot2);  // X
        mat_apply(&mat_rot1);  // Y
        mat_store(&mat_internal);
        print_matrix(&mat_internal);

        // Test 2 : X puis Y
        mat_identity();
        mat_apply(&mat_rot1);  // Y
        mat_apply(&mat_rot2);  // X
        mat_store(&mat_internal);
        print_matrix(&mat_internal);


        mat_identity();
        mat_rotate_y(3.14f/2.0f);
        printf("matrice rotate Y 90\n");
        mat_store(&mat_internal);
        print_matrix(&mat_internal);

        mat_identity();
        mat_translate(5.0f, 0.0f, 0.0f);
        printf("matrice translate X 5\n");
        mat_store(&mat_internal);
        print_matrix(&mat_internal);


        printf("calcul test rotate translate avec mat_rotat_y et mat_translate\n");
        mat_identity();
        mat_rotate_y(3.14f/2.0f);
        mat_translate(5.0f, 0.0f, 0.0f);        
        mat_store(&mat_internal);
        print_matrix(&mat_internal);

       
        

        printf("calcul test rotate translate avec les matapply()\n");
        mat_identity();
        mat_apply(&mat_rot1);  // Y
        mat_apply(&mat_trans);  // X  
        
        mat_store(&mat_internal);
        print_matrix(&mat_internal);

        printf("calcul test rotate translate TRANSPOSE avec les matapply\n");
        transposeMatrix(mat_rot1);
        transposeMatrix(mat_trans);
        mat_identity();
        mat_apply(&mat_rot1);  // Y
        mat_apply(&mat_trans);  // X  
        
        mat_store(&mat_internal);
        print_matrix(&mat_internal);


}


void dct_deform_model(dct_model_t *model)
{
    
    //printf("FN dct deform mesh \n");
    // Pour chaque vertex du mesh
    
    for (int i_mesh=0; i_mesh<model->meshesCount; i_mesh++)
    {
        
        dct_mesh_t *mesh = &model->meshes[i_mesh];
        //printf("model count %d mesh name %s \n",model->meshesCount,mesh->name);
        
        if(!mesh || !model->armature || !mesh->vertexWeights || !mesh->originalVtx || !mesh->animatedVtx) {
        return;
        }

        
        for(int i = 0; i < mesh->vtxCount; i++) 
        {
            float final_pos[3] = {0.0f, 0.0f, 0.0f};
            //printf("mesh origina Vtx : %d %f %f %f \n",i,mesh->originalVtx[i].x,mesh->originalVtx[i].y,mesh->originalVtx[i].z);
            // Position originale du vertex
            float orig_pos[3] = {
                mesh->originalVtx[i].x,
                mesh->originalVtx[i].y,
                mesh->originalVtx[i].z
            };
            

            // Pour chaque influence sur ce vertex
            int weightStartIdx = i * 4; // Supposant max 4 influences par vertex
            
            for(int w = 0; w < 4; w++) 
            {
                
                dct_vertexWeight_t weight = mesh->vertexWeights[i];
                
                // Si le poids est 0, on skip
                if(weight.weight[w] <= 0.0f) continue;
                
                // Récupérer l'os correspondant
                if(weight.indiceBone[w] >= model->armature->bone_count) continue;
                dct_bone_t* bone = &model->armature->bones[weight.indiceBone[w]];

                // Transformer le point par la matrice de l'os
                //float transformed_pos[3];
                float w_component = 1.0f;  // Pour la transformation homogène

                // Utiliser les macros de KOS pour la transformation
                mat_identity();
            
                float internalMat[16];

                //mat_apply(bone->localMatrix);
                //mat_apply(bone->invBindMatrix);
                mat_apply(bone->finalMatrix);
                mat_apply(bone->invBindMatrix);
                mat_trans_single3_nodiv(orig_pos[0], orig_pos[1], orig_pos[2]);
                // transformed_pos[0] = orig_pos[0];
                // transformed_pos[1] = orig_pos[1];
                // transformed_pos[2] = orig_pos[2];

                // Ajouter la contribution pondérée de cet os
                final_pos[0] += orig_pos[0] * weight.weight[w];
                final_pos[1] += orig_pos[1] * weight.weight[w];
                final_pos[2] += orig_pos[2] * weight.weight[w];
            }
            
            // Mettre à jour le vertex déformé
            mesh->animatedVtx[i].x = final_pos[0];
            mesh->animatedVtx[i].y = final_pos[1];
            mesh->animatedVtx[i].z = final_pos[2];
            //printf("animatedVtx %d x%f y%f z%f offsetAnimFinalPos x%f y%f z%f \n",i,mesh->animatedVtx[i].x,mesh->animatedVtx[i].y,mesh->animatedVtx[i].z,final_pos[0],final_pos[1],final_pos[2] );
            
            // Copier les autres attributs du vertex (UV, couleur, etc.)
            mesh->animatedVtx[i].u = mesh->originalVtx[i].u;
            mesh->animatedVtx[i].v = mesh->originalVtx[i].v;
            mesh->animatedVtx[i].argb = mesh->originalVtx[i].argb;
            mesh->animatedVtx[i].oargb = mesh->originalVtx[i].oargb;
            mesh->animatedVtx[i].flags = mesh->originalVtx[i].flags;
        }
        
    }
    
   

}


void dct_deform_mesh(dct_mesh_t* mesh, dct_armature_t* armature) {
    if(!mesh || !armature || !mesh->vertexWeights || !mesh->originalVtx || !mesh->animatedVtx) {
        return;
    }
    //printf("FN dct deform mesh \n");
    // Pour chaque vertex du mesh
    for(int i = 0; i < mesh->vtxCount; i++) {
        float final_pos[3] = {0.0f, 0.0f, 0.0f};
        //printf("mesh origina Vtx : %d %f %f %f \n",i,mesh->originalVtx[i].x,mesh->originalVtx[i].y,mesh->originalVtx[i].z);
        // Position originale du vertex
        float orig_pos[3] = {
            mesh->originalVtx[i].x,
            mesh->originalVtx[i].y,
            mesh->originalVtx[i].z
        };
        

        // Pour chaque influence sur ce vertex
        int weightStartIdx = i * 4; // Supposant max 4 influences par vertex
        for(int w = 0; w < 4; w++) {
            dct_vertexWeight_t weight = mesh->vertexWeights[i];
            
            // Si le poids est 0, on skip
            if(weight.weight[w] <= 0.0f) continue;
            
            // Récupérer l'os correspondant
            if(weight.indiceBone[w] >= armature->bone_count) continue;
            dct_bone_t* bone = &armature->bones[weight.indiceBone[w]];

            // Transformer le point par la matrice de l'os
            float transformed_pos[3];
            float w_component = 1.0f;  // Pour la transformation homogène

            // Utiliser les macros de KOS pour la transformation
            mat_identity();
           
            float internalMat[16];

            //mat_apply(&bone->localMatrix);
            //mat_apply(&bone->invBindMatrix);

            mat_apply(bone->finalMatrix);

            //printf("internal matrix after apply local matrix and invbindMatrix\n");
            //mat_store(&internalMat);
            //print_matrix(&internalMat);
            
            

            //printf("original pos before: %f %f %f \n",orig_pos[0],orig_pos[1],orig_pos[2]);
            //mat_trans_nodiv(orig_pos[0], orig_pos[1], orig_pos[2], w_component);
            mat_trans_single3_nodiv(orig_pos[0], orig_pos[1], orig_pos[2]);
            transformed_pos[0] = orig_pos[0];
            transformed_pos[1] = orig_pos[1];
            transformed_pos[2] = orig_pos[2];
            //printf("current Bone :%s \n",bone->name);
            //printf("print local matrix \n");
            //printf("bone %s \n",bone->name);
            // print_matrix(&bone->finalMatrix);
            //printf("original pos after: %f %f %f \n",orig_pos[0],orig_pos[1],orig_pos[2]);
           
            //printf("transformed Pos Vtx : i:%d windice:%d weight:%f Trposition:%f %f %f \n",i, w, weight.weight[w] ,transformed_pos[0],transformed_pos[1],transformed_pos[2]);
        

            // Ajouter la contribution pondérée de cet os
            final_pos[0] = transformed_pos[0] * weight.weight[w];
            final_pos[1] = transformed_pos[1] * weight.weight[w];
            final_pos[2] = transformed_pos[2] * weight.weight[w];
        }

        // Mettre à jour le vertex déformé
        mesh->animatedVtx[i].x = final_pos[0];
        mesh->animatedVtx[i].y = final_pos[1];
        mesh->animatedVtx[i].z = final_pos[2];
        //printf("animatedVtx %d x%f y%f z%f offsetAnimFinalPos x%f y%f z%f \n",i,mesh->animatedVtx[i].x,mesh->animatedVtx[i].y,mesh->animatedVtx[i].z,final_pos[0],final_pos[1],final_pos[2] );
        
        // Copier les autres attributs du vertex (UV, couleur, etc.)
        mesh->animatedVtx[i].u = mesh->originalVtx[i].u;
        mesh->animatedVtx[i].v = mesh->originalVtx[i].v;
        mesh->animatedVtx[i].argb = mesh->originalVtx[i].argb;
        mesh->animatedVtx[i].oargb = mesh->originalVtx[i].oargb;
        mesh->animatedVtx[i].flags = mesh->originalVtx[i].flags;
    }
    
    // Si on a des normales, les transformer aussi
    // if(mesh->vertexNormals) {
    //     for(int i = 0; i < mesh->vtxCount; i++) {
    //         float final_normal[3] = {0.0f, 0.0f, 0.0f};
    //         float orig_normal[3] = {
    //             mesh->vertexNormals[i].x,
    //             mesh->vertexNormals[i].y,
    //             mesh->vertexNormals[i].z
    //         };

    //         int weightStartIdx = i * 4;
    //         for(int w = 0; w < 4; w++) {
    //             dct_vertexWeight_t weight = mesh->vertexWeights[weightStartIdx + w];
    //             if(weight.weight[w] <= 0.0f) continue;
    //             if(weight.indiceBone[w] >= armature->bone_count) continue;
                
    //             dct_bone_t* bone = &armature->bones[weight.indiceBone[w]];

    //             // Transformer la normale (sans translation)
    //             float transformed_normal[3];
    //             mat_load((matrix_t*)bone->finalMatrix);
    //             mat_trans_normal3(orig_normal[0], orig_normal[1], orig_normal[2]);
    //             transformed_normal[0] = orig_normal[0];
    //             transformed_normal[1] = orig_normal[1];
    //             transformed_normal[2] = orig_normal[2];

    //             final_normal[0] += transformed_normal[0] * weight.weight[w];
    //             final_normal[1] += transformed_normal[1] * weight.weight[w];
    //             final_normal[2] += transformed_normal[2] * weight.weight[w];
    //         }

    //         // Normaliser le vecteur résultant
    //         float length = sqrtf(final_normal[0] * final_normal[0] + 
    //                            final_normal[1] * final_normal[1] + 
    //                            final_normal[2] * final_normal[2]);
            
    //         if(length > 0.0001f) {
    //             mesh->vertexNormals[i].x = final_normal[0] / length;
    //             mesh->vertexNormals[i].y = final_normal[1] / length;
    //             mesh->vertexNormals[i].z = final_normal[2] / length;
    //         }
    //     }
    // }

    //printf("-- END FN dct deform mesh \n");
}



void dct_applyParentBonesArmatureMatrix(dct_bone_t *bone) 
{ 
    // Protections contre les erreurs
    if (!bone) return;
    
    // Limiter la profondeur de récursion
    static int depth = 0;
    if (depth > MAX_BONE_DEPTH) return;
    
    if(bone->parent_index > -1 && bone->parent != NULL)
    {
        depth++;
        //print_matrix(bone->parent->invBindMatrix);
        dct_applyParentBonesArmatureMatrix(bone->parent);
        mat_apply(bone->parent->localMatrix);
        mat_apply(bone->parent->invBindMatrix);
        depth--;
    }
}


void dct_armature_update(dct_model_t* model)
{

    for (int i = 0; i < model->armature->bone_count; i++)
    {
        // Passage du pointeur, pas d'une copie
        dct_bone_t *bone = &model->armature->bones[i];
        
        if(bone->parent_index < 0)
        {
            mat_identity();
            dct_armature_bones_update(bone);
        }
    }
}



void dct_armature_bones_update(dct_bone_t *bone)
{
    //printf("--- Armature bone %s \ndebugOriginalmatrix\n",bone->name);
    //print_matrix(bone->animatedMatrix);
    

    if(bone->parent_index>-1)
    {
        //float mdbg[16] = {0};
        
        mat_identity();
        //printf("apply parent bone %s \n",bone->name);
        dct_applyParentBonesArmatureMatrix(bone);
        // mat_store(&mdbg);
        // printf("all matrice invbind*local of parent bones multiplied \n");
        // print_matrix(mdbg);
        mat_apply(bone->localMatrix);
        //mat_apply(bone->invBindMatrix);
        // mat_store(&mdbg);
        // printf("matrice final after apply local matrix\n");
        // print_matrix(mdbg);
        //printf("apply parent bones %s \n",bone->name);
        
        mat_store(&bone->finalMatrix);


    }

    if(bone->parent_index == -1)
    {
        bone->finalMatrix[0] = bone->localMatrix[0];
        bone->finalMatrix[1] = bone->localMatrix[1];
        bone->finalMatrix[2] = bone->localMatrix[2];

        bone->finalMatrix[4] = bone->localMatrix[4];
        bone->finalMatrix[5] = bone->localMatrix[5];
        bone->finalMatrix[6] = bone->localMatrix[6];

        bone->finalMatrix[8] = bone->localMatrix[8];
        bone->finalMatrix[9] = bone->localMatrix[9];
        bone->finalMatrix[10] = bone->localMatrix[10];

        bone->finalMatrix[12] = bone->localMatrix[12];
        bone->finalMatrix[13] = bone->localMatrix[13];
        bone->finalMatrix[14] = bone->localMatrix[14];
    }

    if(bone->children_count>0)
    {
         for(int i=0; i<bone->children_count; i++)
        {
            dct_bone_t *boneChild = bone->children[i];
            
            dct_armature_bones_update(boneChild);
        }
    }

}



void dct_animation_update(dct_animation_state_t* state, float delta_time, dct_model_t* model) {
    if (!state->is_playing || !state->current_animation) {
        return;
    }

    matrix_t m001;

    //printf("transmat \n");
    //print_matrix(trans_mat); 
    //printf("rotmat \n");
    //printf("state currentTime %d  stateCurrentAnimaitonFramcount  \n",state->current_time);
    //print_matrix(rot_mat);
    
    state->current_time += delta_time * state->playback_speed;
    float anim_duration = state->current_animation->frames.frames[state->current_animation->frame_count - 1].time;

    if (state->current_time >= anim_duration) {
        if (state->is_looping) {
            state->current_time = fmodf(state->current_time, anim_duration);
        } else {
            state->current_time = anim_duration;
            state->is_playing = false;
            return;
        }
    }

    uint32_t frame1 = 0;
    uint32_t frame2 = 0;
    for (uint32_t i = 0; i < state->current_animation->frame_count - 1; i++) {
        if (state->current_time >= state->current_animation->frames.frames[i].time &&
            state->current_time < state->current_animation->frames.frames[i + 1].time) {
            frame1 = i;
            frame2 = i + 1;
            break;
        }
    }

    float time1 = state->current_animation->frames.frames[frame1].time;
    float time2 = state->current_animation->frames.frames[frame2].time;
    float t = (state->current_time - time1) / (time2 - time1);
    
    for (uint32_t i = 0; i < state->current_animation->frames.frames[frame1].bone_count; i++) {
        dct_bone_keyframe_t* bone1 = &state->current_animation->frames.frames[frame1].bones[i];
        dct_bone_keyframe_t* bone2 = &state->current_animation->frames.frames[frame2].bones[i];

        // Location
        vec3f_t interpolated_loc = {
            bone1->location[0] * (1 - t) + bone2->location[0] * t,
            bone1->location[1] * (1 - t) + bone2->location[1] * t,
            bone1->location[2] * (1 - t) + bone2->location[2] * t
        };
        
        // Scale
        vec3f_t interpolated_scl = {
            bone1->scale[0] * (1 - t) + bone2->scale[0] * t,
            bone1->scale[1] * (1 - t) + bone2->scale[1] * t,
            bone1->scale[2] * (1 - t) + bone2->scale[2] * t
        };

        // Rotation (quaternion) interpolation
        float interpolated_rot[4];
        interpolated_rot[0] = bone1->rotation[0] * (1 - t) + bone2->rotation[0] * t;
        interpolated_rot[1] = bone1->rotation[1] * (1 - t) + bone2->rotation[1] * t;
        interpolated_rot[2] = bone1->rotation[2] * (1 - t) + bone2->rotation[2] * t;
        interpolated_rot[3] = bone1->rotation[3] * (1 - t) + bone2->rotation[3] * t;
        

        // Normaliser le quaternion interpolé
        float rot_norm = sqrtf(
            interpolated_rot[0] * interpolated_rot[0] +
            interpolated_rot[1] * interpolated_rot[1] +
            interpolated_rot[2] * interpolated_rot[2] +
            interpolated_rot[3] * interpolated_rot[3]
        );
        
        if (rot_norm > 0.0f) {
            interpolated_rot[0] /= rot_norm;
            interpolated_rot[1] /= rot_norm;
            interpolated_rot[2] /= rot_norm;
            interpolated_rot[3] /= rot_norm;
        }
        matrix_t trans_mat;
        matrix_t rot_mat;
        matrix_t scale_mat;
        matrix_t local_mat;
        //printf("\nframe1:%d  frame 2:%d  bone:%d \n",frame1,frame2,i );
        for (uint32_t bone_idx = 0; bone_idx < model->armature->bone_count; bone_idx++) {
            
            if (strcmp(model->armature->bones[bone_idx].name, bone1->name) == 0) 
            {
                

               float trans_values[4][4] = {
                    {1.0f, 0.0f, 0.0f, 0.0f},
                    {0.0f, 1.0f, 0.0f, 0.0f},
                    {0.0f, 0.0f, 1.0f, 0.0f},
                    {interpolated_loc.x, 
                    interpolated_loc.y, 
                    interpolated_loc.z, 1.0f}  // Translation dans la dernière ligne
                };

                //printf("interpolation Translated %f %f %f \n",interpolated_loc.x,interpolated_loc.y,interpolated_loc.z);
                memcpy(&trans_mat, trans_values, sizeof(matrix_t));

                // Rotation matrix (transposée pour Dreamcast)
                float qx = -interpolated_rot[1];
                float qy = -interpolated_rot[2];
                float qz = -interpolated_rot[3];
                float qw = interpolated_rot[0];

                // Matrice de rotation transposée
                float rot_values[4][4] = {
                    {1.0f - 2.0f*(qy*qy + qz*qz),  2.0f*(qx*qy - qw*qz),         2.0f*(qx*qz + qw*qy),         0.0f},
                    {2.0f*(qx*qy + qw*qz),         1.0f - 2.0f*(qx*qx + qz*qz),  2.0f*(qy*qz - qw*qx),         0.0f},
                    {2.0f*(qx*qz - qw*qy),         2.0f*(qy*qz + qw*qx),         1.0f - 2.0f*(qx*qx + qy*qy),  0.0f},
                    {0.0f,                          0.0f,                          0.0f,                          1.0f}
                };
                memcpy(&rot_mat, rot_values, sizeof(matrix_t));

                // Scale matrix (transposée pour Dreamcast)
                float scale_values[4][4] = {
                    {interpolated_scl.x, 0.0f, 0.0f, 0.0f},
                    {0.0f, interpolated_scl.y, 0.0f, 0.0f},
                    {0.0f, 0.0f, interpolated_scl.z, 0.0f},
                    {0.0f, 0.0f, 0.0f, 1.0f}
                };
                memcpy(&scale_mat, scale_values, sizeof(matrix_t));
                
                // Appliquer les transformations
                
                mat_identity();
                //printf("print bindMatrix \n");
                //print_matrix((matrix_t*)model->armature->bones[bone_idx].bindMatrix);
                mat_apply((matrix_t*)model->armature->bones[bone_idx].bindMatrix);
                mat_apply(&scale_mat);
                mat_apply(&rot_mat);
                mat_apply(&trans_mat);
                
                

                if(bone_idx == 0)
                {
                    
                    
                }
                
               
                //float mdbg[16] = {0};
                //mat_store(&mdbg);

                // if(bone_idx==11)
                // {
                //     printf("--------------------\n");
                //     // printf("frame1: %d frame2: %d time1: %f time2: %f \n", frame1, frame2 ,time1,time2);
                //      printf("bone %s \n",model->armature->bones[bone_idx].name);
                //     // printf("print rot mat \n");
                //     // print_matrix(rot_mat);
                //      printf("print apply rot trans scale \n");
                //      print_matrix(mdbg);
                // }
                
                
                //float mdbg[4][4] = { {0.0f,0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f,0.0f}};
                
                //mat_store(&m001);
                mat_store((matrix_t*)model->armature->bones[bone_idx].localMatrix);


                // if(bone_idx==1)
                // {
                //     //printf("frame1: %d frame2: %d time1: %f time2: %f \n", frame1, frame2 ,time1,time2);
                //     //printf("bone %s \n",model->armature->bones[bone_idx].name);
                //     // printf("print local matrix final\n");
                //     // print_matrix(model->armature->bones[bone_idx].localMatrix);
                // }
                break;
            }
            
        }
        
    }

    
    //printf("\n end frame bones \n ");

}



void dct_draw_gizmo() {
    const float AXIS_LENGTH = 30.0f;
    const float AXIS_WIDTH = 2.0f;  // Largeur des axes
    const float SCREEN_OFFSET = 50.0f;

    pvr_poly_cxt_t cxt;
    pvr_poly_cxt_col(&cxt, PVR_LIST_OP_POLY);
    cxt.gen.culling = PVR_CULLING_NONE;
    cxt.depth.write = PVR_DEPTHWRITE_ENABLE;
    cxt.depth.comparison = PVR_DEPTHCMP_GEQUAL;
    
    pvr_poly_hdr_t hdr;
    pvr_poly_compile(&hdr, &cxt);
    pvr_prim(&hdr, sizeof(pvr_poly_hdr_t));

    vec3f_t base_pos = {SCREEN_OFFSET, SCREEN_OFFSET, 1.0f};

    // Pour chaque axe, créer deux triangles (6 vertices)
    pvr_vertex_t vertices[6];

    // Axe X (Rouge)
    // Premier triangle
    vertices[0].x = base_pos.x;
    vertices[0].y = base_pos.y - AXIS_WIDTH;
    vertices[0].z = 1.0f;
    vertices[0].argb = 0xFFFF0000;
    vertices[0].oargb = 0;
    vertices[0].flags = PVR_CMD_VERTEX;

    vertices[1].x = base_pos.x + AXIS_LENGTH;
    vertices[1].y = base_pos.y;
    vertices[1].z = 1.0f;
    vertices[1].argb = 0xFFFF0000;
    vertices[1].oargb = 0;
    vertices[1].flags = PVR_CMD_VERTEX;

    vertices[2].x = base_pos.x;
    vertices[2].y = base_pos.y + AXIS_WIDTH;
    vertices[2].z = 1.0f;
    vertices[2].argb = 0xFFFF0000;
    vertices[2].oargb = 0;
    vertices[2].flags = PVR_CMD_VERTEX_EOL;

    // Envoyer l'axe X
    for(int i = 0; i < 3; i++) {
        pvr_prim(&vertices[i], sizeof(pvr_vertex_t));
    }

    // Axe Y (Vert)
    vertices[0].x = base_pos.x - AXIS_WIDTH;
    vertices[0].y = base_pos.y;
    vertices[0].z = 1.0f;
    vertices[0].argb = 0xFF00FF00;
    vertices[0].flags = PVR_CMD_VERTEX;

    vertices[1].x = base_pos.x + AXIS_WIDTH;
    vertices[1].y = base_pos.y;
    vertices[1].z = 1.0f;
    vertices[1].argb = 0xFF00FF00;
    vertices[1].flags = PVR_CMD_VERTEX;

    vertices[2].x = base_pos.x;
    vertices[2].y = base_pos.y + AXIS_LENGTH;
    vertices[2].z = 1.0f;
    vertices[2].argb = 0xFF00FF00;
    vertices[2].flags = PVR_CMD_VERTEX_EOL;

    // Envoyer l'axe Y
    for(int i = 0; i < 3; i++) {
        pvr_prim(&vertices[i], sizeof(pvr_vertex_t));
    }

    // Axe Z (Bleu)
    vertices[0].x = base_pos.x - AXIS_WIDTH;
    vertices[0].y = base_pos.y - AXIS_WIDTH;
    vertices[0].z = 1.0f;
    vertices[0].argb = 0xFF0000FF;
    vertices[0].flags = PVR_CMD_VERTEX;

    vertices[1].x = base_pos.x + AXIS_WIDTH;
    vertices[1].y = base_pos.y + AXIS_WIDTH;
    vertices[1].z = 1.0f;
    vertices[1].argb = 0xFF0000FF;
    vertices[1].flags = PVR_CMD_VERTEX;

    vertices[2].x = base_pos.x - AXIS_LENGTH/2;
    vertices[2].y = base_pos.y - AXIS_LENGTH/2;
    vertices[2].z = 1.0f;
    vertices[2].argb = 0xFF0000FF;
    vertices[2].flags = PVR_CMD_VERTEX_EOL;

    // Envoyer l'axe Z
    for(int i = 0; i < 3; i++) {
        pvr_prim(&vertices[i], sizeof(pvr_vertex_t));
    }
}



void dct_draw_armature(dct_armature_t* armature) {
    if (!armature) return;
    dct_camera_t *currCam = getCurrentCamera(); 
    // Paramètres du cercle
    const float BONE_RADIUS = 5.0f;
    const int SEGMENTS = 8;  // Nombre de segments pour le cercle
    
    // Créer un cercle de référence
    vec3f_t circle_points[SEGMENTS];
    for(int i = 0; i < SEGMENTS; i++) {
        float angle = (2.0f * M_PI * i) / SEGMENTS;
        circle_points[i].x = cosf(angle) * BONE_RADIUS;
        circle_points[i].y = sinf(angle) * BONE_RADIUS;
        circle_points[i].z = 0.0f;
    }

    // Pour chaque os
    for (uint32_t bone_idx = 0; bone_idx < armature->bone_count; bone_idx++) {
        dct_bone_t* bone = &armature->bones[bone_idx];
        
        // Position du bone (centre du cercle)
        vec3f_t bone_pos = {0.0f, 0.0f, 0.0f};
        mat_identity();
        //mat_perspective(currCam->offsetCenterScreen[0], currCam->offsetCenterScreen[1], currCam->cot, currCam->znear, currCam->zfar);
        //mat_translate(currCam->position.x, currCam->position.y, currCam->position.z);
        mat_apply(currCam->final);

        
        
        mat_apply((matrix_t*)bone->finalMatrix);
        //mat_store(&bone->finalMatrix);
        
        //print_matrix(bone->finalMatrix);
        
        mat_trans_single3(bone_pos.x, bone_pos.y, bone_pos.z);
        //printf("\n bone %d/%d %s pos %f %f %f \n",bone_idx, armature->bone_count,bone->name, bone_pos.x,bone_pos.y,bone_pos.z);
        //printf("bone id%d x:%f y:%f z:%f  \n", bone_idx, bone_pos.x, bone_pos.y, bone_pos.z);
        
        // Couleur du bone
        uint32_t bone_color = 0xFF000000 |  // Alpha
                             ((bone_idx * 50) << 16) |  // R
                             ((255 - bone_idx * 30) << 8) |  // G
                             (bone_idx * 20);  // B
        
        if(bone_idx == 0)
        {
            bone_color = 0xFF0000FF;
        }
        // Configuration du PVR pour le rendu
        pvr_poly_cxt_t cxt;
        pvr_poly_cxt_col(&cxt, PVR_LIST_OP_POLY);
        cxt.gen.culling = PVR_CULLING_NONE; 
        cxt.depth.write = PVR_DEPTHWRITE_ENABLE;
        cxt.depth.comparison = PVR_DEPTHCMP_GEQUAL;
        cxt.blend.src = PVR_BLEND_ONE;
        cxt.blend.dst = PVR_BLEND_ZERO;
        cxt.blend.src_enable = PVR_BLEND_DISABLE;
        cxt.blend.dst_enable = PVR_BLEND_DISABLE; 
        
        pvr_poly_hdr_t hdr;
        pvr_poly_compile(&hdr, &cxt);
        pvr_prim(&hdr, sizeof(pvr_poly_hdr_t));
        float line_z_depth = 1;
        // Dessiner le cercle comme une série de triangles
        for(int i = 0; i < SEGMENTS; i++) {
            int next = (i + 1) % SEGMENTS;
            
            // Triangle pour ce segment
            pvr_vertex_t vertices[3];
            
            // Centre
            vertices[0].x = bone_pos.x;
            vertices[0].y = bone_pos.y;
            vertices[0].z = bone_pos.z+0.1;
            vertices[0].argb = bone_color;
            vertices[0].oargb = 0;
            vertices[0].flags = PVR_CMD_VERTEX;

            // Point courant
            vertices[1].x = bone_pos.x + circle_points[i].x;
            vertices[1].y = bone_pos.y + circle_points[i].y;
            vertices[1].z = bone_pos.z + circle_points[i].z+0.1;
            vertices[1].oargb = 0;
            vertices[1].argb = bone_color;
            vertices[1].flags = PVR_CMD_VERTEX;

            // Point suivant
            vertices[2].x = bone_pos.x + circle_points[next].x;
            vertices[2].y = bone_pos.y + circle_points[next].y;
            vertices[2].z = bone_pos.z + circle_points[next].z+0.1;
            vertices[2].argb = bone_color;
            vertices[2].oargb = 0;
            vertices[2].flags = PVR_CMD_VERTEX_EOL;

            if( vertices[0].z >=0 &&  vertices[0].z <=1 && vertices[1].z <=1 && vertices[1].z >=0 &&  vertices[2].z <=1 && vertices[2].z >=0  )
            {
                // Envoyer le triangle au PVR
                pvr_prim(&vertices[0], sizeof(pvr_vertex_t));
                pvr_prim(&vertices[1], sizeof(pvr_vertex_t));
                pvr_prim(&vertices[2], sizeof(pvr_vertex_t));
            }

            
        }

        // float offsetBoneEnd[16];
        // memcpy(offsetBoneEnd, bone->localMatrix,sizeof(matrix_t));
        // offsetBoneEnd[2] += bone->localMatrix[2];
        // offsetBoneEnd[6] += bone->localMatrix[6];
        // offsetBoneEnd[10] += bone->localMatrix[10];
        // mat_apply(offsetBoneEnd);

        // Si l'os a un parent, dessiner une ligne de connexion
        vec3f_t parent_pos = {0.0f, 0.0f, 0.0f};
        vec3f_t last_bone_pos = {0.0f, 0.0f, 0.0f};
        line_z_depth = 0.95;
        if (bone->parent_index >= 0) {
            
            mat_identity();
            //mat_perspective(currCam->offsetCenterScreen[0], currCam->offsetCenterScreen[1], currCam->cot, currCam->znear, currCam->zfar);
            //mat_translate(currCam->position.x, currCam->position.y, currCam->position.z);
            mat_apply(currCam->final);
            if(bone->parent!=NULL && bone->parent_index>=0)
            {
                mat_apply(bone->parent->finalMatrix);
            }
            mat_trans_single3(parent_pos.x, parent_pos.y, parent_pos.z);
            
            // Dessiner une ligne entre le parent et l'os
            pvr_vertex_t line[6];
            line[0].x = parent_pos.x-5;
            line[0].y = parent_pos.y;
            line[0].z = parent_pos.z-0.1;
            line[0].argb = 0xFFAAAAFF; // Blanc
            line[0].oargb = 0;
            line[0].flags = PVR_CMD_VERTEX;

            line[1].x = parent_pos.x+5;
            line[1].y = parent_pos.y;
            line[1].z = parent_pos.z-0.1;
            line[1].argb = 0xFFAAAAFF; // Blanc
            line[1].oargb = 0;
            line[1].flags = PVR_CMD_VERTEX;

            line[2].x = bone_pos.x;
            line[2].y = bone_pos.y;
            line[2].z = bone_pos.z-0.1;
            line[2].argb = 0xFF0000FF;
            line[2].oargb = 0;
            line[2].flags = PVR_CMD_VERTEX_EOL;

            line[3].x = parent_pos.x;
            line[3].y = parent_pos.y-5;
            line[3].z = parent_pos.z-0.1;
            line[3].argb = 0xFFAAAAFF; // Blanc
            line[3].oargb = 0;
            line[3].flags = PVR_CMD_VERTEX;

            line[4].x = parent_pos.x;
            line[4].y = parent_pos.y+5;
            line[4].z = parent_pos.z-0.1;
            line[4].argb = 0xFFAAAAFF; // Blanc
            line[4].oargb = 0;
            line[4].flags = PVR_CMD_VERTEX;

            line[5].x = bone_pos.x;
            line[5].y = bone_pos.y;
            line[5].z = bone_pos.z-0.1;
            line[5].argb = 0xFF00FF00;
            line[5].oargb = 0;
            line[5].flags = PVR_CMD_VERTEX_EOL;
            
            pvr_prim(&line[0], sizeof(pvr_vertex_t));
            pvr_prim(&line[1], sizeof(pvr_vertex_t));
            pvr_prim(&line[2], sizeof(pvr_vertex_t));
            pvr_prim(&line[3], sizeof(pvr_vertex_t));
            pvr_prim(&line[4], sizeof(pvr_vertex_t));
            pvr_prim(&line[5], sizeof(pvr_vertex_t));
            
        }
        
        last_bone_pos.x = bone_pos.x;
        last_bone_pos.y = bone_pos.y;
        last_bone_pos.z = bone_pos.z;

        // le dernier bone 
        
        if (bone->children_count == 0)
        {
            // Triangle pour ce segment
            pvr_vertex_t vertices[3];
            bone_pos.x = 0.0f;
            bone_pos.y = 0.0f;
            bone_pos.z = 0.0f;
            
            mat_identity();
            mat_apply(currCam->final);
            
            vec3f_t offsetTr = { bone->finalMatrix[4], bone->finalMatrix[5],bone->finalMatrix[6]};
            mat_translate(offsetTr.x*0.5f, offsetTr.y*0.5f , offsetTr.z*0.5f );
            //mat_apply((matrix_t*)bone->localMatrix);
            mat_apply((matrix_t*)bone->finalMatrix);
            
            
            
            mat_trans_single3(bone_pos.x, bone_pos.y, bone_pos.z);
            // dessiner la pointe du dernier os
            // Dessiner le cercle comme une série de triangles
            for(int i = 0; i < SEGMENTS; i++) {
                int next = (i + 1) % SEGMENTS;
                
                
                // Centre
                vertices[0].x = bone_pos.x;
                vertices[0].y = bone_pos.y;
                vertices[0].z = bone_pos.z-0.1;
                vertices[0].argb = 0xFFFF0000;
                vertices[0].oargb = 0;
                vertices[0].flags = PVR_CMD_VERTEX;

                // Point courant
                vertices[1].x = bone_pos.x + circle_points[i].x;
                vertices[1].y = bone_pos.y + circle_points[i].y;
                vertices[1].z = bone_pos.z + circle_points[i].z-0.1;
                vertices[1].oargb = 0;
                vertices[1].argb = 0xFFFF0000;
                vertices[1].flags = PVR_CMD_VERTEX;

                // Point suivant
                vertices[2].x = bone_pos.x + circle_points[next].x;
                vertices[2].y = bone_pos.y + circle_points[next].y;
                vertices[2].z = bone_pos.z + circle_points[next].z-0.1;
                vertices[2].argb = 0xFFFF0000;
                vertices[2].oargb = 0;
                vertices[2].flags = PVR_CMD_VERTEX_EOL;

                if( vertices[0].z >=0 &&  vertices[0].z <=1 && vertices[1].z <=1 && vertices[1].z >=0 &&  vertices[2].z <=1 && vertices[2].z >=0  )
                {
                    // Envoyer le triangle au PVR
                    pvr_prim(&vertices[0], sizeof(pvr_vertex_t));
                    pvr_prim(&vertices[1], sizeof(pvr_vertex_t));
                    pvr_prim(&vertices[2], sizeof(pvr_vertex_t));
                }

                
            }

             // Dessiner une ligne entre le parent et l'os
            pvr_vertex_t line[6];
            line[0].x = last_bone_pos.x-3;
            line[0].y = last_bone_pos.y;
            line[0].z = last_bone_pos.z-0.1;
            line[0].argb = 0xFFAAFFAA; // Blanc
            line[0].oargb = 0;
            line[0].flags = PVR_CMD_VERTEX;

            line[1].x = last_bone_pos.x+3;
            line[1].y = last_bone_pos.y;
            line[1].z = last_bone_pos.z-0.1;
            line[1].argb = 0xFFAAFFAA; // Blanc
            line[1].oargb = 0;
            line[1].flags = PVR_CMD_VERTEX;

            line[2].x = bone_pos.x;
            line[2].y = bone_pos.y;
            line[2].z = bone_pos.z-0.1;
            line[2].argb = 0xFFFF0000;
            line[2].oargb = 0;
            line[2].flags = PVR_CMD_VERTEX_EOL;

            line[3].x = last_bone_pos.x;
            line[3].y = last_bone_pos.y-3;
            line[3].z = last_bone_pos.z-0.1;
            line[3].argb = 0xFFAAFFAA; // Blanc
            line[3].oargb = 0;
            line[3].flags = PVR_CMD_VERTEX;

            line[4].x = last_bone_pos.x;
            line[4].y = last_bone_pos.y+3;
            line[4].z = last_bone_pos.z-0.1;
            line[4].argb = 0xFFAAFFAA; // Blanc
            line[4].oargb = 0;
            line[4].flags = PVR_CMD_VERTEX;

            line[5].x = bone_pos.x;
            line[5].y = bone_pos.y;
            line[5].z = bone_pos.z-0.1;
            line[5].argb = 0xFFFF0000;
            line[5].oargb = 0;
            line[5].flags = PVR_CMD_VERTEX_EOL;
            
            pvr_prim(&line[0], sizeof(pvr_vertex_t));
            pvr_prim(&line[1], sizeof(pvr_vertex_t));
            pvr_prim(&line[2], sizeof(pvr_vertex_t));
            pvr_prim(&line[3], sizeof(pvr_vertex_t));
            pvr_prim(&line[4], sizeof(pvr_vertex_t));
            pvr_prim(&line[5], sizeof(pvr_vertex_t));

        }
        
        

        
    }

   

}


void setModelMeshPCM(dct_mesh_t *mesh, uint32_t color)
{

    mesh->type = MESH_PCM_OP;
    mesh->renderVtx_PCM = (pvr_vertex_pcm_t*)memalign(32,sizeof(pvr_vertex_pcm_t)*mesh->vtxCount);

    for (int i_v=0; i_v < mesh->vtxCount; i_v++)
    {
        mesh->renderVtx_PCM[i_v].flags = mesh->originalVtx[i_v].flags;
        mesh->renderVtx_PCM[i_v].x = mesh->originalVtx[i_v].x;
        mesh->renderVtx_PCM[i_v].y = mesh->originalVtx[i_v].y;
        mesh->renderVtx_PCM[i_v].z = mesh->originalVtx[i_v].z;

        mesh->renderVtx_PCM[i_v].argb0 = mesh->originalVtx[i_v].argb;
        //cube.meshes[0].renderVtx_PCM[i_v].argb0 = 0xFFFF0000;
        mesh->renderVtx_PCM[i_v].argb1 = color;
        mesh->renderVtx_PCM[i_v].d1 = 0;
        mesh->renderVtx_PCM[i_v].d2 = 0;
        
    }
}

void updateModelModifier(dct_model_t *model)
{
    dct_camera_t *currCam = getCurrentCamera();
    matrix_t mdbg;

    mat_identity();
    // PERSPECTIVE DEFORMATION
    // CAMERA TRANSFROM  Translate  lootAt 
    
    mat_load(currCam->final);
    mat_apply(model->modifier.transform);

    for(int i_m=0; i_m<model->modifier.triCount; i_m++)
    {
        float ax= model->modifier.modifierTrianglesVolOriginal[i_m].ax;
        float ay= model->modifier.modifierTrianglesVolOriginal[i_m].ay;
        float az= model->modifier.modifierTrianglesVolOriginal[i_m].az;
        float aw = 1.0f;
        float bx= model->modifier.modifierTrianglesVolOriginal[i_m].bx;
        float by= model->modifier.modifierTrianglesVolOriginal[i_m].by;
        float bz= model->modifier.modifierTrianglesVolOriginal[i_m].bz;
        float bw = 1.0f;
        float cx= model->modifier.modifierTrianglesVolOriginal[i_m].cx;
        float cy= model->modifier.modifierTrianglesVolOriginal[i_m].cy;
        float cz= model->modifier.modifierTrianglesVolOriginal[i_m].cz;
        float cw = 1.0f;
        mat_trans_single4(ax, ay, az, aw);

        
        mat_trans_single4(bx, by, bz, bw);

        
        mat_trans_single4(cx, cy, cz, cw);

        model->modifier.modifierTrianglesVolFinal[i_m].ax = ax;
        model->modifier.modifierTrianglesVolFinal[i_m].ay = ay;
        model->modifier.modifierTrianglesVolFinal[i_m].az = aw;

        model->modifier.modifierTrianglesVolFinal[i_m].bx = bx;
        model->modifier.modifierTrianglesVolFinal[i_m].by = by;
        model->modifier.modifierTrianglesVolFinal[i_m].bz = bw;
        
        model->modifier.modifierTrianglesVolFinal[i_m].cx = cx;
        model->modifier.modifierTrianglesVolFinal[i_m].cy = cy;
        model->modifier.modifierTrianglesVolFinal[i_m].cz = cw;

        // model->modifier.modifierTrianglesVolFinal[i_m].az = 0.15f;
        // model->modifier.modifierTrianglesVolFinal[i_m].bz = 0.15f;
        // model->modifier.modifierTrianglesVolFinal[i_m].cz = 0.15f;

    }
    

    mat_identity();
}

void updateModel(dct_model_t *model) {
    if(model->DEBUG_MODEL_VTX) { printf("\n\n---------------------------------------------\n---------------------------------------------\n"); printf("\n{ START >>> [UPDATE-MODEL] Model: %s\n", model->name);}
    
    updateModelModifier(model);

    dct_camera_t *currCam = getCurrentCamera(); 
    matrix_t matrix_view;
    matrix_t matrix_MVP;
    matrix_t matrix_persp;

    matrix_t matrix_modelTransform;

    mat_identity();
    mat_load(currCam->transform);
    //mat_translate(currCam->position.x, currCam->position.y, currCam->position.z);
    
    //mat_apply(currCam->transform);
    //mat_lookat(&currCam->position, &currCam->target,&currCam->up);

    mat_translate(model->position.x, model->position.y, model->position.z);
    mat_rotate_y(model->rotation.y);
    mat_rotate_x(model->rotation.x);
    mat_rotate_z(model->rotation.z);
    mat_scale(model->scale.x, model->scale.y, model->scale.z);
    mat_store(&matrix_view);


    // RESET
    mat_identity();
    // PERSPECTIVE DEFORMATION
    //mat_perspective(currCam->offsetCenterScreen[0], currCam->offsetCenterScreen[1], currCam->cot, currCam->znear, currCam->zfar);

    //mat_store(&matrix_persp);
    // CAMERA TRANSFROM  Translate  lootAt 
    mat_load(currCam->final);
    //print_matrix(currCam->final);
    //mat_translate(currCam->position.x, currCam->position.y, currCam->position.z);
    
    // MODEL TRANSFORM 
    mat_translate(model->position.x, model->position.y, model->position.z);
    model->rotation.y = fmod(model->rotation.y,2*F_PI);
    model->rotation.x = fmod(model->rotation.x,2*F_PI);
    model->rotation.z = fmod(model->rotation.z, 2*F_PI);
    mat_rotate_y(model->rotation.y);
    mat_rotate_x(model->rotation.x);
    mat_rotate_z(model->rotation.z);
    mat_scale(model->scale.x, model->scale.y, model->scale.z);
    
    mat_store(&matrix_MVP);
    
    
    pvr_vertex_t *v_test  =  (pvr_vertex_t*)memalign(32, sizeof(pvr_vertex_t));
    pvr_vertex_t *v_test1 =  (pvr_vertex_t*)memalign(32, sizeof(pvr_vertex_t));
    pvr_vertex_t *v_test2 =  (pvr_vertex_t*)memalign(32, sizeof(pvr_vertex_t));
    
    int indice_current_clipping_vtx = 0;
    initClipTrianglesVtx(model);

    for (int i_mesh = 0; i_mesh < model->meshesCount; i_mesh++) {
        // sq_cpy(model->meshes[i_mesh].renderVtx, 
        //        model->meshes[i_mesh].animatedVtx, 
        //        model->meshes[i_mesh].sizeOfVtx);
        memcpy(model->meshes[i_mesh].renderVtx, 
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

                    model->meshes[i_mesh].renderVtx_PCM[i_vtx].x = x;
                    model->meshes[i_mesh].renderVtx_PCM[i_vtx].y = y;
                    model->meshes[i_mesh].renderVtx_PCM[i_vtx].z = w;

                    model->meshes[i_mesh].renderVtx_PCM[i_vtx+1].x = x1;
                    model->meshes[i_mesh].renderVtx_PCM[i_vtx+1].y = y1;
                    model->meshes[i_mesh].renderVtx_PCM[i_vtx+1].z = w1;

                    model->meshes[i_mesh].renderVtx_PCM[i_vtx+2].x = x2;
                    model->meshes[i_mesh].renderVtx_PCM[i_vtx+2].y = y2;
                    model->meshes[i_mesh].renderVtx_PCM[i_vtx+2].z = w2;


                    model->meshes[i_mesh].renderVtx_TPCM[i_vtx].x = x;
                    model->meshes[i_mesh].renderVtx_TPCM[i_vtx].y = y;
                    model->meshes[i_mesh].renderVtx_TPCM[i_vtx].z = w;

                    model->meshes[i_mesh].renderVtx_TPCM[i_vtx+1].x = x1;
                    model->meshes[i_mesh].renderVtx_TPCM[i_vtx+1].y = y1;
                    model->meshes[i_mesh].renderVtx_TPCM[i_vtx+1].z = w1;

                    model->meshes[i_mesh].renderVtx_TPCM[i_vtx+2].x = x2;
                    model->meshes[i_mesh].renderVtx_TPCM[i_vtx+2].y = y2;
                    model->meshes[i_mesh].renderVtx_TPCM[i_vtx+2].z = w2;


                    break;
                case 1:


                    
                    //model->meshes[i_mesh].currentClippingTrianglesCount = 3;
                    mat_load(&currCam->view);

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
                    mat_load(&currCam->view);

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

        if(model->meshes[i_mesh].shading)
        {
            vec3f_t lightDir = {0, -1, 0}; // Lumière venant du haut
            calculateVertexLighting(&model->meshes[i_mesh], lightDir);
        }
    }

    
    free(v_test);
    free(v_test1);
    free(v_test2);
    if(model->DEBUG_MODEL_VTX) printf("  [UPDATE-MODEL] <<< END }\n\n");
}



void updateModel_optimize(dct_model_t *model) {
    if(model->DEBUG_MODEL_VTX) { printf("\n\n---------------------------------------------\n---------------------------------------------\n"); printf("\n{ START >>> [UPDATE-MODEL] Model: %s\n", model->name);}
    

    dct_camera_t *currCam = getCurrentCamera(); 
    matrix_t matrix_view;
    matrix_t matrix_MVP;
    matrix_t matrix_persp;

    matrix_t matrix_modelTransform;

    mat_identity();
    mat_load(currCam->transform);
    
    mat_translate(model->position.x, model->position.y, model->position.z);
    mat_rotate_y(model->rotation.y);
    mat_rotate_x(model->rotation.x);
    mat_rotate_z(model->rotation.z);
    mat_scale(model->scale.x, model->scale.y, model->scale.z);
    mat_store(&matrix_view);


    // RESET
    mat_identity();
    // CAMERA TRANSFROM  Translate  lootAt 
    mat_load(currCam->final);

    // MODEL TRANSFORM 
    mat_translate(model->position.x, model->position.y, model->position.z);
    model->rotation.y = fmod(model->rotation.y,2*F_PI);
    model->rotation.x = fmod(model->rotation.x,2*F_PI);
    model->rotation.z = fmod(model->rotation.z, 2*F_PI);
    mat_rotate_y(model->rotation.y);
    mat_rotate_x(model->rotation.x);
    mat_rotate_z(model->rotation.z);
    mat_scale(model->scale.x, model->scale.y, model->scale.z);
    
    mat_store(&matrix_MVP);
    
    
    pvr_vertex_t *v_test  =  (pvr_vertex_t*)memalign(32, sizeof(pvr_vertex_t));
    pvr_vertex_t *v_test1 =  (pvr_vertex_t*)memalign(32, sizeof(pvr_vertex_t));
    pvr_vertex_t *v_test2 =  (pvr_vertex_t*)memalign(32, sizeof(pvr_vertex_t));
    
    int indice_current_clipping_vtx = 0;
    initClipTrianglesVtx(model);

    for (int i_mesh = 0; i_mesh < model->meshesCount; i_mesh++) {
        
        memcpy(model->meshes[i_mesh].renderVtx, 
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
            float x,y,z,w,x1,y1,z1,w1,x2,y2,z2,w2 = {0.0f};
            
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


            if(model->DEBUG_MODEL_VTX) 
            {
                printf("indice_current_clipping_vtx : %d \n",indice_current_clipping_vtx);
                printf("DEBUG UPDATE VTX %d <<< END \n",i_vtx);
            }
        }

        if(model->meshes[i_mesh].shading)
        {
            vec3f_t lightDir = {0, -1, 0}; // Lumière venant du haut
            calculateVertexLighting(&model->meshes[i_mesh], lightDir);
        }
    }

    
    free(v_test);
    free(v_test1);
    free(v_test2);
    if(model->DEBUG_MODEL_VTX) printf("  [UPDATE-MODEL] <<< END }\n\n");
}





void renderMeshENV(dct_model_t *model, int i_mesh)
{

    pvr_prim(&model->textures[model->meshes[i_mesh].currentCxtSelected].hdr ,sizeof(pvr_poly_hdr_t));

    int quadDraw = 0;
    int vertices_in_poly = 0;
    pvr_vertex_t *last_vertices[3] = {NULL, NULL, NULL};
    
    for (int i_vtx = 0; i_vtx < model->meshes[i_mesh].vtxCount; i_vtx+=3) 
    {
        pvr_vertex_t *v = &model->meshes[i_mesh].renderVtx[i_vtx];
        pvr_vertex_t *v1 = &model->meshes[i_mesh].renderVtx[i_vtx+1];
        pvr_vertex_t *v2 = &model->meshes[i_mesh].renderVtx[i_vtx+2];
        if(model->DEBUG_RENDER_VTX) 
        {
            printf("%d/%d v1 %f %f %f  v2 %f %f %f v3 %f %f %f \n", i_vtx, model->meshes[i_mesh].vtxCount, v->x,v->y,v->z,v1->x,v1->y,v1->z,v2->x,v2->y,v2->z);
        }
        pvr_prim(v, sizeof(pvr_vertex_t));
        pvr_prim(v1, sizeof(pvr_vertex_t));
        pvr_prim(v2, sizeof(pvr_vertex_t));
        
    }

}

void renderMeshSTD(dct_model_t *model, int i_mesh)
{
    
    if(model->meshes[i_mesh].currentCxtSelected >= 0)
    {
        //printf("textures cxt \n");
        pvr_prim(&model->textures[model->meshes[i_mesh].currentCxtSelected].hdr ,sizeof(pvr_poly_hdr_t));
        
    }
    if(model->meshes[i_mesh].currentCxtSelected == -1)
    {
        //printf("mesh hdr \n");
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
        if(model->DEBUG_RENDER_VTX) 
        {
            printf("%d/%d v1 %f %f %f  v2 %f %f %f v3 %f %f %f \n", i_vtx, model->meshes[i_mesh].vtxCount, v->x,v->y,v->z,v1->x,v1->y,v1->z,v2->x,v2->y,v2->z);
        }
        pvr_prim(v, sizeof(pvr_vertex_t));
        pvr_prim(v1, sizeof(pvr_vertex_t));
        pvr_prim(v2, sizeof(pvr_vertex_t));
        
    }

    
    if(model->meshes[i_mesh].currentClippingTrianglesCount>0)
    {
        
        for(int i = 0; i < model->meshes[i_mesh].currentClippingTrianglesCount; i++) {
            pvr_prim(&model->meshes[i_mesh].clippingTrianglesVtx[i], sizeof(pvr_vertex_t));
        }
    }



}


void renderMeshSTD_optimize(dct_model_t *model, int i_mesh)
{
    
    if(model->meshes[i_mesh].currentCxtSelected >= 0)
    {
        //printf("textures cxt \n");
        pvr_prim(&model->textures[model->meshes[i_mesh].currentCxtSelected].hdr ,sizeof(pvr_poly_hdr_t));
        
    }
    if(model->meshes[i_mesh].currentCxtSelected == -1)
    {
        //printf("mesh hdr \n");
        pvr_prim(&model->meshes[i_mesh].hdr, sizeof(pvr_poly_hdr_t));
    }


    int quadDraw = 0;
    int vertices_in_poly = 0;
    pvr_vertex_t *last_vertices[3] = {NULL, NULL, NULL};

    static pvr_vertex_t vertex_buffer[VERTEX_BUFFER_SIZE] __attribute__((aligned(32)));
    int buffer_count = 0;
    
    // Option 1: Rendu des triangles non clippés en batch
    if (model->meshes[i_mesh].vtxCount > 0) {
        // Aligner le nombre de vertices pour optimiser le transfert DMA
        int aligned_count = model->meshes[i_mesh].vtxCount;
        
        // Remplir et envoyer par blocs
        for (int i_vtx = 0; i_vtx < model->meshes[i_mesh].vtxCount; i_vtx += 3) {
            
            // // Vérifier si le triangle est derrière le near plane ou marqué pour clipping
            // if (model->meshes[i_mesh].renderVtx[i_vtx].flags & PVR_CMD_VERTEX ||
            //     model->meshes[i_mesh].renderVtx[i_vtx+1].flags & PVR_CMD_VERTEX ||
            //     model->meshes[i_mesh].renderVtx[i_vtx+2].flags & PVR_CMD_VERTEX_EOL) {
            //     continue;  // Triangle déjà traité par le clipping
            // }
            
            // Copier les 3 vertices dans le buffer
            memcpy(&vertex_buffer[buffer_count], &model->meshes[i_mesh].renderVtx[i_vtx], sizeof(pvr_vertex_t));
            vertex_buffer[buffer_count].flags = PVR_CMD_VERTEX;
            buffer_count++;
            
            memcpy(&vertex_buffer[buffer_count], &model->meshes[i_mesh].renderVtx[i_vtx+1], sizeof(pvr_vertex_t));
            vertex_buffer[buffer_count].flags = PVR_CMD_VERTEX;
            buffer_count++;
            
            // Dernier vertex de ce triangle
            memcpy(&vertex_buffer[buffer_count], &model->meshes[i_mesh].renderVtx[i_vtx+2], sizeof(pvr_vertex_t));
            vertex_buffer[buffer_count].flags = PVR_CMD_VERTEX_EOL;
            buffer_count++;
            
            // Quand le buffer est plein, envoyer au PVR
            if (buffer_count >= VERTEX_BUFFER_SIZE - 3) {
                pvr_prim(vertex_buffer, sizeof(pvr_vertex_t) * buffer_count);
                buffer_count = 0;
            }
        }
        
        // Envoyer les vertices restants
        if (buffer_count > 0) {
            pvr_prim(vertex_buffer, sizeof(pvr_vertex_t) * buffer_count);
            buffer_count = 0;
        }
    }

    
    if(model->meshes[i_mesh].currentClippingTrianglesCount>0)
    {
        
        for(int i = 0; i < model->meshes[i_mesh].currentClippingTrianglesCount; i++) {
            pvr_prim(&model->meshes[i_mesh].clippingTrianglesVtx[i], sizeof(pvr_vertex_t));
        }
    }



}



void renderMeshPCM(dct_model_t *model, int i_mesh)
{
    pvr_poly_mod_hdr_t hdr;
    pvr_poly_cxt_t cxt;
    pvr_poly_cxt_col_mod(&cxt, PVR_LIST_OP_POLY);
    cxt.gen.shading = PVR_SHADE_GOURAUD;
    cxt.gen.culling = PVR_CULLING_NONE;
    cxt.gen.modifier_mode = PVR_MODIFIER_NORMAL;
    cxt.fmt.modifier = PVR_MODIFIER_ENABLE;
    cxt.fmt.color = PVR_CLRFMT_ARGBPACKED;
    cxt.gen.alpha = PVR_ALPHA_ENABLE;
    cxt.gen.alpha2 = PVR_ALPHA_ENABLE;
    cxt.gen.color_clamp = PVR_CLRCLAMP_DISABLE;
    cxt.depth.comparison = PVR_DEPTHCMP_GEQUAL;
    cxt.depth.write = PVR_DEPTHWRITE_ENABLE;
    
    pvr_poly_mod_compile(&hdr, &cxt);
    pvr_prim(&hdr, sizeof(pvr_poly_hdr_t));

    int quadDraw = 0;
    int vertices_in_poly = 0;


    
    for (int i_vtx = 0; i_vtx < model->meshes[i_mesh].vtxCount; i_vtx+=3) 
    {
        pvr_vertex_pcm_t *v =  &model->meshes[i_mesh].renderVtx_PCM[i_vtx];
        pvr_vertex_pcm_t *v1 = &model->meshes[i_mesh].renderVtx_PCM[i_vtx+1];
        pvr_vertex_pcm_t *v2 = &model->meshes[i_mesh].renderVtx_PCM[i_vtx+2];

        if(model->DEBUG_RENDER_VTX) 
        {
            printf("%d/%d v1 %f %f %f  v2 %f %f %f v3 %f %f %f \n", i_vtx, model->meshes[i_mesh].vtxCount, v->x,v->y,v->z,v1->x,v1->y,v1->z,v2->x,v2->y,v2->z);
        }
        pvr_prim(v, sizeof(pvr_vertex_pcm_t));
        pvr_prim(v1, sizeof(pvr_vertex_pcm_t));
        pvr_prim(v2, sizeof(pvr_vertex_pcm_t));
        
    }


    

    // for(int i = 0; i < model->meshes[i_mesh].vtxCount; i++) {
    //     printf("PCM Vertex %d: argb0=%x argb1=%x\n", 
    //         i, 
    //         model->meshes[i_mesh].renderVtx_PCM[i].argb0,
    //         model->meshes[i_mesh].renderVtx_PCM[i].argb1);
    // }
}


void renderMeshTPCM(dct_model_t *model, int i_mesh)
{
    pvr_poly_mod_hdr_t hdr;
    pvr_poly_cxt_t cxt;
    //pvr_poly_cxt_col_mod(&cxt, PVR_LIST_OP_POLY);
    pvr_poly_cxt_col(&cxt, PVR_LIST_OP_POLY);
    cxt.gen.shading = PVR_SHADE_GOURAUD;
    cxt.gen.culling = PVR_CULLING_NONE;
    cxt.gen.modifier_mode = PVR_MODIFIER_NORMAL;
    cxt.fmt.modifier = PVR_MODIFIER_ENABLE;
    cxt.fmt.color = PVR_CLRFMT_ARGBPACKED;
    cxt.gen.alpha = PVR_ALPHA_ENABLE;
    cxt.gen.alpha2 = PVR_ALPHA_ENABLE;
    cxt.gen.color_clamp = PVR_CLRCLAMP_DISABLE;
    cxt.depth.comparison = PVR_DEPTHCMP_GEQUAL;
    cxt.depth.write = PVR_DEPTHWRITE_ENABLE;
    pvr_poly_mod_compile(&hdr, &cxt);
    //pvr_prim(&hdr, sizeof(pvr_poly_mod_hdr_t));

    
    pvr_prim(&model->textures[model->meshes[i_mesh].currentCxtSelected].hdr, sizeof(pvr_poly_mod_hdr_t));

    int quadDraw = 0;
    int vertices_in_poly = 0;


    
    for (int i_vtx = 0; i_vtx < model->meshes[i_mesh].vtxCount; i_vtx+=3) 
    {
        pvr_vertex_tpcm_t *v =  &model->meshes[i_mesh].renderVtx_TPCM[i_vtx];
        pvr_vertex_tpcm_t *v1 = &model->meshes[i_mesh].renderVtx_TPCM[i_vtx+1];
        pvr_vertex_tpcm_t *v2 = &model->meshes[i_mesh].renderVtx_TPCM[i_vtx+2];

        if(model->DEBUG_RENDER_VTX) 
        {
            printf("%d/%d v1 %f %f %f  v2 %f %f %f v3 %f %f %f \n", i_vtx, model->meshes[i_mesh].vtxCount, v->x,v->y,v->z,v1->x,v1->y,v1->z,v2->x,v2->y,v2->z);
        }
        pvr_prim(v, sizeof(pvr_vertex_tpcm_t));
        pvr_prim(v1, sizeof(pvr_vertex_tpcm_t));
        pvr_prim(v2, sizeof(pvr_vertex_tpcm_t));
        
    }


    

    // for(int i = 0; i < model->meshes[i_mesh].vtxCount; i++) {
    //     printf("PCM Vertex %d: argb0=%x argb1=%x\n", 
    //         i, 
    //         model->meshes[i_mesh].renderVtx_PCM[i].argb0,
    //         model->meshes[i_mesh].renderVtx_PCM[i].argb1);
    // }
}

void renderModel(dct_model_t *model) {
    pvr_list_begin(PVR_LIST_OP_POLY);
    
    for (int i_mesh = 0; i_mesh < model->meshesCount; i_mesh++) 
    {
        
        switch(model->meshes[i_mesh].type)
        {
            case MESH_STD_OP:
                renderMeshSTD_optimize(model,i_mesh);
                break;

            case MESH_PCM_OP:
                renderMeshPCM(model,i_mesh);
                break;

            case MESH_TPCM_OP:
                renderMeshTPCM(model,i_mesh);
                break;

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


void debug_print_polyCxt(pvr_poly_cxt_t *cxt)
{
    // List type
    printf("List Type: ");
    switch(cxt->list_type) {
        case PVR_LIST_OP_POLY:  printf("Opaque Polygon\n"); break;
        case PVR_LIST_OP_MOD:   printf("Opaque Modifier\n"); break;
        case PVR_LIST_TR_POLY:  printf("Translucent Polygon\n"); break;
        case PVR_LIST_TR_MOD:   printf("Translucent Modifier\n"); break;
        case PVR_LIST_PT_POLY:  printf("Punch-Through Polygon\n"); break;
        default:                printf("Unknown (%d)\n", cxt->list_type); break;
    }
    
    printf("\nGeneral Parameters:\n");
    printf("------------------\n");
    
    // Alpha
    printf("Alpha: %s\n", cxt->gen.alpha == PVR_ALPHA_ENABLE ? "Enabled" : "Disabled");
    
    // Shading
    printf("Shading: %s\n", cxt->gen.shading == PVR_SHADE_FLAT ? "Flat" : "Gouraud");
    
    // Fog type
    printf("Fog Type: ");
    switch(cxt->gen.fog_type) {
        case PVR_FOG_TABLE:   printf("Table\n"); break;
        case PVR_FOG_VERTEX:  printf("Vertex\n"); break;
        case PVR_FOG_DISABLE: printf("Disabled\n"); break;
        case PVR_FOG_TABLE2:  printf("Table2\n"); break;
        default:              printf("Unknown (%d)\n", cxt->gen.fog_type); break;
    }
    
    // Culling
    printf("Culling: ");
    switch(cxt->gen.culling) {
        case PVR_CULLING_NONE:  printf("None\n"); break;
        case PVR_CULLING_SMALL: printf("Small\n"); break;
        case PVR_CULLING_CCW:   printf("Counter-clockwise\n"); break;
        case PVR_CULLING_CW:    printf("Clockwise\n"); break;
        default:                printf("Unknown (%d)\n", cxt->gen.culling); break;
    }
    
    printf("Color Clamp: %s\n", cxt->gen.color_clamp == PVR_CLRCLAMP_ENABLE ? "Enabled" : "Disabled");
    
    // Clip mode
    printf("Clip Mode: ");
    switch(cxt->gen.clip_mode) {
        case PVR_USERCLIP_DISABLE: printf("Disabled\n"); break;
        case PVR_USERCLIP_INSIDE:  printf("Inside\n"); break;
        case PVR_USERCLIP_OUTSIDE: printf("Outside\n"); break;
        default:                   printf("Unknown (%d)\n", cxt->gen.clip_mode); break;
    }
    
    printf("Modifier Mode: %d\n", cxt->gen.modifier_mode);
    printf("Specular: %s\n", cxt->gen.specular == PVR_SPECULAR_ENABLE ? "Enabled" : "Disabled");
    
    printf("\nBlending Parameters:\n");
    printf("-------------------\n");
    
    // Source blend mode
    printf("Source Blend: ");
    switch(cxt->blend.src) {
        case PVR_BLEND_ZERO:         printf("Zero\n"); break;
        case PVR_BLEND_ONE:          printf("One\n"); break;
        case PVR_BLEND_DESTCOLOR:    printf("Destination Color\n"); break;
        case PVR_BLEND_INVDESTCOLOR: printf("Inverse Destination Color\n"); break;
        case PVR_BLEND_SRCALPHA:     printf("Source Alpha\n"); break;
        case PVR_BLEND_INVSRCALPHA:  printf("Inverse Source Alpha\n"); break;
        case PVR_BLEND_DESTALPHA:    printf("Destination Alpha\n"); break;
        case PVR_BLEND_INVDESTALPHA: printf("Inverse Destination Alpha\n"); break;
        default:                     printf("Unknown (%d)\n", cxt->blend.src); break;
    }
    
    // Destination blend mode
    printf("Destination Blend: ");
    switch(cxt->blend.dst) {
        case PVR_BLEND_ZERO:         printf("Zero\n"); break;
        case PVR_BLEND_ONE:          printf("One\n"); break;
        case PVR_BLEND_DESTCOLOR:    printf("Destination Color\n"); break;
        case PVR_BLEND_INVDESTCOLOR: printf("Inverse Destination Color\n"); break;
        case PVR_BLEND_SRCALPHA:     printf("Source Alpha\n"); break;
        case PVR_BLEND_INVSRCALPHA:  printf("Inverse Source Alpha\n"); break;
        case PVR_BLEND_DESTALPHA:    printf("Destination Alpha\n"); break;
        case PVR_BLEND_INVDESTALPHA: printf("Inverse Destination Alpha\n"); break;
        default:                     printf("Unknown (%d)\n", cxt->blend.dst); break;
    }
    
    printf("Source Blend Enable: %s\n", cxt->blend.src_enable == PVR_BLEND_ENABLE ? "Enabled" : "Disabled");
    printf("Destination Blend Enable: %s\n", cxt->blend.dst_enable == PVR_BLEND_ENABLE ? "Enabled" : "Disabled");
    
    printf("\nFormat Parameters:\n");
    printf("-----------------\n");
    printf("Color Format: %d\n", cxt->fmt.color);
    printf("UV Format: %s\n", cxt->fmt.uv == PVR_UVFMT_32BIT ? "32-bit" : "16-bit");
    printf("Modifier: %s\n", cxt->fmt.modifier == PVR_MODIFIER_ENABLE ? "Enabled" : "Disabled");
    
    printf("\nDepth Parameters:\n");
    printf("----------------\n");
    
    // Depth comparison mode
    printf("Depth Comparison: ");
    switch(cxt->depth.comparison) {
        case PVR_DEPTHCMP_NEVER:    printf("Never\n"); break;
        case PVR_DEPTHCMP_LESS:     printf("Less\n"); break;
        case PVR_DEPTHCMP_EQUAL:    printf("Equal\n"); break;
        case PVR_DEPTHCMP_LEQUAL:   printf("Less or Equal\n"); break;
        case PVR_DEPTHCMP_GREATER:  printf("Greater\n"); break;
        case PVR_DEPTHCMP_NOTEQUAL: printf("Not Equal\n"); break;
        case PVR_DEPTHCMP_GEQUAL:   printf("Greater or Equal\n"); break;
        case PVR_DEPTHCMP_ALWAYS:   printf("Always\n"); break;
        default:                    printf("Unknown (%d)\n", cxt->depth.comparison); break;
    }
    
    printf("Depth Write: %s\n", cxt->depth.write == PVR_DEPTHWRITE_ENABLE ? "Enabled" : "Disabled");
    
    printf("\nTexture Parameters:\n");
    printf("------------------\n");
    printf("Texture Enable: %s\n", cxt->txr.enable == PVR_TEXTURE_ENABLE ? "Enabled" : "Disabled");
    
    // Filter mode
    printf("Filter: ");
    switch(cxt->txr.filter) {
        case PVR_FILTER_NONE:       printf("None/Nearest\n"); break;
        case PVR_FILTER_BILINEAR:   printf("Bilinear\n"); break;
        case PVR_FILTER_TRILINEAR1: printf("Trilinear Pass 1\n"); break;
        case PVR_FILTER_TRILINEAR2: printf("Trilinear Pass 2\n"); break;
        default:                    printf("Unknown (%d)\n", cxt->txr.filter); break;
    }
    
    printf("Mipmap: %s\n", cxt->txr.mipmap == PVR_MIPMAP_ENABLE ? "Enabled" : "Disabled");
    printf("Mipmap Bias: %d\n", cxt->txr.mipmap_bias);
    
    // UV flip/clamp
    printf("UV Flip: ");
    switch(cxt->txr.uv_flip) {
        case PVR_UVFLIP_NONE: printf("None\n"); break;
        case PVR_UVFLIP_V:    printf("V only\n"); break;
        case PVR_UVFLIP_U:    printf("U only\n"); break;
        case PVR_UVFLIP_UV:   printf("U and V\n"); break;
        default:              printf("Unknown (%d)\n", cxt->txr.uv_flip); break;
    }
    
    printf("UV Clamp: ");
    switch(cxt->txr.uv_clamp) {
        case PVR_UVCLAMP_NONE: printf("None\n"); break;
        case PVR_UVCLAMP_V:    printf("V only\n"); break;
        case PVR_UVCLAMP_U:    printf("U only\n"); break;
        case PVR_UVCLAMP_UV:   printf("U and V\n"); break;
        default:               printf("Unknown (%d)\n", cxt->txr.uv_clamp); break;
    }
    
    printf("Texture Alpha: %s\n", cxt->txr.alpha == PVR_TXRALPHA_ENABLE ? "Enabled" : "Disabled");
    
    // Texture environment
    printf("Texture Environment: ");
    switch(cxt->txr.env) {
        case PVR_TXRENV_REPLACE:       printf("Replace\n"); break;
        case PVR_TXRENV_MODULATE:      printf("Modulate\n"); break;
        case PVR_TXRENV_DECAL:         printf("Decal\n"); break;
        case PVR_TXRENV_MODULATEALPHA: printf("Modulate Alpha\n"); break;
        default:                       printf("Unknown (%d)\n", cxt->txr.env); break;
    }
    
    printf("Texture Width: %d\n", cxt->txr.width);
    printf("Texture Height: %d\n", cxt->txr.height);
    printf("Texture Address: %p\n", cxt->txr.base);
    
    if(cxt->fmt.modifier) {
        printf("\nModifier Texture Parameters:\n");
        printf("-------------------------\n");
        printf("Texture Enable: %s\n", cxt->txr2.enable == PVR_TEXTURE_ENABLE ? "Enabled" : "Disabled");
        
        // Filter mode
        printf("Filter: ");
        switch(cxt->txr2.filter) {
            case PVR_FILTER_NONE:       printf("None/Nearest\n"); break;
            case PVR_FILTER_BILINEAR:   printf("Bilinear\n"); break;
            case PVR_FILTER_TRILINEAR1: printf("Trilinear Pass 1\n"); break;
            case PVR_FILTER_TRILINEAR2: printf("Trilinear Pass 2\n"); break;
            default:                    printf("Unknown (%d)\n", cxt->txr2.filter); break;
        }
        
        printf("Mipmap: %s\n", cxt->txr2.mipmap == PVR_MIPMAP_ENABLE ? "Enabled" : "Disabled");
        printf("Mipmap Bias: %d\n", cxt->txr2.mipmap_bias);
        
        // UV flip/clamp
        printf("UV Flip: ");
        switch(cxt->txr2.uv_flip) {
            case PVR_UVFLIP_NONE: printf("None\n"); break;
            case PVR_UVFLIP_V:    printf("V only\n"); break;
            case PVR_UVFLIP_U:    printf("U only\n"); break;
            case PVR_UVFLIP_UV:   printf("U and V\n"); break;
            default:              printf("Unknown (%d)\n", cxt->txr2.uv_flip); break;
        }
        
        printf("UV Clamp: ");
        switch(cxt->txr2.uv_clamp) {
            case PVR_UVCLAMP_NONE: printf("None\n"); break;
            case PVR_UVCLAMP_V:    printf("V only\n"); break;
            case PVR_UVCLAMP_U:    printf("U only\n"); break;
            case PVR_UVCLAMP_UV:   printf("U and V\n"); break;
            default:               printf("Unknown (%d)\n", cxt->txr2.uv_clamp); break;
        }
        
        printf("Texture Alpha: %s\n", cxt->txr2.alpha == PVR_TXRALPHA_ENABLE ? "Enabled" : "Disabled");
        
        // Texture environment
        printf("Texture Environment: ");
        switch(cxt->txr2.env) {
            case PVR_TXRENV_REPLACE:       printf("Replace\n"); break;
            case PVR_TXRENV_MODULATE:      printf("Modulate\n"); break;
            case PVR_TXRENV_DECAL:         printf("Decal\n"); break;
            case PVR_TXRENV_MODULATEALPHA: printf("Modulate Alpha\n"); break;
            default:                       printf("Unknown (%d)\n", cxt->txr2.env); break;
        }
        
        printf("Texture Width: %d\n", cxt->txr2.width);
        printf("Texture Height: %d\n", cxt->txr2.height);
        printf("Texture Address: %p\n", cxt->txr2.base);
    }
}

void debugCurrentModelActivatedCxt(dct_model_t *model)
{
    printf("Model %s \n",model->name);
    for (int i=0; i<model->meshesCount; i++)
    {
        dct_mesh_t *mesh = &model->meshes[i];
        int indiceCurrentCxt = mesh->currentCxtSelected;

        if(indiceCurrentCxt==-1)
        {
            printf("mesh %d current cxt %d \n",i,indiceCurrentCxt,model->name );
            debug_print_polyCxt(&mesh->cxt);
        }
        else
        {
            printf("mesh %d current cxt %d dans model->textures[%d]  \n",i,mesh->currentCxtSelected,mesh->currentCxtSelected);
            debug_print_polyCxt(&model->textures[indiceCurrentCxt].cxt);
        }
        
    }
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


dct_mesh_modifier_vol_t createModifierVolumeCone()
{

    printf(BLEU_START_FN "\n{ >>> START >>> [CREATE MODIFIER VOLUME CONE] \n" BLEU_INSIDE_FN);
    int vertexCount;
    uint32_t colors[3] = {0xFF0000FF, 0x00FF00FF, 0x0000FFFF}; // Rouge, Vert, Bleu
    
    pvr_vertex_t* coneVertices = createVerticesCone(1.0f,    // Rayon haut
                                          5.0f,      // Rayon bas
                                          4.0f,      // Hauteur
                                          12,        // Nombre de segments
                                          colors,    // Couleurs
                                          &vertexCount);

    pvr_modifier_vol_t *conedata = malloc(sizeof(pvr_modifier_vol_t)*vertexCount/3);
    for (int i=0; i<vertexCount/3; i++)
    {
        conedata[i].flags = PVR_CMD_VERTEX;
        conedata[i].ax = coneVertices[i].x;
        conedata[i].ay = coneVertices[i].y;
        conedata[i].az = coneVertices[i].z;
        conedata[i].bx = coneVertices[i+1].x;
        conedata[i].by = coneVertices[i+1].y;
        conedata[i].bz = coneVertices[i+1].z;
        conedata[i].cx = coneVertices[i+2].x;
        conedata[i].cy = coneVertices[i+2].y;
        conedata[i].cz = coneVertices[i+2].z;
        conedata[i].d1 = 0.0f;
        conedata[i].d2 = 0.0f;
        conedata[i].d3 = 0.0f;
        conedata[i].d4 = 0.0f;
        conedata[i].d5 = 0.0f;
        conedata[i].d6 = 0.0f;
        
        if( i==(vertexCount/3)-1 )
        {
            conedata[i].flags = PVR_CMD_VERTEX_EOL;
        }
    }
    dct_mesh_modifier_vol_t mod = {0};
    mod.triCount = vertexCount/3;
    mod.modifierTrianglesVolOriginal = (pvr_modifier_vol_t*) memalign(32,sizeof(pvr_modifier_vol_t)*mod.triCount);
    mod.modifierTrianglesVolFinal = (pvr_modifier_vol_t*) memalign(32,sizeof(pvr_modifier_vol_t)*mod.triCount);
    memcpy(mod.modifierTrianglesVolOriginal, conedata ,sizeof(pvr_modifier_vol_t)*mod.triCount);
    memcpy(mod.modifierTrianglesVolFinal, conedata ,sizeof(pvr_modifier_vol_t)*mod.triCount);
    free(coneVertices);
    free(conedata);
    return mod;
    printf(BLEU_END_FN "\n  [CREATE MODIFIER VOLUME CONE] <<< END <<< }\n" ANSI_COLOR_RESET);

}


dct_mesh_modifier_vol_t createModifierVolumeCube()
{
    printf(BLEU_START_FN "\n{ >>> START >>> [CREATE MODIFIER VOLUME CUBE] \n" BLEU_INSIDE_FN);
    float size = 1.50f;
    const pvr_modifier_vol_t moddata[] = 
    {
        // Face gauche (-X) - triangles {1,2,0} et {1,3,2}
        {PVR_CMD_VERTEX,    -1*size,   1*size,   1*size, -1*size,  -1*size,  -1*size, -1*size,  -1*size,   1*size, 0,0,0,0,0,0 },  // 1
        {PVR_CMD_VERTEX,    -1*size,   1*size,   1*size, -1*size,   1*size,  -1*size, -1*size,  -1*size,  -1*size, 0,0,0,0,0,0 },
        {PVR_CMD_VERTEX,    -1*size,   1*size,  -1*size,  1*size,  -1*size,  -1*size, -1*size,  -1*size,  -1*size, 0,0,0,0,0,0 },
        {PVR_CMD_VERTEX,    -1*size,   1*size,  -1*size,  1*size,   1*size,  -1*size,  1*size,  -1*size,  -1*size, 0,0,0,0,0,0 },
        {PVR_CMD_VERTEX,     1*size,   1*size,  -1*size,  1*size,  -1*size,   1*size,  1*size,  -1*size,  -1*size, 0,0,0,0,0,0 },
        {PVR_CMD_VERTEX,     1*size,   1*size,  -1*size,  1*size,   1*size,   1*size,  1*size,  -1*size,   1*size, 0,0,0,0,0,0 },
        {PVR_CMD_VERTEX,     1*size,   1*size,   1*size, -1*size,  -1*size,   1*size,  1*size,  -1*size,   1*size, 0,0,0,0,0,0 },
        {PVR_CMD_VERTEX,     1*size,   1*size,   1*size, -1*size,   1*size,   1*size, -1*size,  -1*size,   1*size, 0,0,0,0,0,0 },
        {PVR_CMD_VERTEX,     1*size,  -1*size,  -1*size, -1*size,  -1*size,   1*size, -1*size,  -1*size,  -1*size, 0,0,0,0,0,0 },
        {PVR_CMD_VERTEX,     1*size,  -1*size,  -1*size,  1*size,  -1*size,   1*size, -1*size,  -1*size,   1*size, 0,0,0,0,0,0 },
        {PVR_CMD_VERTEX,    -1*size,   1*size,  -1*size,  1*size,   1*size,   1*size,  1*size,   1*size,  -1*size, 0,0,0,0,0,0 },
        {PVR_CMD_VERTEX_EOL ,    -1*size,   1*size,  -1*size, -1*size,   1*size,   1*size,  1*size,   1*size,   1*size, 0,0,0,0,0,0 }

    };

    dct_mesh_modifier_vol_t mod = {0};
    mod.triCount = 12;
    mod.modifierTrianglesVolOriginal = (pvr_modifier_vol_t*) memalign(32,sizeof(pvr_modifier_vol_t)*mod.triCount);
    mod.modifierTrianglesVolFinal = (pvr_modifier_vol_t*) memalign(32,sizeof(pvr_modifier_vol_t)*mod.triCount);
    memcpy(mod.modifierTrianglesVolOriginal, moddata ,sizeof(pvr_modifier_vol_t)*mod.triCount);
    memcpy(mod.modifierTrianglesVolFinal, moddata ,sizeof(pvr_modifier_vol_t)*mod.triCount);

    mod.transform[0][0] = 1.0f;
    mod.transform[0][1] = 0.0f;
    mod.transform[0][2] = 0.0f;
    mod.transform[0][3] = 0.0f;
    mod.transform[1][0] = 0.0f;
    mod.transform[1][1] = 1.0f;
    mod.transform[1][2] = 0.0f;
    mod.transform[1][3] = 0.0f;
    mod.transform[2][0] = 0.0f;
    mod.transform[2][1] = 0.0f;
    mod.transform[2][2] = 1.0f;
    mod.transform[2][3] = 0.0f;
    mod.transform[3][0] = 0.0f;
    mod.transform[3][1] = 0.0f;
    mod.transform[3][2] = 0.0f;
    mod.transform[3][3] = 1.0f;
    printf(BLEU_END_FN "\n  [CREATE MODIFIER VOLUME CUBE] <<< END <<< }\n" ANSI_COLOR_RESET);
    return mod;
    
}



void createPrimitiveTriangle(dct_model_t *triangle, const char *name, 
                      float x1, float y1, float z1, float x2, float y2, float z2 ,float x3, float y3, float z3, 
                      uint32_t color[3]){
    //triangle->name = strdup(name);
    strcpy(triangle->name,name); 
    //sprintf(triangle->type ,"TRIANGLE");
    triangle->ident = DCT_MODEL;
    triangle->meshesCount = 1;
    triangle->meshes = malloc(sizeof(dct_mesh_t));
    triangle->meshes[0].shading = 0;
    triangle->meshes[0].vtxCount = 3;
    triangle->meshes[0].triCount = 1;
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

    triangle->meshes[0].type = MESH_STD_OP;
    triangle->meshes[0].ident = DCT_MESH;
    triangle->meshes[0].renderVtx_PCM = NULL;
    triangle->meshes[0].renderVtx_TPCM = NULL;

    
    triangle->position = (vec3f_t){0, 0, 0};
    triangle->rotation = (vec3f_t){0, 0, 0};
    triangle->scale = (vec3f_t){1, 1, 1};
    
    triangle->anim_count = 0;
    triangle->armature = NULL;
    triangle->animations = NULL;

    triangle->texturesCount = 0;
    triangle->textures = NULL;



    triangle->modifier.triCount = 0;
    triangle->modifier.modifierTrianglesVolFinal = NULL;
    triangle->modifier.modifierTrianglesVolOriginal = NULL;
    


    pvr_poly_cxt_col(&triangle->meshes[0].cxt, PVR_LIST_OP_POLY);
    triangle->meshes[0].cxt.gen.culling = PVR_CULLING_NONE;
    pvr_poly_compile(&triangle->meshes[0].hdr, &triangle->meshes[0].cxt);

    
    initModel(triangle);

}



void createPrimitiveRect(dct_model_t *rect, const char *name,
                       float x, float y, float width, float height,
                       uint32_t color){
    //rect->name = strdup(name);
    strcpy(rect->name,name); 
    //sprintf(rect->type ,"RECTANGLE");
    rect->meshesCount = 1;
    rect->ident = DCT_MODEL;
    rect->meshes = malloc(sizeof(dct_mesh_t));
    rect->meshes[0].shading = 0;
    rect->meshes[0].vtxCount = 6;
    rect->meshes[0].triCount = rect->meshes[0].vtxCount/3;
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

    rect->meshes[0].type = MESH_STD_OP;
    rect->meshes[0].ident = DCT_MESH;
    rect->meshes[0].renderVtx_PCM = NULL;
    rect->meshes[0].renderVtx_TPCM = NULL;

    rect->position = (vec3f_t){0, 0, 0};
    rect->rotation = (vec3f_t){0, 0, 0};
    rect->scale = (vec3f_t){1, 1, 1};

    rect->anim_count = 0;
    rect->armature = NULL;
    rect->animations = NULL;

    rect->texturesCount = 0;
    rect->textures = NULL;

    rect->modifier.triCount = 0;
    rect->modifier.modifierTrianglesVolFinal = NULL;
    rect->modifier.modifierTrianglesVolOriginal = NULL;
    
    
    pvr_poly_cxt_col(&rect->meshes[0].cxt, PVR_LIST_OP_POLY);
    rect->meshes[0].cxt.gen.culling = PVR_CULLING_NONE;
    pvr_poly_compile(&rect->meshes[0].hdr, &rect->meshes[0].cxt);

    initModel(rect);
}



void createPrimitiveLine(dct_model_t *line, const char *name,
                   float x1, float y1, float x2, float y2,
                   float thickness, uint32_t color){
    
    line->meshes = malloc(sizeof(dct_mesh_t));
    line->ident = DCT_MODEL;
    line->meshes[0].shading = 0;
    line->meshes[0].vtxCount = 6;
    line->meshes[0].triCount = line->meshes[0].vtxCount/3;
    line->meshes[0].sizeOfVtx = line->meshes[0].vtxCount*sizeof(pvr_vertex_t);
    line->meshes[0].originalVtx = (pvr_vertex_t*)memalign(32, line->meshes[0].vtxCount*sizeof(pvr_vertex_t));
    line->meshes[0].animatedVtx = (pvr_vertex_t*)memalign(32, line->meshes[0].vtxCount*sizeof(pvr_vertex_t));
    line->meshes[0].renderVtx   = (pvr_vertex_t*)memalign(32, line->meshes[0].vtxCount*sizeof(pvr_vertex_t));
    

    line->meshesCount = 1;
    strcpy(line->name,name); 
    //line->name = strdup(name);
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

    line->meshes[0].type = MESH_STD_OP;
    line->meshes[0].ident = DCT_MESH;
    line->meshes[0].renderVtx_PCM = NULL;
    line->meshes[0].renderVtx_TPCM = NULL;
    
    line->position = (vec3f_t){0, 0, 0};
    line->rotation = (vec3f_t){0, 0, 0};
    line->scale = (vec3f_t){1, 1, 1};

    line->anim_count = 0;
    line->armature = NULL;
    line->animations = NULL;

    line->texturesCount = 0;
    line->textures = NULL;

    line->modifier.triCount = 0;
    line->modifier.modifierTrianglesVolFinal = NULL;
    line->modifier.modifierTrianglesVolOriginal = NULL;
    
    
    pvr_poly_cxt_col(&line->meshes[0].cxt, PVR_LIST_OP_POLY);
    line->meshes[0].cxt.gen.culling = PVR_CULLING_NONE;
    pvr_poly_compile(&line->meshes[0].hdr, &line->meshes[0].cxt);

    initModel(line);
    
}



void createPrimitiveCircle(dct_model_t *circle, const char *name,
                    float posX, float posY, float radius,
                    int segments, uint32_t color){
    
    circle->ident = DCT_MODEL;
    circle->meshes = malloc(sizeof(dct_mesh_t));
    circle->meshesCount = 1;
    //circle->name = strdup(name);
    strcpy(circle->name,name); 
    // Allouer la mémoire pour les vertices (3 vertices par triangle)
    float centerX,centerY = 0;
    circle->meshes[0].shading = 0;
    circle->meshes[0].vtxCount = 3*segments;
    circle->meshes[0].triCount = circle->meshes[0].vtxCount/3;
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

    circle->meshes[0].type = MESH_STD_OP;
    circle->meshes[0].ident = DCT_MESH;
    circle->meshes[0].renderVtx_PCM = NULL;
    circle->meshes[0].renderVtx_TPCM = NULL;

    circle->position = (vec3f_t){posX, posY, 0};
    circle->rotation = (vec3f_t){0, 0, 0};
    circle->scale = (vec3f_t){1, 1, 1};

    circle->anim_count = 0;
    circle->armature = NULL;
    circle->animations = NULL;

    circle->texturesCount = 0;
    circle->textures = NULL;

    circle->modifier.triCount = 0;
    circle->modifier.modifierTrianglesVolFinal = NULL;
    circle->modifier.modifierTrianglesVolOriginal = NULL;
    
    
    pvr_poly_cxt_col(&circle->meshes[0].cxt, PVR_LIST_OP_POLY);
    circle->meshes[0].cxt.gen.culling = PVR_CULLING_NONE;
    pvr_poly_compile(&circle->meshes[0].hdr, &circle->meshes[0].cxt);

    initModel(circle);
    
}



void createPrimitivePolygon(dct_model_t *polygon, const char *name, 
                     float *points, int numPoints, uint32_t color){
    
    polygon->ident = DCT_MODEL;
    polygon->meshes = malloc(sizeof(dct_mesh_t));
    polygon->meshesCount = 1;
    //polygon->name = strdup(name);
    strcpy(polygon->name,name); 
    // Pour un polygone de N points, nous avons besoin de N-2 triangles
    int totalVertices = (numPoints - 2) * 3;
    polygon->meshes[0].vtxCount = totalVertices;
    polygon->meshes[0].shading = 0;
    polygon->meshes[0].triCount = polygon->meshes[0].vtxCount/3;
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

    polygon->meshes[0].type = MESH_STD_OP;
    polygon->meshes[0].ident = DCT_MESH;
    polygon->meshes[0].renderVtx_PCM = NULL;
    polygon->meshes[0].renderVtx_TPCM = NULL;
    
    polygon->position = (vec3f_t){0.0f, 0.0f, 0.0f};
    polygon->rotation = (vec3f_t){0.0f, 0.0f, 0.0f};
    polygon->scale = (vec3f_t){1.0f, 1.0f, 1.0f};

    polygon->anim_count = 0;
    polygon->armature = NULL;
    polygon->animations = NULL;

    polygon->texturesCount = 0;
    polygon->textures = NULL;

    polygon->modifier.triCount = 0;
    polygon->modifier.modifierTrianglesVolFinal = NULL;
    polygon->modifier.modifierTrianglesVolOriginal = NULL;
    
    
    pvr_poly_cxt_col(&polygon->meshes[0].cxt, PVR_LIST_OP_POLY);
    polygon->meshes[0].cxt.gen.culling = PVR_CULLING_NONE;
    pvr_poly_compile(&polygon->meshes[0].hdr, &polygon->meshes[0].cxt);

    initModel(polygon);
    
}





void createPrimitivePath(dct_model_t *path, const char *name, 
                  float *points, int numPoints, 
                  float thickness, uint32_t color){
    
    //path->name = strdup(name);
    path->ident = DCT_MODEL;
    strcpy(path->name,name); 
    path->meshesCount = 1;
    path->meshes = malloc(sizeof(dct_mesh_t));
    // Pour chaque segment:
    // - 6 vertices pour le segment lui-même (2 triangles)
    // - 6 vertices pour la jonction (2 triangles, sauf dernier point)
    int verticesPerSegment = 6;
    int verticesPerJunction = 6;
    path->meshes[0].vtxCount = ( (numPoints - 1) * verticesPerSegment) + ((numPoints - 2) * verticesPerJunction);
    path->meshes[0].triCount = path->meshes[0].vtxCount /3;
    path->meshes[0].shading = 0;
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

    path->meshes[0].type = MESH_STD_OP;
    path->meshes[0].ident = DCT_MESH;
    path->meshes[0].renderVtx_PCM = NULL;
    path->meshes[0].renderVtx_TPCM = NULL;
    
    path->position = (vec3f_t){0.0f, 0.0f, 0.0f};
    path->rotation = (vec3f_t){0.0f, 0.0f, 0.0f};
    path->scale = (vec3f_t){1.0f, 1.0f, 1.0f};

    path->anim_count = 0;
    path->armature = NULL;
    path->animations = NULL;

    path->texturesCount = 0;
    path->textures = NULL;

    path->modifier.triCount = 0;
    path->modifier.modifierTrianglesVolFinal = NULL;
    path->modifier.modifierTrianglesVolOriginal = NULL;
    
    pvr_poly_cxt_col(&path->meshes[0].cxt, PVR_LIST_OP_POLY);
    path->meshes[0].cxt.gen.culling = PVR_CULLING_NONE;
    pvr_poly_compile(&path->meshes[0].hdr, &path->meshes[0].cxt);

    initModel(path);

}


// Fonction pour créer un cône
pvr_vertex_t* createVerticesCone(float radiusTop, float radiusBottom, float height, int segments, uint32_t* color, int* vertexCount) {
    // Calcul du nombre total de vertices nécessaires
    // (segments * 6) pour le corps (2 triangles par segment)
    // (segments * 3) pour le haut
    // (segments * 3) pour le bas
    *vertexCount = segments * 12;
    
    pvr_vertex_t* vertices = malloc(sizeof(pvr_vertex_t) * (*vertexCount));
    int currentVertex = 0;
    float angleStep = 2.0f * F_PI / segments;
    
    // Générer les vertices pour la surface latérale
    for(int i = 0; i < segments; i++) {
        float angle1 = i * angleStep;
        float angle2 = ((i + 1) % segments) * angleStep;
        
        // Points du bas
        float x1Bottom = radiusBottom * cosf(angle1);
        float z1Bottom = radiusBottom * sinf(angle1);
        float x2Bottom = radiusBottom * cosf(angle2);
        float z2Bottom = radiusBottom * sinf(angle2);
        
        // Points du haut
        float x1Top = radiusTop * cosf(angle1);
        float z1Top = radiusTop * sinf(angle1);
        float x2Top = radiusTop * cosf(angle2);
        float z2Top = radiusTop * sinf(angle2);
        
        // Premier triangle
        vertices[currentVertex++] = (pvr_vertex_t){PVR_CMD_VERTEX,    x1Bottom, -height/2, z1Bottom, 0.0f, 0.0f, color[0], 0.0f};
        vertices[currentVertex++] = (pvr_vertex_t){PVR_CMD_VERTEX,    x2Top,    height/2, z2Top,    1.0f, 0.0f, color[0], 0.0f};
        vertices[currentVertex++] = (pvr_vertex_t){PVR_CMD_VERTEX_EOL,x1Top,    height/2, z1Top,    0.0f, 1.0f, color[0], 0.0f};
        
        // Second triangle
        vertices[currentVertex++] = (pvr_vertex_t){PVR_CMD_VERTEX,    x1Bottom, -height/2, z1Bottom, 0.0f, 0.0f, color[0], 0.0f};
        vertices[currentVertex++] = (pvr_vertex_t){PVR_CMD_VERTEX,    x2Bottom, -height/2, z2Bottom, 1.0f, 0.0f, color[0], 0.0f};
        vertices[currentVertex++] = (pvr_vertex_t){PVR_CMD_VERTEX_EOL,x2Top,    height/2, z2Top,    1.0f, 1.0f, color[0], 0.0f};
        
        // Triangle pour le bas
        vertices[currentVertex++] = (pvr_vertex_t){PVR_CMD_VERTEX,    0.0f,     -height/2, 0.0f,    0.5f, 0.5f, color[1], 0.0f};
        vertices[currentVertex++] = (pvr_vertex_t){PVR_CMD_VERTEX,    x2Bottom, -height/2, z2Bottom, 1.0f, 0.0f, color[1], 0.0f};
        vertices[currentVertex++] = (pvr_vertex_t){PVR_CMD_VERTEX_EOL,x1Bottom, -height/2, z1Bottom, 0.0f, 0.0f, color[1], 0.0f};
        
        // Triangle pour le haut
        vertices[currentVertex++] = (pvr_vertex_t){PVR_CMD_VERTEX,    0.0f,     height/2, 0.0f,     0.5f, 0.5f, color[2], 0.0f};
        vertices[currentVertex++] = (pvr_vertex_t){PVR_CMD_VERTEX,    x1Top,    height/2, z1Top,    0.0f, 0.0f, color[2], 0.0f};
        vertices[currentVertex++] = (pvr_vertex_t){PVR_CMD_VERTEX_EOL,x2Top,    height/2, z2Top,    1.0f, 0.0f, color[2], 0.0f};
    }
    
    return vertices;
}


void createConeMesh(dct_model_t *cone, const char *name, float radiusTop, float radiusBottom, float height, int segments, uint32_t* color) {
    
    printf(BLEU_START_FN "\n{ >>> START >>> [CREATE CONE] \n" BLEU_INSIDE_FN);
    int vertexCount;
    uint32_t colors[3] = {0xFF0000FF, 0x00FF00FF, 0x0000FFFF}; // Rouge, Vert, Bleu
    
    pvr_vertex_t* coneVertices = createCone(1.0f,    // Rayon haut
                                          5.0f,      // Rayon bas
                                          4.0f,      // Hauteur
                                          12,        // Nombre de segments
                                          colors,    // Couleurs
                                          &vertexCount);
    
    //strcpy(cone->name,name, sizeof(cone->name) ); 
    cone->ident = DCT_MODEL;
    strcpy(cone->name,name); 
    cone->meshesCount = 1;
    cone->meshes[0].shading = 0;
    cone->meshes = (dct_mesh_t*)malloc(sizeof(dct_mesh_t));
    cone->meshes[0].vtxCount = sizeof(coneVertices)/sizeof(pvr_vertex_t);
    cone->meshes[0].triCount = cone->meshes[0].vtxCount / 3;
    cone->meshes[0].sizeOfVtx = cone->meshes[0].vtxCount*sizeof(pvr_vertex_t);
    cone->meshes[0].originalVtx = (pvr_vertex_t*)memalign(32, cone->meshes[0].vtxCount*sizeof(pvr_vertex_t));
    cone->meshes[0].animatedVtx = (pvr_vertex_t*)memalign(32, cone->meshes[0].vtxCount*sizeof(pvr_vertex_t));
    cone->meshes[0].renderVtx = (pvr_vertex_t*)memalign(32, cone->meshes[0].vtxCount*sizeof(pvr_vertex_t));

    memcpy(cone->meshes[0].originalVtx, coneVertices , cone->meshes[0].sizeOfVtx);
    memcpy(cone->meshes[0].animatedVtx, coneVertices , cone->meshes[0].sizeOfVtx);
    memcpy(cone->meshes[0].renderVtx, coneVertices , cone->meshes[0].sizeOfVtx);
    
    cone->meshes[0].type = MESH_STD_OP;
    cone->meshes[0].ident = DCT_MESH;
    cone->meshes[0].renderVtx_PCM = NULL;
    cone->meshes[0].renderVtx_TPCM = NULL;
    cone->meshes[0].shading = false;

    cone->anim_count = 0;
    cone->armature = NULL;
    cone->animations = NULL;

    
    cone->modifier.triCount = 0;
    cone->modifier.modifierTrianglesVolFinal = NULL;
    cone->modifier.modifierTrianglesVolOriginal = NULL;

    cone->texturesCount = 1;
    cone->textures = malloc(sizeof(dct_texture_t)*cone->texturesCount);
    cone->textures[0].addrA = NULL;
    cone->textures[0].addrB = NULL;
    cone->textures[0].type  = NONE;
   
    initModel(cone);
    free(coneVertices);
    printf(BLEU_END_FN "\n  [CREATE CONE] <<< END <<< }\n" ANSI_COLOR_RESET);

}

void createPlane(dct_model_t *plane, const char *name, float size, uint32_t color)
{
    printf(BLEU_START_FN "\n{ >>> START >>> [CREATE PLANE] \n" BLEU_INSIDE_FN);

    const pvr_vertex_t rawdata[] = {
    // Face bas (-Y) - triangles {6,0,2} et {6,4,0}
   {PVR_CMD_VERTEX,     1*size,  0.0f,  -1*size,  0.0f, 0.0f, color, 0.0f},  // 6
   {PVR_CMD_VERTEX,    -1*size,  0.0f,   1*size,  1.0f, 1.0f, color, 0.0f},  // 0
   {PVR_CMD_VERTEX_EOL,-1*size,  0.0f,  -1*size,  0.0f, 1.0f, color, 0.0f},  // 2

   {PVR_CMD_VERTEX,     1*size,  0.0f,  -1*size,  0.0f, 0.0f, color, 0.0f},  // 6
   {PVR_CMD_VERTEX,     1*size,  0.0f,   1*size,  1.0f, 0.0f, color, 0.0f},  // 4
   {PVR_CMD_VERTEX_EOL,-1*size,  0.0f,   1*size,  1.0f, 1.0f, color, 0.0f}  // 0
    };
    plane->ident = DCT_MODEL;
    strcpy(plane->name,name); 
    //plane->name = strdup(name);
    plane->meshesCount = 1;
    plane->meshes[0].shading = 0;
    plane->meshes = (dct_mesh_t*)malloc(sizeof(dct_mesh_t));
    plane->meshes[0].vtxCount = sizeof(rawdata)/sizeof(pvr_vertex_t);
    plane->meshes[0].triCount = plane->meshes[0].vtxCount /3;
    plane->meshes[0].sizeOfVtx = plane->meshes[0].vtxCount*sizeof(pvr_vertex_t);
    plane->meshes[0].originalVtx = (pvr_vertex_t*)memalign(32, plane->meshes[0].vtxCount*sizeof(pvr_vertex_t));
    plane->meshes[0].animatedVtx = (pvr_vertex_t*)memalign(32, plane->meshes[0].vtxCount*sizeof(pvr_vertex_t));
    plane->meshes[0].renderVtx = (pvr_vertex_t*)memalign(32, plane->meshes[0].vtxCount*sizeof(pvr_vertex_t));

    
    memcpy(plane->meshes[0].originalVtx, rawdata , plane->meshes[0].sizeOfVtx);
    memcpy(plane->meshes[0].animatedVtx, rawdata , plane->meshes[0].sizeOfVtx);
    memcpy(plane->meshes[0].renderVtx, rawdata , plane->meshes[0].sizeOfVtx);
    
    plane->meshes[0].type = MESH_STD_OP;
    plane->meshes[0].ident = DCT_MESH;
    plane->meshes[0].renderVtx_PCM = NULL;
    plane->meshes[0].renderVtx_TPCM = NULL;
    plane->meshes[0].shading = false;

    // Allouer et initialiser les normales
    plane->meshes[0].vertexNormals = (vec3f_t*)malloc(plane->meshes[0].vtxCount * sizeof(vec3f_t));
    for(int i = 0; i < plane->meshes[0].vtxCount; i++) 
    {
        plane->meshes[0].vertexNormals[i] = (vec3f_t){-1.0f, 0.0f, 0.0f};
    }
    
    plane->anim_count = 0;
    plane->armature = NULL;
    plane->animations = NULL;

    
    plane->modifier.triCount = 0;
    plane->modifier.modifierTrianglesVolFinal = NULL;
    plane->modifier.modifierTrianglesVolOriginal = NULL;

    plane->texturesCount = 1;
    plane->textures = malloc(sizeof(dct_texture_t)*plane->texturesCount);
    plane->textures[0].addrA = NULL;
    plane->textures[0].addrB = NULL;
    plane->textures[0].type  = NONE;
   
    initModel(plane);

    printf(BLEU_END_FN "\n  [CREATE PLANE] <<< END <<< }\n" ANSI_COLOR_RESET);
}


void createCube(dct_model_t *cube, const char *name, 
                  float size, uint32_t color[6]){

    printf(BLEU_START_FN "\n{ >>> START >>> [CREATE CUBE] \n" BLEU_INSIDE_FN);


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
   {PVR_CMD_VERTEX,    -1*size,   1*size,  -1*size,  0.0f, 0.0f, color[1], 0.0f},  // 3
   {PVR_CMD_VERTEX,     1*size,  -1*size,  -1*size,  1.0f, 0.0f, color[1], 0.0f},  // 6
   {PVR_CMD_VERTEX_EOL,-1*size,  -1*size,  -1*size,  0.0f, 1.0f, color[1], 0.0f},  // 2

   {PVR_CMD_VERTEX,    -1*size,   1*size,  -1*size,  0.0f, 0.0f, color[1], 0.0f},  // 3
   {PVR_CMD_VERTEX,     1*size,   1*size,  -1*size,  1.0f, 0.0f, color[1], 0.0f},  // 7
   {PVR_CMD_VERTEX_EOL, 1*size,  -1*size,  -1*size,  1.0f, 1.0f, color[1], 0.0f},  // 6

   // Face droite (+X) - triangles {7,4,6} et {7,5,4}
   {PVR_CMD_VERTEX,     1*size,   1*size,  -1*size,  0.0f, 0.0f, color[2], 0.0f},  // 7
   {PVR_CMD_VERTEX,     1*size,  -1*size,   1*size,  1.0f, 0.0f, color[2], 0.0f},  // 4
   {PVR_CMD_VERTEX_EOL, 1*size,  -1*size,  -1*size,  0.0f, 1.0f, color[2], 0.0f},  // 6

   {PVR_CMD_VERTEX,     1*size,   1*size,  -1*size,  0.0f, 0.0f, color[2], 0.0f},  // 7
   {PVR_CMD_VERTEX,     1*size,   1*size,   1*size,  1.0f, 0.0f, color[2], 0.0f},  // 5
   {PVR_CMD_VERTEX_EOL, 1*size,  -1*size,   1*size,  1.0f, 1.0f, color[2], 0.0f},  // 4

   // Face avant (+Z) - triangles {5,0,4} et {5,1,0}
   {PVR_CMD_VERTEX,     1*size,   1*size,   1*size,  0.0f, 0.0f, color[3], 0.0f},  // 5
   {PVR_CMD_VERTEX,    -1*size,  -1*size,   1*size,  1.0f, 0.0f, color[3], 0.0f},  // 0
   {PVR_CMD_VERTEX_EOL, 1*size,  -1*size,   1*size,  0.0f, 1.0f, color[3], 0.0f},  // 4

   {PVR_CMD_VERTEX,     1*size,   1*size,   1*size,  0.0f, 0.0f, color[3], 0.0f},  // 5
   {PVR_CMD_VERTEX,    -1*size,   1*size,   1*size,  1.0f, 0.0f, color[3], 0.0f},  // 1
   {PVR_CMD_VERTEX_EOL,-1*size,  -1*size,   1*size,  1.0f, 1.0f, color[3], 0.0f},  // 0

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



    // const pvr_vertex_t rawdata[] = {
    //         // Face avant (+Z) 
    //         {PVR_CMD_VERTEX,     1*size,  -1*size,   1*size,  0.0f, 0.0f, color[3], 0.0f},  // 4
    //         {PVR_CMD_VERTEX,    -1*size,  -1*size,   1*size,  1.0f, 0.0f, color[3], 0.0f},  // 0
    //         {PVR_CMD_VERTEX_EOL, 1*size,   1*size,   1*size,  0.0f, 1.0f, color[3], 0.0f},  // 5

    //         {PVR_CMD_VERTEX,    -1*size,  -1*size,   1*size,  0.0f, 0.0f, color[3], 0.0f},  // 0
    //         {PVR_CMD_VERTEX,    -1*size,   1*size,   1*size,  1.0f, 0.0f, color[3], 0.0f},  // 1
    //         {PVR_CMD_VERTEX_EOL, 1*size,   1*size,   1*size,  1.0f, 1.0f, color[3], 0.0f},  // 5

    //         // Face arrière (-Z)
    //         {PVR_CMD_VERTEX,     1*size,  -1*size,  -1*size,  0.0f, 0.0f, color[1], 0.0f},  // 6
    //         {PVR_CMD_VERTEX,    -1*size,  -1*size,  -1*size,  1.0f, 0.0f, color[1], 0.0f},  // 2
    //         {PVR_CMD_VERTEX_EOL, 1*size,   1*size,  -1*size,  0.0f, 1.0f, color[1], 0.0f},  // 7

    //         {PVR_CMD_VERTEX,    -1*size,  -1*size,  -1*size,  0.0f, 0.0f, color[1], 0.0f},  // 2
    //         {PVR_CMD_VERTEX,    -1*size,   1*size,  -1*size,  1.0f, 0.0f, color[1], 0.0f},  // 3
    //         {PVR_CMD_VERTEX_EOL, 1*size,   1*size,  -1*size,  1.0f, 1.0f, color[1], 0.0f},  // 7

    //         // Face droite (+X)
    //         {PVR_CMD_VERTEX,     1*size,  -1*size,  -1*size,  0.0f, 0.0f, color[2], 0.0f},  // 6
    //         {PVR_CMD_VERTEX,     1*size,  -1*size,   1*size,  1.0f, 0.0f, color[2], 0.0f},  // 4
    //         {PVR_CMD_VERTEX_EOL, 1*size,   1*size,  -1*size,  0.0f, 1.0f, color[2], 0.0f},  // 7

    //         {PVR_CMD_VERTEX,     1*size,  -1*size,   1*size,  0.0f, 0.0f, color[2], 0.0f},  // 4
    //         {PVR_CMD_VERTEX,     1*size,   1*size,   1*size,  1.0f, 0.0f, color[2], 0.0f},  // 5
    //         {PVR_CMD_VERTEX_EOL, 1*size,   1*size,  -1*size,  1.0f, 1.0f, color[2], 0.0f},  // 7

    //         // Face gauche (-X)
    //         {PVR_CMD_VERTEX,    -1*size,  -1*size,   1*size,  0.0f, 0.0f, color[0], 0.0f},  // 0
    //         {PVR_CMD_VERTEX,    -1*size,  -1*size,  -1*size,  1.0f, 0.0f, color[0], 0.0f},  // 2
    //         {PVR_CMD_VERTEX_EOL,-1*size,   1*size,   1*size,  0.0f, 1.0f, color[0], 0.0f},  // 1

    //         {PVR_CMD_VERTEX,    -1*size,  -1*size,  -1*size,  0.0f, 0.0f, color[0], 0.0f},  // 2
    //         {PVR_CMD_VERTEX,    -1*size,   1*size,  -1*size,  1.0f, 0.0f, color[0], 0.0f},  // 3
    //         {PVR_CMD_VERTEX_EOL,-1*size,   1*size,   1*size,  1.0f, 1.0f, color[0], 0.0f},  // 1

    //         // Face haut (Y+)
    //         {PVR_CMD_VERTEX,    -1*size,   1*size,   1*size,  0.0f, 0.0f, color[5], 0.0f},  // 1
    //         {PVR_CMD_VERTEX,    -1*size,   1*size,  -1*size,  1.0f, 0.0f, color[5], 0.0f},  // 3
    //         {PVR_CMD_VERTEX_EOL, 1*size,   1*size,   1*size,  0.0f, 1.0f, color[5], 0.0f},  // 5

    //         {PVR_CMD_VERTEX,    -1*size,   1*size,  -1*size,  0.0f, 0.0f, color[5], 0.0f},  // 3
    //         {PVR_CMD_VERTEX,     1*size,   1*size,  -1*size,  1.0f, 0.0f, color[5], 0.0f},  // 7
    //         {PVR_CMD_VERTEX_EOL, 1*size,   1*size,   1*size,  1.0f, 1.0f, color[5], 0.0f},  // 5

    //         // Face bas (Y-)
    //         {PVR_CMD_VERTEX,    -1*size,  -1*size,  -1*size,  0.0f, 0.0f, color[4], 0.0f},  // 2
    //         {PVR_CMD_VERTEX,    -1*size,  -1*size,   1*size,  1.0f, 0.0f, color[4], 0.0f},  // 0
    //         {PVR_CMD_VERTEX_EOL, 1*size,  -1*size,  -1*size,  0.0f, 1.0f, color[4], 0.0f},  // 6

    //         {PVR_CMD_VERTEX,    -1*size,  -1*size,   1*size,  0.0f, 0.0f, color[4], 0.0f},  // 0
    //         {PVR_CMD_VERTEX,     1*size,  -1*size,   1*size,  1.0f, 0.0f, color[4], 0.0f},  // 4
    //         {PVR_CMD_VERTEX_EOL, 1*size,  -1*size,  -1*size,  1.0f, 1.0f, color[4], 0.0f}   // 6
    //     };
    
    
    
    cube->ident = DCT_MODEL;
    strcpy(cube->name,name); 
    //cube->name = strdup(name);
    cube->meshesCount = 1;
    cube->meshes = (dct_mesh_t*)malloc(sizeof(dct_mesh_t));
    cube->meshes[0].shading = 0;
    char meshName[32];
    snprintf( meshName,32,"mesh0 %s", cube->name );

    strcpy(cube->meshes[0].name,meshName);
    cube->meshes[0].vtxCount = sizeof(rawdata)/sizeof(pvr_vertex_t);
    cube->meshes[0].triCount = cube->meshes[0].vtxCount / 3;
    cube->meshes[0].sizeOfVtx = cube->meshes[0].vtxCount*sizeof(pvr_vertex_t);
    cube->meshes[0].originalVtx = (pvr_vertex_t*)memalign(32, cube->meshes[0].vtxCount*sizeof(pvr_vertex_t));
    cube->meshes[0].animatedVtx = (pvr_vertex_t*)memalign(32, cube->meshes[0].vtxCount*sizeof(pvr_vertex_t));
    cube->meshes[0].renderVtx = (pvr_vertex_t*)memalign(32, cube->meshes[0].vtxCount*sizeof(pvr_vertex_t));

    
    memcpy(cube->meshes[0].originalVtx, rawdata , cube->meshes[0].sizeOfVtx);
    memcpy(cube->meshes[0].animatedVtx, rawdata , cube->meshes[0].sizeOfVtx);
    memcpy(cube->meshes[0].renderVtx, rawdata , cube->meshes[0].sizeOfVtx);
    
    cube->meshes[0].type = MESH_STD_OP;
    cube->meshes[0].ident = DCT_MESH;
    cube->meshes[0].renderVtx_PCM = NULL;
    cube->meshes[0].renderVtx_TPCM = NULL;


    // Allouer et initialiser les normales
    cube->meshes[0].vertexNormals = (vec3f_t*)malloc(cube->meshes[0].vtxCount * sizeof(vec3f_t));

    // Pour les 36 vertices (6 faces * 2 triangles * 3 vertices)
    for(int i = 0; i < cube->meshes[0].vtxCount; i++) {
        int faceIndex = i / 6;  // 6 vertices par face
        //printf("Face Index %d \n",faceIndex);
        switch(faceIndex) {
            case 0: // Face gauche (-X)
                cube->meshes[0].vertexNormals[i] = (vec3f_t){-1.0f, 0.0f, 0.0f};
                break;
            case 1: // Face arrière (-Z)
                cube->meshes[0].vertexNormals[i] = (vec3f_t){0.0f, 0.0f, -1.0f};
                break;
            case 2: // Face droite (+X)
                cube->meshes[0].vertexNormals[i] = (vec3f_t){1.0f, 0.0f, 0.0f};
                break;
            case 3: // Face avant (+Z)
                cube->meshes[0].vertexNormals[i] = (vec3f_t){0.0f, 0.0f, 1.0f};
                break;
            case 4: // Face bas (-Y)
                cube->meshes[0].vertexNormals[i] = (vec3f_t){0.0f, -1.0f, 0.0f};
                break;
            case 5: // Face haut (+Y)
                cube->meshes[0].vertexNormals[i] = (vec3f_t){0.0f, 1.0f, 0.0f};
                break;
        }
    }

    // for(int i = 0; i < cube->meshes[0].vtxCount; i++) 
    // {
    //     printf("vertex Normals %d %f %f %f \n",i,cube->meshes[0].vertexNormals[i].x,cube->meshes[0].vertexNormals[i].y,cube->meshes[0].vertexNormals[i].z);
    // }
    cube->meshes[0].smoothingWeights = NULL;

    cube->anim_count = 0;
    cube->armature = NULL;
    cube->animations = NULL;

    cube->texturesCount = 0;
    cube->textures = NULL;

    cube->modifier.triCount = 0;
    cube->modifier.modifierTrianglesVolFinal = NULL;
    cube->modifier.modifierTrianglesVolOriginal = NULL;
    
   
    initModel(cube);
    //displayModelData(&cube->model);

    printf(BLEU_END_FN"\n  [CREATE CUBE] <<< END <<< }\n" ANSI_COLOR_RESET);



}



void createPentahedre(dct_model_t *penta, const char *name, 
                  float size, uint32_t colors[5]){
    printf(BLEU_START_FN"{ >>> START >>> [CREATE PENTAHEDRE] \n"BLEU_INSIDE_FN);


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
    



    //penta->name = strdup(name);
    strcpy(penta->name,name);
    // penta->name = (char)malloc(sizeof(name));
    // penta->name = strcpy(name,sizeof(name));
    // penta->name[sizeof(name)-1] = '\0';
    penta->ident = DCT_MODEL;
    penta->meshesCount = 1;
    penta->meshes = (dct_mesh_t*)malloc(sizeof(dct_mesh_t));
    penta->meshes[0].indice = 0;
    char meshName[20];
    sprintf(meshName, "mesh-%d %s", penta->meshes[0].indice, name);
    //penta->meshes[0].name = (char)malloc(sizeof(meshName));
    //penta->meshes[0].name = strdup(meshName);
    strcpy(penta->meshes[0].name,meshName);
    penta->meshes[0].vtxCount = 18;
    penta->meshes[0].shading = 0;
    penta->meshes[0].triCount = penta->meshes[0].vtxCount / 3;
    penta->meshes[0].sizeOfVtx = penta->meshes[0].vtxCount*sizeof(pvr_vertex_t);
    penta->meshes[0].originalVtx = (pvr_vertex_t*)memalign(32, penta->meshes[0].vtxCount*sizeof(pvr_vertex_t));
    penta->meshes[0].animatedVtx = (pvr_vertex_t*)memalign(32, penta->meshes[0].vtxCount*sizeof(pvr_vertex_t));
    penta->meshes[0].renderVtx = (pvr_vertex_t*)memalign(32, penta->meshes[0].vtxCount*sizeof(pvr_vertex_t));

    memcpy(penta->meshes[0].originalVtx, rawdata , penta->meshes[0].sizeOfVtx);
    memcpy(penta->meshes[0].animatedVtx, rawdata , penta->meshes[0].sizeOfVtx);
    memcpy(penta->meshes[0].renderVtx, rawdata , penta->meshes[0].sizeOfVtx);

    penta->meshes[0].type = MESH_STD_OP;
    penta->meshes[0].ident = DCT_MESH;
    penta->meshes[0].renderVtx_PCM = NULL;
    penta->meshes[0].renderVtx_TPCM = NULL;

    penta->armature = NULL;
    penta->animations = NULL;
    penta->anim_count = 0;

    penta->texturesCount = 0;
    penta->textures = NULL;

    penta->modifier.triCount = 0;
    penta->modifier.modifierTrianglesVolFinal = NULL;
    penta->modifier.modifierTrianglesVolOriginal = NULL;
   
    initModel(penta);
    //displayModelData(&penta->model);

    printf(BLEU_END_FN"\n[CREATE PENTAHEDRE] <<< END <<< }\n"ANSI_COLOR_RESET);


}


void createSphere(dct_model_t *sphere, const char *name ,float size, uint32_t color)
{

    printf(BLEU_START_FN "\n{ >>> START >>> [CREATE SPHERE] \n" BLEU_INSIDE_FN);


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


    sphere->ident = DCT_MODEL;
    strcpy(sphere->name,name); 
    //sphere->name = strdup(name);
    sphere->meshesCount = 1;
    sphere->meshes = (dct_mesh_t*)malloc(sizeof(dct_mesh_t));
    sphere->meshes[0].vtxCount = sizeof(sphere_data)/sizeof(pvr_vertex_t);
    sphere->meshes[0].triCount = sphere->meshes[0].vtxCount / 3;
    sphere->meshes[0].shading = 0;
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
    
    sphere->meshes[0].ident = DCT_MESH;
    sphere->meshes[0].type = MESH_STD_OP;
    sphere->meshes[0].renderVtx_PCM = NULL;
    sphere->meshes[0].renderVtx_TPCM = NULL;

    sphere->anim_count = 0;
    sphere->armature = NULL;
    sphere->animations = NULL;

    sphere->texturesCount = 0;
    sphere->textures = NULL;

    sphere->modifier.triCount = 0;
    sphere->modifier.modifierTrianglesVolFinal = NULL;
    sphere->modifier.modifierTrianglesVolOriginal = NULL;
    

    initModel(sphere);
    //displayModelData(&sphere->model);

    printf(BLEU_END_FN "\n  [CREATE SPHERE] <<< END <<< }\n" ANSI_COLOR_RESET);

}


