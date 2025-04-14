#include <kos.h>
#include "dct_model.h"
#include "dct_camera.h"
#include "dct_utils.h"

extern uint8 romdisk[];
extern uint8 romdisk_end[];

int main() {
    //pvr_init_defaults();

    pvr_init_params_t params = 
    {
        { PVR_BINSIZE_16, PVR_BINSIZE_16, PVR_BINSIZE_16, PVR_BINSIZE_16, PVR_BINSIZE_0 },
        512*1024,
        0,
        0,
        0
    };
    pvr_init(&params);

    fs_romdisk_mount("/rd", romdisk, romdisk_end - romdisk);
    pvr_set_bg_color(0.2f, 0.2f, 0.2f);

    printf(ANSI_COLOR_GREEN "\n Initial start \n" ANSI_COLOR_RESET);

    dct_model_t ground;
    uint32_t c = {0xFFFFFFFF};
    float size = {5.0f};
    createPlane(&ground,"sol",size,c);
    ground.position.z = -10.0F;
    ground.rotation.x = 3.14f/2.0f;
    for(int i=0; i<ground.meshes[0].vtxCount; i++)
    {
        ground.meshes[0].animatedVtx[i].u *= 10.0f;
        ground.meshes[0].animatedVtx[i].v *= 10.0f;

    }
    initMeshesHeader(&ground);
    ground.meshes[0].type = MESH_STD_OP;
    setMeshTexture(&ground, 0 ,"/rd/Pave_stylized_128x128.kmg");

    setModelMeshPCM(&ground.meshes[0], 0xFFFF0000);
    ground.modifier = createModifierVolumeCube();
    mat_identity();
    mat_rotate_x(3.14f/2.0f);
    mat_translate(0,0,-2);
    mat_store(&ground.modifier.transform);

    // ground.meshes[0].type = MESH_STD;

    dct_camera_t *currentCam = getCurrentCamera();
    initDefaultCurrentCamera(currentCam);
    setCurrentCameraPosition(currentCam,0.0f,0.5f,-5.0f);

    while(1) {

        updateCurrentCamera(currentCam);
        updateModel(&ground);


        pvr_wait_ready();
        pvr_scene_begin();

        renderModel(&ground);

        pvr_list_finish();

        pvr_list_begin(PVR_LIST_OP_MOD);
        applyModifier(&ground.modifier);
        pvr_list_finish();

        pvr_scene_finish();

        MAPLE_FOREACH_BEGIN(MAPLE_FUNC_CONTROLLER, cont_state_t, st)
            if(st->buttons & CONT_START)
            {
                return -1;
            }
            if(st->buttons & CONT_DPAD_UP)
            {
                ground.position.y -= 1.0F;
            }
            if(st->buttons & CONT_DPAD_DOWN)
            {
                ground.position.y += 1.0F;
            }
            if(st->buttons & CONT_DPAD_LEFT)
            {
                ground.position.x -= 1.0F;
            }
            if(st->buttons & CONT_DPAD_RIGHT)
            {
                ground.position.x += 1.0F;
            }
            if(st->joyy > 64)
            {
                ground.scale.z += 0.1F;
            }
            if(st->joyy < -64)
            {
                ground.scale.z -= 0.1F;
            }
            if(st->joyx < -64)
            {
                ground.scale.x -= 0.1F;
            }
            if(st->joyx > 64)
            {
                ground.scale.x += 0.1F;
            }
            if(st->ltrig > 20)
            {
                ground.rotation.z += 0.1F;
            }
            if(st->rtrig > 20)
            {
                ground.rotation.z -= 0.1F;
            }
        MAPLE_FOREACH_END()

    }

    return 0;
}