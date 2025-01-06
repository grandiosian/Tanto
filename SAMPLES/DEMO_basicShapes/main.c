#include <kos.h>
#include <malloc.h>
#include <dc/pvr.h>
#include <dc/sound/sound.h>
#include <dc/sound/aica_comm.h>
#include "dct_model.h"  // Notre fichier avec toutes les primitives
//#include "dct_camera.h"


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


// Vérification des streams audio
void check_audio_streams() {
    // Pour un stream spécifique
    snd_stream_hnd_t handle = 0; // votre handle de stream
    
    // Mettre à jour l'état du stream
    int status = snd_stream_poll(handle);
    if (status > 0) {
        printf("Stream %d actif\n", handle);
    }
}

// RAM Principale
void check_main_ram() {
    struct mallinfo info = mallinfo();
    printf("RAM Principale:\n");
    printf("Utilisée: %lu KB\n", (unsigned long)(info.uordblks / 1024));
    printf("Non-allouée: %lu KB\n", (unsigned long)(info.fordblks / 1024));
}

// PVR RAM
void check_pvr_ram() {
    uint32_t free_pvr = pvr_mem_available();
    printf("PVR RAM:\n");
    printf("Libre: %lu KB\n", (unsigned long)(free_pvr / 1024));
}

// AICA RAM
void check_aica_memory() {
    volatile unsigned char* aica_base = (unsigned char*)0xa0800000;
    printf("AICA RAM totale: 2MB\n");
    
    // Accès à la RAM AICA nécessite une synchronisation appropriée
    // et doit être fait via les fonctions AICA appropriées
}
void check_real_memory() {
    void* start_ptr = malloc(1);
    void* end_ptr = sbrk(0);  // Obtient la fin du heap
    
    printf("Taille réelle heap: %lu KB\n", 
           (unsigned long)((char*)end_ptr - (char*)start_ptr) / 1024);
    
    free(start_ptr);
}
// Pour tout vérifier
void check_all_memory() {
    check_main_ram();
    check_pvr_ram();
    check_aica_memory();

    check_real_memory();
    
    // Pour les interruptions et DMA
    printf("Registres système:\n");
    printf("État DMA: 0x%08lx\n", *((volatile uint32_t*)0xa05f6800));
}

int main() {
    // Initialisation
    pvr_init_defaults();
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

    initDefaultCurrentCamera();
    dct_camera *currentCam = getCurrentCamera();
    setCurrentCameraPosition(0.0f,0.0f,-100.0f);

    


    // Boucle principale
    while(1) {
        // Vérifier le bouton START pour quitter
        MAPLE_FOREACH_BEGIN(MAPLE_FUNC_CONTROLLER, cont_state_t, state)
            if (state->buttons & CONT_START)
            {
                check_all_memory();
                goto cleanup;
            }
            if(state->buttons & CONT_DPAD_UP )
            {
                triangle.position.y -= 1;
                rectangle.position.y -= 1;
                circle.position.y -= 1;
                line.position.y -= 1;
                path.position.y -= 1;
                polygon.position.y -= 1;
            }
            if(state->buttons & CONT_DPAD_DOWN )
            {
                triangle.position.y += 1;
                rectangle.position.y += 1;
                circle.position.y += 1;
                line.position.y += 1;
                path.position.y += 1;
                polygon.position.y += 1;
            }
            if(state->buttons & CONT_DPAD_LEFT )
            {
                triangle.position.x -= 1;
                rectangle.position.x -= 1;
                circle.position.x -= 1;
                line.position.x -= 1;
                path.position.x -= 1;
                polygon.position.x -= 1;
            }
            if(state->buttons & CONT_DPAD_RIGHT )
            {
                triangle.position.x += 1;
                rectangle.position.x += 1;
                circle.position.x += 1;
                line.position.x += 1;
                path.position.x += 1;
                polygon.position.x += 1;
            }
                
        MAPLE_FOREACH_END()

        mat_identity();

        float znear = 0.01f;
        float zfar = 100.0f;
        float fovy_rad = F_PI/4.0f;  // 45 degrés
        float cot = 1.0f / dct_tan(fovy_rad); // tan est buggé et retourne 4.0 pour tan(0.785f)
        mat_perspective(640.0f/2.0f, 480.0f/2.0f, cot, znear, zfar);
        mat_translate(0.0f, 0.0f, -10.0f);

        mat_store(&mat_persp);
        
        //mat_translate(CAM_POSITION.x, CAM_POSITION.y, CAM_POSITION.z);
        
        updateCurrentCamera();


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