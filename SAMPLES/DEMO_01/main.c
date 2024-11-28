#include <kos.h>
#include "TANTO/dct_utils.h"
#include "TANTO/dct_primitives.h"

vec3f_t CAM_POSITION;





int main(void) {

    pvr_init_defaults();
    pvr_set_pal_format(PVR_PAL_RGB565);

    matrix_t mat_persp;

    CAM_POSITION.x = 0.0f;
    CAM_POSITION.y = 0.0f;
    CAM_POSITION.z = -2.0f;

    primitive_pentahedre_t *penta;
    penta = createPentahedre();
    
    primitive_cube_t *cube;
    cube = createCube();

    primitive_sphere_t *sphere;
    sphere = createSphere();


    printf("HELLO OOOOOOOOO \n");

    while(1) {

        pvr_set_bg_color(0.2f, 0.2f, 0.2f);

        
        //---------------------------------//
        //----   MATRIX TRANSFORMATION ----//
        //---------------------------------//
        
        mat_identity();

       
        float znear = 0.01f;
        float zfar = 100.0f;
        float fovy_rad = F_PI/4.0f;  // 45 degrés
        float cot = 1.0f / my_tan(fovy_rad); // tan est buggé et retourne 4.0 pour tan(0.785f)
        mat_perspective(640.0f/2.0f, 480.0f/2.0f, cot, znear, zfar);
        mat_store(&mat_persp);
        
        mat_translate(CAM_POSITION.x, CAM_POSITION.y, CAM_POSITION.z);

        //mat_rotate_y(angle);
        
        

        
        //-------------------------------------------------//
        //----   VERTEX TRANSFORMATION BASED ON MATRIX ----//
        //-------------------------------------------------//

        updateModel(&penta->model);
        updateModel(&cube->model);
        updateModel(&sphere->model);

        //----------------------------------//
        //----------------------------------//
        //----------------------------------//
        //------------- RENDU --------------//
        //----------------------------------//
        //----------------------------------//
        //----------------------------------//
        
        //----------------------------------//
        //---- MODEL OPAQUES EN PREMIER ----//
        //----------------------------------//

        pvr_wait_ready();
        pvr_scene_begin();

        //pvr_list_begin(PVR_LIST_OP_POLY);

        renderModel(&penta->model);
        renderModel(&cube->model);
        renderModel(&sphere->model);
        

        pvr_list_finish();
        pvr_scene_finish();

        //--------------------------------------//
        //---- MODEL TRANSPARENT EN PREMIER ----//
        //--------------------------------------//



        //---------------------------//
        //---- UPDATE CONTROLLER ----//
        //---------------------------//


        MAPLE_FOREACH_BEGIN(MAPLE_FUNC_CONTROLLER, cont_state_t, st)
            if(st->buttons & CONT_START)
            {
                return -1;
            }
            if(st->buttons & CONT_DPAD_UP)
            {
                //angle += 0.1f;

                penta->model.rotation.y += 0.1f;
            }
            if(st->buttons & CONT_DPAD_DOWN)
            {
                //angle -= 0.1f;
                penta->model.rotation.y -= 0.1f;
            }
            if(st->buttons & CONT_DPAD_LEFT)
            {
                //position.x -= 0.1f;
                penta->model.position.x -= 0.1f;
            }
            if(st->buttons & CONT_DPAD_RIGHT)
            {
                //position.x += 0.1f;
                penta->model.position.x += 0.1f;
            }
            
        MAPLE_FOREACH_END()
    }

    return 0;
}