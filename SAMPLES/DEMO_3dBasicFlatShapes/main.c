#include <kos.h>
#include <malloc.h>
#include <dc/pvr.h>
#include <dc/sound/sound.h>
#include <dc/sound/aica_comm.h>
#include "dct_model.h"  // Notre fichier avec toutes les primitives
//#include "dct_camera.h"

#include "dct_guiText.h"
#include "dct_input.h"


extern uint8 romdisk[];
extern uint8 romdisk_end[];


// Déclaration des primitives
dct_model_t triangle;
dct_model_t rectangle;
dct_model_t line;
dct_model_t circle;
dct_model_t polygon;

void init_scene() {
    // Triangle rouge
    uint32_t c3[3] = {0xFFFF0000, 0xFF00FF00,0xFF0000FF};
    createPrimitiveTriangle(&triangle, "triangle1", 
        -10.0f, 10.0f, 0.1f,  // Point 1
        10.0f, 10.0f,  0.1f, // Point 2
        0.0f, -10.0f,  0.1f, // Point 3
        c3);      // Rouge
    

    // Rectangle vert
    createPrimitiveRect(&rectangle, "rect1",
        100.0f, 100.0f,   // Position
        100.0f, 20.0f,    // Taille
        0xFF00FF00);      // Vert
    
    // Ligne bleue épaisse
    
    createPrimitiveLine(&line, "line1",
        400.0f, 100.0f,   // Début
        500.0f, 200.0f,   // Fin
        5.0f,             // Épaisseur
        0xFF0000FF);      // Bleu
    
    // Cercle jaune
    
    createPrimitiveCircle(&circle, "circle1",
        300.0f, 300.0f,   // Centre
        50.0f,            // Rayon
        32,               // Segments
        0xFFFFFF00);      // Jaune

    // Polygone magenta (pentagoné)
    float points[] = {
        100.0f, 300.0f,   // Point 1
        200.0f, 300.0f,   // Point 2
        250.0f, 400.0f,   // Point 3
        150.0f, 450.0f,   // Point 4
        50.0f,  400.0f    // Point 5
    };
    
    createPrimitivePolygon(&polygon, "polygon1", points, 5, 0xFFFFFFFF);
}

