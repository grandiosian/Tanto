#include "dct_guiText.h"
#include "dct_input.h"

extern uint8 romdisk[];
extern uint8 romdisk_end[];

int main() {
    pvr_init_defaults();
    fs_romdisk_mount("/rd", romdisk, romdisk_end - romdisk);

    pvr_set_bg_color(0.2f, 0.2f, 0.2f);

    dct_input_init();
    dct_controllerState_t controller = {0};

    
    BitmapFont* font = dct_init_font("/rd", "/rd/Arial.fnt");
    if (!font) return 1;

    const char* texts[] = {
        "\\c[FF0000]Bonjour !\\ce\nAppuyez sur A pour continuer...",
        "\\c[00FF00]Voici un texte animé\\ce\nLigne 2",
        "\\c[0000FF]Dernier message\\ce"
    };
    
    TextProperties props = {
        .x = 50.0f,
        .y = 50.0f,
        .scale = 1.0f,
        .color = 0xFFFFFFFF
    };

    
    

    while(1) {
        dct_input_update(0, &controller);

        bool button_pressed = false;

        MAPLE_FOREACH_BEGIN(MAPLE_FUNC_CONTROLLER, cont_state_t, st)
            if (st->buttons & CONT_A) button_pressed = true;
            if (st->buttons & CONT_START) goto cleanup;
        MAPLE_FOREACH_END()

        if (controller.just_released.up)
        {
            button_pressed = true;
        }

        pvr_wait_ready();
        pvr_scene_begin();
        pvr_list_begin(PVR_LIST_TR_POLY);
        
        dct_draw_text(font,"Gui texte \\c[FF0000]Rouge \\c[00FF00]Vert \\c[0000FF]Bleu \n \\c[FFFFFF]Ligne 2 \n Ligne 3 " ,props);
        
        pvr_list_finish();
        pvr_scene_finish();
    }

cleanup:
    dct_free_font(font);
    return 0;
}