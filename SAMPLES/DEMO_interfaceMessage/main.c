#include "dct_guiText.h"
#include "dct_input.h"

int main() {
    pvr_init_defaults();
    pvr_set_bg_color(0.2f, 0.2f, 0.2f);

    dct_input_init();
    DCT_ControllerState controller = {0};

    
    BitmapFont* font = dct_init_font("/cd", "/cd/Arial.fnt");
    if (!font) return 1;

    const char* texts[] = {
        "\\c[FF0000]Bonjour !\\ce\nAppuyez sur A pour continuer...",
        "\\c[00FF00]Voici un texte animé\\ce\nLigne 2",
        "\\c[0000FF]Dernier message\\ce"
    };
    
    TextSequence* seq = dct_init_text_sequence(texts, 3, 15.0f); // 15 chars/sec
    TextProperties props = {
        .x = 50.0f,
        .y = 50.0f,
        .scale = 1.0f,
        .color = 0xFFFFFFFF
    };

    uint64_t last_time = timer_ms_gettime64();

    while(1) {
        dct_input_update(0, &controller);

        uint64_t current_time = timer_ms_gettime64();
        float delta = (current_time - last_time) / 1000.0f;
        last_time = current_time;

        bool button_pressed = false;

        MAPLE_FOREACH_BEGIN(MAPLE_FUNC_CONTROLLER, cont_state_t, st)
            //if (st->buttons & CONT_A) button_pressed = true;
            if (st->buttons & CONT_START) goto cleanup;
        MAPLE_FOREACH_END()

        if (controller.just_released.up)
        {
            button_pressed = true;
        }

        dct_update_text_sequence(seq, delta, button_pressed);

        pvr_wait_ready();
        pvr_scene_begin();
        pvr_list_begin(PVR_LIST_TR_POLY);
        
        dct_draw_text_sequence(seq, font, props);
        
        pvr_list_finish();
        pvr_scene_finish();
    }

cleanup:
    dct_free_text_sequence(seq);
    dct_free_font(font);
    return 0;
}