#include "dct_camera.h"

static dct_camera_t currentCamera = {
    .transform[0][0] = 1.0f,
    .transform[0][1] = 0.0f,
    .transform[0][2] = 0.0f,
    .transform[0][3] = 0.0f,

    .transform[1][0] = 0.0f,
    .transform[1][1] = 1.0f,
    .transform[1][2] = 0.0f,
    .transform[1][3] = 0.0f,

    .transform[2][0] = 0.0f,
    .transform[2][1] = 0.0f,
    .transform[2][2] = 1.0f,
    .transform[2][3] = 0.0f,

    .transform[3][0] = 0.0f,
    .transform[3][1] = 0.0f,
    .transform[3][2] = 0.0f,
    .transform[3][3] = 1.0f,

    .position.x = 0.0f,
    .position.y = 0.0f,   
    .position.z = 0.0f,
    .position.w = 0.0f,

    .rotation.x = 0.0f,
    .rotation.y = 0.0f,
    .rotation.z = 0.0f,

    .offsetCenterScreen[0] = 640.0f/2.0f,
    .offsetCenterScreen[1] = 480.0f/2.0f,

    .up.x = 0.0f,
    .up.y = 1.0f,
    .up.z = 0.0f,
    .up.w = 0.0f,

    .target.x = 0.0f,
    .target.y = 0.0f,
    .target.z = 0.0f,
    .target.w = 0.0f,
    .viewAngle = 90.0f,
    .fovDeg = 90.0/2.0f,
    .fovRad = 0.0f,
    .cot    = 0.0f,
    .znear = 0.01f,
    .zfar  = 1000.0f
    
    };


dct_camera_t* getCurrentCamera()
{
    return &currentCamera;
}


void initDefaultCurrentCamera(dct_camera_t *cam)
{
    cam->transform[0][0] = 1.0f;
    cam->transform[0][1] = 0.0f;
    cam->transform[0][2] = 0.0f;
    cam->transform[0][3] = 0.0f;

    cam->transform[1][0] = 0.0f;
    cam->transform[1][1] = 1.0f;
    cam->transform[1][2] = 0.0f;
    cam->transform[1][3] = 0.0f;

    cam->transform[2][0] = 0.0f;
    cam->transform[2][1] = 0.0f;
    cam->transform[2][2] = 1.0f;
    cam->transform[2][3] = 0.0f;

    cam->transform[3][0] = 0.0f;
    cam->transform[3][1] = 0.0f;
    cam->transform[3][2] = 0.0f;
    cam->transform[3][3] = 1.0f;

    cam->position.x = 0.0f;
    cam->position.y = 0.0f;
    cam->position.z = 0.0f;

    cam->target.x = 0.0f;
    cam->target.y = 0.0f;
    cam->target.z = 0.0f;

    cam->up.x = 0.0f;
    cam->up.y = 1.0f;
    cam->up.z = 0.0f;

    cam->viewAngle = 60.0f;
    setCurrentCameraFOV(cam,cam->fovDeg);
    cam->znear = 0.01f;
    cam->zfar = 1000.0f;

    updateCurrentCamera(cam);
}



void setCurrentCameraPosition(dct_camera_t *cam, float x, float y, float z)
{
    cam->position.x = x;
    cam->position.y = y;
    cam->position.z = z;
    updateCurrentCamera(cam);
}


void setCurrentCameraTarget(dct_camera_t *cam,float x, float y, float z)
{
    cam->target.x = x;
    cam->target.y = y;
    cam->target.z = z;
    updateCurrentCamera(cam);
}

void setCurrentCameraUp(dct_camera_t *cam,float x, float y, float z)
{
    cam->target.x = x;
    cam->target.y = y;
    cam->target.z = z;
    updateCurrentCamera(cam);
}

void setCurrentCameraFOV(dct_camera_t *cam, float viewAngle)
{
    printf(BLEU_START_FN"\n\n{ >>> START >>> [SET-CURRENTCAMERA-FOV]\n"BLEU_INSIDE_FN); 
    //cam->viewAngle = viewAngle/2.0f;
    cam->fovDeg = cam->viewAngle;
    cam->fovRad = 3.14159265f/180.0f * cam->fovDeg;
    cam->cot    = 1.0f / tan( cam->fovRad/2 ); 
    updateCurrentCamera(cam);
    printf("\nset current camera fov cot:%f viewAngle :%f \n",cam->cot,cam->viewAngle);

    printf(BLEU_END_FN"\n\n  [SET-CURRENTCAMERA-FOV] <<< END <<< }\n"ANSI_COLOR_RESET);
}

void updateCurrentCamera(dct_camera_t *cam)
{
    mat_identity();
    mat_perspective(cam->offsetCenterScreen[0], cam->offsetCenterScreen[1], cam->cot, cam->znear, cam->zfar);
    mat_store(&cam->view);

    mat_identity();
    mat_translate(cam->position.x, cam->position.y, cam->position.z);
    mat_rotate(cam->rotation.x, cam->rotation.y, cam->rotation.z);
    //mat_lookat(&cam->position, &cam->target,&cam->up);
    mat_store(&cam->transform);

    mat_identity();
    mat_apply(cam->view);
    mat_apply(cam->transform);
    mat_store(&cam->final);
}


void debugCurrentCamera(dct_camera_t *cam)
{
    printf(BLEU_START_FN"\n{ >>> START >>> [DEBUG CURRENT-CAMERA] \n");
    printf("mat transform : \n");
    print_matrix(&cam->transform);
    printf("position : x%f y%f z%f w%f\n", cam->position.x, cam->position.y,cam->position.z,cam->position.w);
    printf("offsetCenter : x%f y%f \n", cam->offsetCenterScreen[0], cam->offsetCenterScreen[1]);
    printf("up : x%f y%f z%f w%f \n", cam->up.x,cam->up.y,cam->up.z,cam->up.w );
    printf("target : x%f y%f z%f w%f \n", cam->target.x,cam->target.y,cam->target.z,cam->target.w );
    printf("fovDeg :%f fovRad: %f cot: %f znear:%f zfar:%f \n",cam->fovDeg,cam->fovRad,cam->cot,cam->znear,cam->zfar);
    printf("[DEBUG CURRENT-CAMERA] <<< END }\n");
}