int main() {
    // Initialisation
    pvr_init_defaults();
    fs_romdisk_mount("/rd", romdisk, romdisk_end - romdisk);

    init_scene();

    // Points définissant un tracé en zigzag
    float points[] = {
        50.0f, 50.0f,  // Point de départ
        20.0f, 200.0f,  // Point 2
        300.0f, 200.0f,   // Point 3
        300.0f, 250.0f,  // Point 4
        500.0f, 260.0f   // Point final
    };

    dct_model_t path;
    createPrimitivePath(&path, "zigzag", points, 5, 10.0f, 0xFFFF00FF);

    matrix_t mat_dbg;
    matrix_t mat_persp;
    //vec3f_t CAM_POSITION = {0.0f,0.0f,-1000.0f};

    
    dct_camera_t *currentCam = getCurrentCamera();
    initDefaultCurrentCamera(currentCam);
    setCurrentCameraPosition(currentCam,-200.0f,-10.0f,-300.0f);

    BitmapFont* font = dct_init_font("/rd", "/rd/Arial.fnt");
    if (!font) return 1;

    TextProperties props = {
        .x = 20.0f,
        .y = 20.0f,
        .scale = 1.0f,
        .color = 0xFFFFFFFF
    };


    // Boucle principale
    while(1) {
        // Vérifier le bouton START pour quitter
        MAPLE_FOREACH_BEGIN(MAPLE_FUNC_CONTROLLER, cont_state_t, state)
        
            if(state->joyx > 64 )
            {
                currentCam->position.x -=1; 
            }
            if(state->joyx < -64 )
            {
                currentCam->position.x +=1; 
            }
            if(state->joyy > 64 )
            {
                currentCam->position.y +=1; 
            }
            if(state->joyy < -64 )
            {
                currentCam->position.y -=1; 
            }
            if(state->buttons & CONT_DPAD_UP )
            {
                currentCam->position.z +=3; 
                printf("Current Cam pos %f %f %f \n",currentCam->position.x,currentCam->position.y,currentCam->position.z);
                // triangle.position.y -= 1;
                // rectangle.position.y -= 1;
                // circle.position.y -= 1;
                // line.position.y -= 1;
                // path.position.y -= 1;
                // polygon.position.y -= 1;
            }
            if(state->buttons & CONT_DPAD_DOWN )
            {
                currentCam->position.z -=3; 
                printf("Current Cam pos %f %f %f \n",currentCam->position.x,currentCam->position.y,currentCam->position.z);

                // triangle.position.y += 1;
                // rectangle.position.y += 1;
                // circle.position.y += 1;
                // line.position.y += 1;
                // path.position.y += 1;
                // polygon.position.y += 1;
            }
            if(state->buttons & CONT_DPAD_LEFT )
            {
                currentCam->position.x +=1; 
                printf("Current Cam pos %f %f %f \n",currentCam->position.x,currentCam->position.y,currentCam->position.z);

                // triangle.position.x -= 1;
                // rectangle.position.x -= 1;
                // circle.position.x -= 1;
                // line.position.x -= 1;
                // path.position.x -= 1;
                // polygon.position.x -= 1;
            }
            if(state->buttons & CONT_DPAD_RIGHT )
            {
                currentCam->position.x -=1; 
                printf("Current Cam pos %f %f %f \n",currentCam->position.x,currentCam->position.y,currentCam->position.z);

                // triangle.position.x += 1;
                // rectangle.position.x += 1;
                // circle.position.x += 1;
                // line.position.x += 1;
                // path.position.x += 1;
                // polygon.position.x += 1;
            }
                
        MAPLE_FOREACH_END()

        // mat_identity();

        // float znear = 0.01f;
        // float zfar = 100.0f;
        // float fovy_rad = F_PI/4.0f;  // 45 degrés
        // float cot = 1.0f / dct_tan(fovy_rad); // tan est buggé et retourne 4.0 pour tan(0.785f)
        // mat_perspective(640.0f/2.0f, 480.0f/2.0f, cot, znear, zfar);
        // mat_translate(0.0f, 0.0f, -30.0f);

        // mat_store(&mat_persp);
        
        //mat_translate(CAM_POSITION.x, CAM_POSITION.y, CAM_POSITION.z);
        
        updateCurrentCamera(currentCam);


        //(&mat_dbg);
        //print_matrix(mat_dbg);
        updateModel(&triangle);

        //mat_store(&mat_dbg);
        //print_matrix(mat_dbg);
        updateModel(&rectangle);

        //mat_store(&mat_dbg);
        //print_matrix(mat_dbg);
        updateModel(&polygon);

        //mat_store(&mat_dbg);
        //print_matrix(mat_dbg);
        updateModel(&line);

        //mat_store(&mat_dbg);
        //print_matrix(mat_dbg);
        updateModel(&circle);

        //mat_store(&mat_dbg);
        //print_matrix(mat_dbg);
        updateModel(&path);



        // Rendu
        pvr_wait_ready();
        pvr_scene_begin();
        pvr_set_bg_color(0.2f,0.2f,0.2f);
        //pvr_list_begin(PVR_LIST_OP_POLY);

        // Dessiner toutes les primitives
        renderModel(&rectangle);
        renderModel(&triangle);
       
        renderModel(&line);
        renderModel(&circle);
        renderModel(&polygon);

        renderModel(&path);

        pvr_list_finish();

        pvr_list_begin(PVR_LIST_TR_POLY);
        dct_draw_text(font,"Demo 3d BasicFlatShapes \n\\c[005000]Dpad to move  " ,props);
        pvr_list_finish();

        pvr_scene_finish();
    }

cleanup:
    // Nettoyage
    freeModel(&path);
    freeModel(&rectangle);
    freeModel(&line);
    freeModel(&circle);
    freeModel(&polygon);
    freeModel(&triangle);
    return 0;
}