#ifndef CAMERA_H
#define CAMERA_H

#include <kos.h>
#include "dct_utils.h"



typedef struct {

    matrix_t  view;             // matrice view pour le calcul du clipping notamment
    matrix_t  transform;
    point_t   position;
    float     offsetCenterScreen[2]; // 640.0f/2.0f, 480.0f/2.0
    vector_t  up;       
    point_t   target;
    float     fovDeg;                // pour l'affichage et la consigne
    float     fovRad;                // pour le calcul ça evite de le recalculer
    float     cot;                   // le parametre envoyé a la matrice perpsective 1.0f / dct_tan( currentCamera.fovRad );
    float     znear;
    float     zfar;


} dct_camera;


dct_camera* getCurrentCamera();


void initDefaultCurrentCamera();
void setCurrentCameraPosition(float x, float y, float z);
void setCurrentCameraTarget(float x, float y, float z);
void setCurrentCameraUp(float x, float y, float z);
void setCurrentCameraFOV(float fovDeg);
void updateCurrentCamera();
void debugCurrentCamera();



#endif