#ifndef QUIKFLIK_H
#define QUIKFLIK_H

/**********************************************************
 * Includes
 **********************************************************/
#define INCL_32
#define INCL_DOSRESOURCES
#define INCL_DOSMODULEMGR
#define INCL_DOSFILEMGR
#define INCL_DOSDATETIME
#define INCL_DOSPROCESS
#define INCL_DOSSEMAPHORES
#define INCL_GPI
#define INCL_WIN
#include <os2.h>

#define INCL_MCIOS2
#define INCL_MMIOOS2
#define INCL_MMIO_CODEC
#include <os2me.h>

#include <stdlib.h>
#include <memory.h>

#ifndef _NPAPI_H_
#include <npapi.h>
#endif

#define NPQKFL_ID 1

#define PLAY_BITMAP_ID           0x00001000
#define PAUSE_BITMAP_ID          0x00001001
#define QMLOGO_BITMAP_ID         0x00001002
#define QFLOGO_BITMAP_ID         0x00001003
#define OPENHAND_ID              0x00001004
#define PULLHAND_ID              0x00001005

#define SCREEN_ID          0x00004000
#define MOVIE_WINDOW_ID    SCREEN_ID + 1
#define LOGO_ID            SCREEN_ID + 2
#define PLAY_BUTTON_ID     SCREEN_ID + 3
#define SLIDER_ID          SCREEN_ID + 4

#define PROPERTIES_ID             0x00005000
#define NONEMBEDDED_ID            PROPERTIES_ID + 3
#define CENTER_ID                 PROPERTIES_ID + 4
#define FILL_ID                   PROPERTIES_ID + 5
#define DETATCH_ID                PROPERTIES_ID + 6
#define CUE_DURING_PREP_ID        PROPERTIES_ID + 7

#define POPUP_MENU_ID      0x00005000
#define MI_SAVEAS          POPUP_MENU_ID + 1

#define WM_CUSTOM_CREATE              WM_USER
#define WM_CUSTOM_GOT_HEADER          WM_USER + 1
#define WM_CUSTOM_OPEN                WM_USER + 2
#define WM_CUSTOM_LOAD                WM_USER + 3
#define WM_CUSTOM_CUE                 WM_USER + 4
#define WM_CUSTOM_PLAY                WM_USER + 5
#define WM_CUSTOM_PAUSE               WM_USER + 6
#define WM_CUSTOM_SEEK                WM_USER + 7
#define WM_CUSTOM_EDIT                WM_USER + 8
#define WM_CUSTOM_DESTROY             WM_USER + 9

#define SEM_PREPARING "\\SEM32\\NPQKFL_PREPARING"
#define CONTROL_PANEL_HEIGHT 20
#define SLIDER_UPDATE_INTERVAL 3000 / 5     // 5 times per second

#define QF_NONEMBED_CENTER   0
#define QF_NONEMBED_TOFIT    1
#define QF_NONEMBED_DETATCH  2
#define QF_NONEMBED_ASPECT   3

#define QF_SCALE_ASPECT   -3.0
#define QF_SCALE_DETATCH  -2.0
#define QF_SCALE_TOFIT    -1.0
#define QF_SCALE_DEFAULT  .0
#define QF_SCALE_CENTER   .0

#ifndef SEEK_SET /* avoids the need to include stdio.h */
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#endif

/**********************************************************
 * Functions
 **********************************************************/
extern "C" int _CRT_init(void);
extern "C" void _CRT_term(void);
extern "C" void __ctordtorInit(void);
extern "C" void __ctordtorTerm(void);

LONG getDefaultPalette(SHORT resourceID, PRGB2 palette, ULONG paletteLength);

#endif
