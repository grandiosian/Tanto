// dct_input.c
#include "dct_input.h"

// Structure interne pour garder trace de l'état précédent
static struct {
    uint32_t buttons;
    int initialized;
} dct_input_previousState[4] = {0};

int dct_input_init(void) {
    for(int i = 0; i < 4; i++) {
        dct_input_previousState[i].buttons = 0;
        dct_input_previousState[i].initialized = 0;
    }
    maple_device_t *cont = maple_enum_type(0, MAPLE_FUNC_CONTROLLER);
    return (cont != NULL);
}

int dct_input_update(int port, DCT_ControllerState* state) {
    if (!state || port < 0 || port > 3) {
        return 0;
    }

    maple_device_t *cont = maple_enum_type(port, MAPLE_FUNC_CONTROLLER);
    if (!cont) {
        return 0;
    }

    cont_state_t *cont_state = (cont_state_t *)maple_dev_status(cont);
    if (!cont_state) {
        return 0;
    }

    uint32_t current_buttons = cont_state->buttons;
    uint32_t prev_buttons = dct_input_previousState[port].buttons;
    
    // Mise à jour des boutons pressés
    state->pressed.up = (current_buttons & CONT_DPAD_UP) ? 1 : 0;
    state->pressed.down = (current_buttons & CONT_DPAD_DOWN) ? 1 : 0;
    state->pressed.left = (current_buttons & CONT_DPAD_LEFT) ? 1 : 0;
    state->pressed.right = (current_buttons & CONT_DPAD_RIGHT) ? 1 : 0;
    state->pressed.a = (current_buttons & CONT_A) ? 1 : 0;
    state->pressed.b = (current_buttons & CONT_B) ? 1 : 0;
    state->pressed.x = (current_buttons & CONT_X) ? 1 : 0;
    state->pressed.y = (current_buttons & CONT_Y) ? 1 : 0;
    state->pressed.start = (current_buttons & CONT_START) ? 1 : 0;

    if (dct_input_previousState[port].initialized) {
        // Mise à jour des boutons just pressed (vient d'être pressé)
        state->just_pressed.up = (current_buttons & CONT_DPAD_UP) && !(prev_buttons & CONT_DPAD_UP);
        state->just_pressed.down = (current_buttons & CONT_DPAD_DOWN) && !(prev_buttons & CONT_DPAD_DOWN);
        state->just_pressed.left = (current_buttons & CONT_DPAD_LEFT) && !(prev_buttons & CONT_DPAD_LEFT);
        state->just_pressed.right = (current_buttons & CONT_DPAD_RIGHT) && !(prev_buttons & CONT_DPAD_RIGHT);
        state->just_pressed.a = (current_buttons & CONT_A) && !(prev_buttons & CONT_A);
        state->just_pressed.b = (current_buttons & CONT_B) && !(prev_buttons & CONT_B);
        state->just_pressed.x = (current_buttons & CONT_X) && !(prev_buttons & CONT_X);
        state->just_pressed.y = (current_buttons & CONT_Y) && !(prev_buttons & CONT_Y);
        state->just_pressed.start = (current_buttons & CONT_START) && !(prev_buttons & CONT_START);

        // Mise à jour des boutons just released (vient d'être relâché)
        state->just_released.up = !(current_buttons & CONT_DPAD_UP) && (prev_buttons & CONT_DPAD_UP);
        state->just_released.down = !(current_buttons & CONT_DPAD_DOWN) && (prev_buttons & CONT_DPAD_DOWN);
        state->just_released.left = !(current_buttons & CONT_DPAD_LEFT) && (prev_buttons & CONT_DPAD_LEFT);
        state->just_released.right = !(current_buttons & CONT_DPAD_RIGHT) && (prev_buttons & CONT_DPAD_RIGHT);
        state->just_released.a = !(current_buttons & CONT_A) && (prev_buttons & CONT_A);
        state->just_released.b = !(current_buttons & CONT_B) && (prev_buttons & CONT_B);
        state->just_released.x = !(current_buttons & CONT_X) && (prev_buttons & CONT_X);
        state->just_released.y = !(current_buttons & CONT_Y) && (prev_buttons & CONT_Y);
        state->just_released.start = !(current_buttons & CONT_START) && (prev_buttons & CONT_START);
    } else {
        // Premier frame, initialiser just_pressed et just_released à 0
        memset(&state->just_pressed, 0, sizeof(state->just_pressed));
        memset(&state->just_released, 0, sizeof(state->just_released));
        dct_input_previousState[port].initialized = 1;
    }

    // Mise à jour du joystick analogique
    state->pressed.joyx = ((float)cont_state->joyx) / 128.0f;
    state->pressed.joyy = ((float)cont_state->joyy) / 128.0f;
    
    // Mise à jour des gâchettes analogiques
    state->pressed.ltrig = cont_state->ltrig;
    state->pressed.rtrig = cont_state->rtrig;

    // Sauvegarder l'état actuel pour le prochain frame
    dct_input_previousState[port].buttons = current_buttons;

    return 1;
}

int dct_input_is_connected(int port) {
    if (port < 0 || port > 3) {
        return 0;
    }
    maple_device_t *cont = maple_enum_type(port, MAPLE_FUNC_CONTROLLER);
    return (cont != NULL);
}