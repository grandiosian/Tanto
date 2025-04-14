#ifndef CAMERA_H
#define CAMERA_H

#include <kos.h>
#include "dct_utils.h"
#include "dct_input.h"



typedef struct {

    matrix_t  view;              // matrice view persp deformation pour le calcul du clipping notamment
    matrix_t  transform;         // position rotation
    matrix_t  final;             // persp deformation + transform
    point_t   position;
    vec3f_t   rotation;
    float     offsetCenterScreen[2]; // 640.0f/2.0f, 480.0f/2.0
    vector_t  up;       
    point_t   target;
    float     viewAngle;
    float     fovDeg;                // pour l'affichage et la consigne
    float     fovRad;                // pour le calcul ça evite de le recalculer
    float     cot;                   // le parametre envoyé a la matrice perpsective 1.0f / dct_tan( currentCamera.fovRad );
    float     znear;
    float     zfar;


} dct_camera_t;


dct_camera_t* getCurrentCamera();


void initDefaultCurrentCamera(dct_camera_t *cam);
void setCurrentCameraPosition(dct_camera_t *cam,float x, float y, float z);
void setCurrentCameraTarget(dct_camera_t *cam,float x, float y, float z);
void setCurrentCameraUp(dct_camera_t *cam,float x, float y, float z);
void setCurrentCameraFOV(dct_camera_t *cam,float fovDeg);
void updateCurrentCamera(dct_camera_t *cam);
void debugCurrentCamera(dct_camera_t *cam);



#endif