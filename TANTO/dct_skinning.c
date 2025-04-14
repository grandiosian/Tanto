
#include "dct_skinning.h"

// Initialise l'état de l'animation
void dct_animation_init(dct_animation_state_t* state, dct_animation_t* animation) {
    state->current_time = 0.0f;
    state->playback_speed = 1.0f;
    state->is_playing = false;
    state->is_looping = false;
    state->current_frame = 0;
    state->current_animation = animation;
}

// Démarre la lecture de l'animation
void dct_animation_play(dct_animation_state_t* state) {
    state->is_playing = true;
}

// Met en pause l'animation
void dct_animation_pause(dct_animation_state_t* state) {
    state->is_playing = false;
}

// Arrête l'animation et retourne au début
void dct_animation_stop(dct_animation_state_t* state) {
    state->is_playing = false;
    state->current_time = 0.0f;
    state->current_frame = state->current_animation->start_frame;
}

// Active/désactive le mode boucle
void dct_animation_set_loop(dct_animation_state_t* state, bool loop) {
    state->is_looping = loop;
}

// Fonction utilitaire pour l'interpolation sphérique des quaternions (SLERP)
void slerp_quaternion(float q1[4], float q2[4], float t, float result[4]) {
    float dot = q1[0]*q2[0] + q1[1]*q2[1] + q1[2]*q2[2] + q1[3]*q2[3];
    
    // Ajuste le signe si nécessaire
    if (dot < 0.0f) {
        dot = -dot;
        for (int i = 0; i < 4; i++) {
            q2[i] = -q2[i];
        }
    }

    if (dot > 0.9995f) {
        // Interpolation linéaire si les quaternions sont très proches
        for (int i = 0; i < 4; i++) {
            result[i] = q1[i] * (1 - t) + q2[i] * t;
        }
    } else {
        float theta = acosf(dot);
        float sin_theta = sinf(theta);
        float scale1 = sinf((1 - t) * theta) / sin_theta;
        float scale2 = sinf(t * theta) / sin_theta;

        for (int i = 0; i < 4; i++) {
            result[i] = q1[i] * scale1 + q2[i] * scale2;
        }
    }

    // Normalisation du quaternion résultant
    float length = sqrtf(result[0]*result[0] + result[1]*result[1] + 
                        result[2]*result[2] + result[3]*result[3]);
    for (int i = 0; i < 4; i++) {
        result[i] /= length;
    }
}

