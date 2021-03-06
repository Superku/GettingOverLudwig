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

int kuSoundLoadFile(int fileID, char* buffer);
int kuSoundWriteFile(int fileID, char* buffer, int bufferSize);
int kuSoundDeleteFileW();

void ackSuperkuDLLUpdateReceive(char* c)
{
	cprintf2("\n >> info from DLL at frame %d: %s",(int)total_frames,c);
}

void settingsProcess(int settingsSlot, int number)
{
    if(number < 0 || number > 9000) return; // just in case
    cprintf2("\n - settingsProcess(%d,%d)", settingsSlot, number);
    switch(settingsSlot)
    {
        case 0:
        menuFullscreenWanted = !!number;
        break;

        case 1:
        screenSizeWantedX = number;
        break;
        
        case 2:
        screenSizeWantedY = number;
        break;
        
        case 3:
        fpsCapModeActive = clamp(number, 0, 5);
        break;
        
        case 4:
        backgroundRenderMode = clamp(number, 0, 3);
        break;
        
        case 5:
        kuSoundMasterVolumeInt = clamp(number, 0, 10);
        kuSoundSetMasterVolumeIn100(kuSoundMasterVolumeInt*10);
        break;
        
        case 6:
        playerLeftRightInverted = !!number;
        break;
        
        case 7:
        showTimer = !!number;
        break;
        
    }
}

int settingsLoad()
{
    int bufferSize = kuSoundLoadFile(0, NULL);
    cprintf2("\n settingsLoad at frame %d: bufferSize(%d)", ITF, bufferSize);
    if(bufferSize <= 10) return 0;
    char* buffer = (char*)sys_malloc(bufferSize);
    kuSoundLoadFile(0, buffer);
    char *workbuffer = buffer;
    int number = 0;
    int numberProcessed = 0;
    int settingsSlot = 0;
    while(*workbuffer != 0)
    {
        //cprintf1("[%x]", *workbuffer);
        if(*workbuffer == '\n')
        {
            workbuffer++;
            settingsProcess(settingsSlot, number);
            number = 0;
            numberProcessed = 1;
            settingsSlot++;
        }
        if(*workbuffer >= 48 && *workbuffer <= 57)
        {
            number *= 10;
            number += (*workbuffer)-48;
            numberProcessed = 0;
        }
        workbuffer++;
    }
    if(!numberProcessed) settingsProcess(settingsSlot, number);
    sys_free(buffer);
    return 1;
}

void settingsSave()
{
    STRING* str = str_create("");
    str_printf(str,"%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d", (int)menuFullscreenWanted, (int)screenSizeWantedX, (int)screenSizeWantedY,
    (int)fpsCapModeActive, (int)backgroundRenderMode, (int)kuSoundMasterVolumeInt, (int)playerLeftRightInverted, (int)showTimer);
    //printf("%s", str->chars);
    int ires = kuSoundWriteFile(0, str->chars, str->length);
    cprintf2("\n settingsSave at frame %d: ires(%d)", ITF, ires);
    ptr_remove(str);
}

void saveGameLoad()
{
    //int ires = kuSoundDeleteFileW();
    //cprintf2("\n saveGameLoad at frame %d: ires(%d)", ITF, ires);
    int bufferSize = kuSoundLoadFile(1, NULL);
    cprintf2("\n saveGameLoad at frame %d: bufferSize(%d)", ITF, bufferSize);
    if(bufferSize <= 10) return 0;
    char* buffer = (char*)sys_malloc(bufferSize);
    kuSoundLoadFile(1, buffer);
    var playerPosition[2];
    memcpy(playerPosition, buffer, 8);
    memcpy(gameData, &buffer[8], sizeof(SPEARGAME));
    playerDataPointer = &gameData.playerData;
    if(entityContainer.entDummy)
    {
        vec_set(entityContainer.entDummy.x, vector(playerPosition[0],0,playerPosition[1]));
		if(entityContainer.entHoneyCombo)
        {
            ent_animate(entityContainer.entHoneyCombo, "loadLeft", 0, 0);
            entityContainer.entHoneyCombo.tilt = playerDataPointer->angle;
        }
    }
    menuClosedOnce = 1;
    menuIntroState = 100;
    sys_free(buffer);
}

void saveGameSave()
{
    int bufferSize = 8 + sizeof(SPEARGAME);
    char* buffer = (char*)sys_malloc(bufferSize);
    var playerPosition[2];
    playerPosition[0] = playerPosLocal.x;
    playerPosition[1] = playerPosLocal.z;
    memcpy(buffer, playerPosition, 8);
    memcpy(&buffer[8], gameData, sizeof(SPEARGAME));
    int ires = kuSoundWriteFile(1, buffer, bufferSize);
    cprintf2("\n saveGameSave at frame %d: ires(%d)", ITF, ires);
    sys_free(buffer);
}

void gameQuit()
{
    saveGameSave();
    wait(1);
    sys_exit("");
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
    //on_e = settingsSave;
   // on_f = saveGameSave;
    screenSizeWantedX = sys_metrics(0);
    screenSizeWantedY = sys_metrics(1);
    settingsLoad();
    mapLoad();
    saveGameLoad();
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

    video_window(NULL, NULL, !!menuFullscreenWanted, "Getting over... Ludwig");
    wait(1);
    video_set(screenSizeWantedX, screenSizeWantedY, 32, 0);
    //video_set(1280, 720, 32, 0);
    wait(1);
	if(menuFullscreenWanted) SetWindowPos(hWnd,HWND_TOP,0,0,0,0, SWP_SHOWWINDOW | SWP_NOSIZE); // | SWP_NOMOVE | SWP_NOSIZE  | SWP_NOMOVE // menuFullscreenWanted
    pp_init();
    #ifdef INCLUDE_SOUND
        KUSOUND_INSTANCE_MUSIC = kuSoundPlay2D(KS_MUSIC_MENU, true, 100, 1, 0);
    #endif

    on_frame = mainFrameEvent;
}


#define PRAGMA_PRINT "\n>>> spearGame.c compiled <<<\n";

