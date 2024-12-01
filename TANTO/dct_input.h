// dc_input.h
#ifndef DCT_INPUT_H
#define DCT_INPUT_H

#include <kos.h>

typedef struct {
    // État actuel (bouton maintenu)
    struct {
        int up;
        int down;
        int left;
        int right;
        int a;
        int b;
        int x;
        int y;
        int start;
        float joyx;
        float joyy;
        int ltrig;
        int rtrig;
    } pressed;
    
    // État "vient d'être pressé"
    struct {
        int up;
        int down;
        int left;
        int right;
        int a;
        int b;
        int x;
        int y;
        int start;
    } just_pressed;
    
    // État "vient d'être relâché"
    struct {
        int up;
        int down;
        int left;
        int right;
        int a;
        int b;
        int x;
        int y;
        int start;
    } just_released;
    
} DCT_ControllerState;

// Initialise le système d'input
// Retourne 1 si succès, 0 si échec
int dct_input_init(void);

// Met à jour l'état du contrôleur spécifié
// port: numéro du port (0-3)
// state: pointeur vers la structure à mettre à jour
// Retourne 1 si succès, 0 si échec
int dct_input_update(int port, DCT_ControllerState* state);

// Vérifie si un contrôleur est branché sur le port spécifié
// port: numéro du port (0-3)
// Retourne 1 si un contrôleur est présent, 0 sinon
int dct_input_is_connected(int port);

#endif // DCT_INPUT_H
