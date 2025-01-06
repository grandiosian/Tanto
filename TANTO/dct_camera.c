#include "dct_camera.h"

static dct_camera currentCamera = {
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

    .fovDeg = 60.0f,
    .fovRad = 0.0f,
    .cot    = 0.0f,
    .znear = 0.01f,
    .zfar  = 1000.0f
    
    };


dct_camera* getCurrentCamera()
{
    return &currentCamera;
}

void initDefaultCurrentCamera()
{
    currentCamera.transform[0][0] = 1.0f;
    currentCamera.transform[0][1] = 0.0f;
    currentCamera.transform[0][2] = 0.0f;
    currentCamera.transform[0][3] = 0.0f;

    currentCamera.transform[1][0] = 0.0f;
    currentCamera.transform[1][1] = 1.0f;
    currentCamera.transform[1][2] = 0.0f;
    currentCamera.transform[1][3] = 0.0f;

    currentCamera.transform[2][0] = 0.0f;
    currentCamera.transform[2][1] = 0.0f;
    currentCamera.transform[2][2] = 1.0f;
    currentCamera.transform[2][3] = 0.0f;

    currentCamera.transform[3][0] = 0.0f;
    currentCamera.transform[3][1] = 0.0f;
    currentCamera.transform[3][2] = 0.0f;
    currentCamera.transform[3][3] = 1.0f;

    currentCamera.position.x = 0.0f;
    currentCamera.position.y = 0.0f;
    currentCamera.position.z = 0.0f;

    currentCamera.target.x = 0.0f;
    currentCamera.target.y = 0.0f;
    currentCamera.target.z = 0.0f;

    currentCamera.up.x = 0.0f;
    currentCamera.up.y = 1.0f;
    currentCamera.up.z = 0.0f;

    currentCamera.fovDeg = 45.0f;
    setCurrentCameraFOV(currentCamera.fovDeg);
    currentCamera.znear = 0.01f;
    currentCamera.zfar = 1000.0f;

}

void setMatrixViewCurrentCamera()
{

}


void setCurrentCameraPosition(float x, float y, float z)
{
    currentCamera.position.x = x;
    currentCamera.position.y = y;
    currentCamera.position.z = z;
}


void setCurrentCameraTarget(float x, float y, float z)
{
    currentCamera.target.x = x;
    currentCamera.target.y = y;
    currentCamera.target.z = z;
}

void setCurrentCameraUp(float x, float y, float z)
{
    currentCamera.target.x = x;
    currentCamera.target.y = y;
    currentCamera.target.z = z;
}

void setCurrentCameraFOV(float fov)
{
    
    currentCamera.fovDeg = fov;
    currentCamera.fovRad = F_PI/180.0f * currentCamera.fovDeg;
    currentCamera.cot    = 1.0f / dct_tan( currentCamera.fovRad ); 
    printf("\nset current camera fov cot:%f\n",currentCamera.cot);
}

void updateCurrentCamera()
{


    mat_identity();
    //mat_perspective(currentCamera.offsetCenterScreen[0], currentCamera.offsetCenterScreen[1], currentCamera.cot, currentCamera.znear, currentCamera.zfar);
    //mat_lookat(&currentCamera.position, &currentCamera.target,&currentCamera.up);
    mat_translate(currentCamera.position.x, currentCamera.position.y, currentCamera.position.z);
    mat_store(&currentCamera.transform);



}


void debugCurrentCamera()
{
    printf("\n{ START >>> [DEBUG CURRENT-CAMERA] \n");
    printf("mat transform : \n");
    print_matrix(&currentCamera.transform);
    printf("position : x%f y%f z%f w%f\n", currentCamera.position.x, currentCamera.position.y,currentCamera.position.z,currentCamera.position.w);
    printf("offsetCenter : x%f y%f \n", currentCamera.offsetCenterScreen[0], currentCamera.offsetCenterScreen[1]);
    printf("up : x%f y%f z%f w%f \n", currentCamera.up.x,currentCamera.up.y,currentCamera.up.z,currentCamera.up.w );
    printf("target : x%f y%f z%f w%f \n", currentCamera.target.x,currentCamera.target.y,currentCamera.target.z,currentCamera.target.w );
    printf("fovDeg :%f fovRad: %f cot: %f znear:%f zfar:%f \n",currentCamera.fovDeg,currentCamera.fovRad,currentCamera.cot,currentCamera.znear,currentCamera.zfar);
    printf("[DEBUG CURRENT-CAMERA] <<< END }\n");
}
