//////////////////////////////
// SpearGame by Superku
// Now called Getting over... Ludwig
// Fake working title: Spearstruck
// Entry for Ludwig Jam
//////////////////////////////

#define PRAGMA_PRINT "\n>>> spearGame.c compile start <<<\n";

#define INCLUDE_SOUND
//#define DEVTRUE // if defined == development build

#include <acknex.h> // include game engine
#ifdef DEVTRUE
    //#include <default.c> // include default super basic debugging code
    #include <windows.h>
    #include <d3d9.h>
    #include <mdlSave.c>
#endif
#include "console2.h"
#include "kuGetDisplayModes.h"
#include "spearTypes.h"
#include "spearHelper.c"
#ifdef INCLUDE_SOUND
	#include "kuSoundSpear.h"
	#include "kuSoundSpear.c"
#endif

#include "ackXinput.h"
#include "spearInput.h"
#include "spearInput.c"
// sound ...
#include "spearMenu.c"
#ifdef DEVTRUE
    #include "spearMaterialsDEV.h"
#else
    #include "spearMaterialsREL.h"
#endif
#include "spearParticles.c"
//#include "spearPlayer.h"
#include "spearPlayer.c"
#include "spearCamera.c"
#include "spearPostprocessing.c"


void gameInitPreLevelLoad()
{
    return;
}

void gameInitPostLevelLoad()
{
    mapReset();
    spearPlayerInit();
    //menuToggle();
}

void mapLoad()
{
    menuClearBeforeLevelLoad();
  	//kuSoundStopAllLevelSounds(0);
    memset(entityContainer, 0, sizeof(ENTITYCONTAINER));
    gameInitPreLevelLoad();
    level_load("map.wmb");
    gameInitPostLevelLoad();
}

void gameUpdate()
{
    if(inputJustPressed(KUINPUT_BUTTON_RETRY) && !menuActive)
    {
        #ifdef DEVTRUE
            mapLoad();
            #else
        mapReset();
        #endif
    }
    playerMove();
    cameraMove(1);
}

void gameQuit()
{
    sys_exit("");
}

void on_exit_event()
{
    #ifdef INCLUDE_SOUND
        kuSoundDestroyDevice();
    #endif
}

int screenshotActive = 0;
void mainFrameEvent()
{
    if(screenshotActive) return;
    if(total_ticks > 100000) total_ticks = 16;
	if(total_frames > 100000) total_frames -= 100000;
    int inputAllowed = 1;
    inputUpdate(inputAllowed, menuActive);
    if(menuActive) menuUpdate();
    if(outroState > 0)
    {
        outroDo();
    }
    else
    {
        if(gameState == GAMESTATE_PLAY) gameUpdate();
    }
    if(inputJustPressed(KUINPUT_BUTTON_OPENMENU)) menuToggle();
    //if(ackXInputGetButtonState3(4)) gameQuit();
    pp_update();
    if(deathPercent > 0)
    {
        deathPercent = maxv(deathPercent-6*time_step, 0);
	    draw_quad(NULL,vector(0,0,0),NULL,vector(screen_size.x+1,screen_size.y+1,0),NULL,COLOR_BLACK,deathPercent*0.30,0);
	    draw_quad(NULL,vector(0,0,0),NULL,vector(screen_size.x+1,screen_size.y+1,0),NULL,COLOR_RED,deathPercent*0.75,0);
    }
    #ifdef INCLUDE_SOUND
    	kuSoundUpdateFrame(camera);
    #endif
}


void take_screenshot()
{
	var fhandle;
	static var screen_shot_count = 0;
	
	if(screenshotActive) return;
	freeze_mode = 2+4;
	proc_mode = PROC_NOFREEZE;
	screenshotActive = 1;
	screen_shot_count = 0;
	if(file_exists("ssc.dat"))
	{
		fhandle = file_open_read("ssc.dat");
		screen_shot_count = file_var_read(fhandle);
		file_close(fhandle);
	}	
	file_for_screen("screenshot.png",screen_shot_count);
	screen_shot_count++;
	fhandle = file_open_write("ssc.dat");
	file_var_write(fhandle,screen_shot_count);
	file_close(fhandle);
	wait(1);
	screenshotActive = 0;
	freeze_mode = 0;
}

void main()
{
    set(camera, NOSHADOW); 
    fps_min = 15;
	fps_max = 120;
	//video_set(800,600,0,2); // open window low res first, just to be sure - weird engine
    video_mode = 6;
	mouse_mode = 4;
	mouse_calm = 8;
	d3d_entsort = 3;
	mip_flat = 2.5;
	max_entities = 10000;
    collision_mode = 2;
    //master_vol = 50;
    mapLoad();
    menuOpen();
    //menuOpenPerc = 100;
    vec_set(sky_color, vector(20,180,250));
    wait(1); // wait for the directX device and initial engine boot up
    #ifdef INCLUDE_SOUND
        cprintf2("\nmain function at frame(%d): calling kuSoundInit(%p)...", ITF, hWnd);
        int pSetCoopLevelResult = -1;
        int ires = kuSoundInit(hWnd, &pSetCoopLevelResult);
        if(!ires) kuSoundInitialized = 1;
        kuSoundInitErrorCode = ires;
        cprintf2("OK(%d, SetCoopLevel %d). calling kuSoundSourcesInit...",ires,pSetCoopLevelResult);
        kuSoundSourcesInit();
    #endif
    on_f1 = NULL; // disable engine crap (mostly when default.c is included)
    on_f2 = NULL;
    on_f3 = NULL;
    on_f4 = NULL;
    //on_f5 = NULL; // cycle through (some) screen resolutions
    on_f6 = take_screenshot;
    on_f7 = NULL;
    on_f8 = NULL;
    on_f9 = NULL;
    on_f10 = gameQuit;
    on_esc = NULL;  //menuToggle
    on_exit = on_exit_event;

    video_window(NULL, NULL, 1, "Getting over... Ludwig");
    wait(1);
    screenSizeWantedX = sys_metrics(0);
    screenSizeWantedY = sys_metrics(1);
    video_set(screenSizeWantedX, screenSizeWantedY, 32, 0);
    //video_set(1280, 720, 32, 0);
    wait(1);
	SetWindowPos(hWnd,HWND_TOP,0,0,0,0, SWP_SHOWWINDOW | SWP_NOSIZE); // | SWP_NOMOVE | SWP_NOSIZE  | SWP_NOMOVE
    pp_init();
    #ifdef INCLUDE_SOUND
        KUSOUND_INSTANCE_MUSIC = kuSoundPlay2D(KS_MUSIC_MENU, true, 100, 1, 0);
    #endif

    on_frame = mainFrameEvent;
}


#define PRAGMA_PRINT "\n>>> spearGame.c compiled <<<\n";

