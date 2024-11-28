#include <kos.h>

//---------------------------------------//
//---------------------------------------//
//------   DREAMCAST TANTO ENGINE  ------//
//---------------------------------------//
//---------------------------------------//


int initDcEngine()
{
    // Par défaut PAL RGB 565

    if(pvr_init_defaults() == -1)
    {
        return -1;
        printf("\n[ENGINE-KOS] Error : pvr has already been initialized. \n");
    }

    pvr_set_pal_format(PVR_PAL_RGB565);


    return 1;
}




