#ifndef DCT_SKINNING_T
#define DCT_SKINNING_T

#include <kos.h>

typedef struct 
{
    float weight[4];
    int   indiceBone[4];
    int   weightCount;
} dct_vertexWeight_t;

typedef struct dct_bone_t{
   char                 name[32];
   int                  parent_index;
   struct dct_bone_t    *parent;
   int                  children_count;
   int                  *children_indices;
   struct dct_bone_t    **children;
   float                originalMatrix[16]  __attribute__((aligned(32)));
   float                animatedMatrix[16]  __attribute__((aligned(32)));
   float                finalMatrix[16]     __attribute__((aligned(32)));
   float                matrix[16]          __attribute__((aligned(32)));

   float                initLocalMatrix[16] __attribute__((aligned(32)));
   float                localMatrix[16]     __attribute__((aligned(32)));
   float                bindMatrix[16]      __attribute__((aligned(32)));
   float                invBindMatrix[16]   __attribute__((aligned(32)));
} dct_bone_t;

typedef struct {
   uint32_t bone_count;
   dct_bone_t* bones;
    
} dct_armature_t;

typedef struct {
   char name[32];
   float location[3];
   float rotation[4];  // quaternion
   float scale[3];
} dct_bone_keyframe_t;



typedef struct dct_keyframe_t {
   float time;
   uint32_t bone_count;
   dct_bone_keyframe_t *bones;
} dct_keyframe_t;

typedef struct {
    char name[32];
    uint32_t start_frame;
    uint32_t end_frame;
    uint32_t frame_count;
    struct {
        struct {
            float time;
            uint32_t bone_count;
            struct {
                char name[32];
                float location[3];
                float rotation[4];
                float scale[3];
            } *bones;
        } *frames;
    } frames;
} dct_animation_t;


// Structure pour gérer l'état de l'animation
typedef struct {
    float current_time;       // Temps actuel de l'animation
    float playback_speed;     // Vitesse de lecture (1.0 = normal)
    bool is_playing;          // État de lecture
    bool is_looping;         // Mode boucle activé/désactivé
    uint32_t current_frame;   // Frame actuelle
    dct_animation_t* current_animation; // Animation en cours
} dct_animation_state_t;


void dct_animation_init(dct_animation_state_t* state, dct_animation_t* animation);
void dct_animation_play(dct_animation_state_t* state);
void dct_animation_pause(dct_animation_state_t* state);
void dct_animation_stop(dct_animation_state_t* state);
void dct_animation_set_loop(dct_animation_state_t* state, bool loop);
void slerp_quaternion(float q1[4], float q2[4], float t, float result[4]);


#endif