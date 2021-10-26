//////////////////////////////
// spearMenu.c
//////////////////////////////

#define MENUITEM_TYPE_CENTER 0
#define MENUITEM_TYPE_LEFT 1
#define MENUITEM_TYPE_LEFTRIGHT 2
#define MENUITEM_TYPE_PLUSMINUS 3
#define MENUITEM_TYPE_SLIDER 4
#define MENUITEM_TYPE_ARROWS 5

typedef struct _MENUITEM
{
    STRING *strTitle;
    STRING *strValue;
    void *pVar;
    int ID, type, variableType, isActive, isGreyedOut, strTitleID, strValueID, eventID, isYesNo;
    var varMin, varMax;
    COLOR color;
} MENUITEM;

#define MENUPAGE_ITEMS_MAX 16
typedef struct _MENUPAGE
{
    MENUITEM items[MENUPAGE_ITEMS_MAX];
    int ID, previousID, hasBackButton, numItemsUsed, numItemsActive, itemSelected, isDirty;
    ENTITY* ent;
} MENUPAGE;
var menuPageItemSelected = 0;

#define MENUPAGE_MAIN 0
#define MENUPAGE_OPTIONS 1
typedef struct _MENUMANAGER
{
    MENUPAGE pages[2];
    int isInitialized, numStrings, pageIDActive;
    ENTITY* entBackground;
    ENTITY* entTitlescreen;
    ENTITY* entBlackPlane;
} MENUMANAGER;
MENUMANAGER menuManager;
BMAP* bmpMenuStringsEncoded = "menuStrings32x32.tga";
BMAP* bmpMenuFont = "spearFont.tga";
BMAP* hexagonsNM_tga = "hexagons3NM.tga";

int fpsCapModeActive = 2;
int fpsCapModes[6] = { 60, 75, 120, 144, 160, 240 };
int kuSoundMasterVolume = 5;

////////////////////////////////////////////

void menuClearBeforeLevelLoad()
{
    menuManager.entBackground = NULL;
    menuManager.entTitlescreen = NULL;
    menuManager.entBlackPlane = NULL;
    int i;
    for(i = 0; i < 2; i++)
    {
        MENUPAGE *page = &menuManager.pages[i];
        page->ent = NULL;
    }
}

void menuOpen()
{
    menuActive = 1;
    camera.y = -4096;

    MENUPAGE *page = &menuManager.pages[MENUPAGE_MAIN];
    page->itemSelected = !menuClosedOnce;

    ENTITY* ent = ent_for_name("menuPage0"); // page->ent;
    ent.x = playerPosLocal.x+230-ent.min_x; // was 300 up until 26.10.
    ent.y = -1900;
    ent.z = playerPosLocal.z;
    ent.emask |= DYNAMIC;
    //reset(ent, INVISIBLE);

    ENTITY* ent = ent_for_name("menuPage1");
    ent.x = playerPosLocal.x+230-ent.min_x;
    ent.y = -1900;
    ent.z = playerPosLocal.z;
    ent.emask |= DYNAMIC;
    //reset(ent, INVISIBLE);

    if(!menuManager.entTitlescreen) menuManager.entTitlescreen = ent_for_name("titleScreen");
    if(!menuManager.entBackground) menuManager.entBackground = ent_for_name("menuBackground");
    ENTITY* entBackground = menuManager.entBackground;
    entBackground.x = playerPosLocal.x+200;
    //entBackground.y = -1600;
    entBackground.z = playerPosLocal.z;
    entBackground.emask |= DYNAMIC;
    reset(entBackground, INVISIBLE);

    if(!menuManager.entBlackPlane) menuManager.entBlackPlane = ent_for_name("menuBlack");
    ENTITY* entBlackPlane = menuManager.entBlackPlane;
    entBlackPlane.x = entBackground.x;
    //y ??
    entBlackPlane.z = entBackground.z;
    entBlackPlane.emask |= DYNAMIC;
    if(menuClosedOnce) set(entBlackPlane, INVISIBLE);
    else reset(entBlackPlane, INVISIBLE);
    //entBlackPlane.y = camera.y +64;
}

void menuClose()
{
    menuActive = 0;
    menuPagePerc = 0;
    menuOpenPerc = 0;
    menuClosedOnce = 1;
    vec_set(sky_color, vector(30,38,96));
    int i;
    for(i = 0; i < 2; i++)
    {
        MENUPAGE *page = &menuManager.pages[i];
        if(page->ent) set(page->ent, INVISIBLE);
    }

    set(menuManager.entBackground, INVISIBLE);
    set(menuManager.entTitlescreen, INVISIBLE);
    set(menuManager.entBlackPlane, INVISIBLE);
}

void menuToggle()
{
    menuActive = !menuActive;
    if(!menuActive) menuClose();
    else
    {
        menuOpenPerc = 0;
        menuPagePerc = 0;
        menuTitleScreenPerc = 0;
        menuTitleHidePerc = 100;
        menuOpen();
    }
}

MENUITEM* menuPageItemAdd(MENUPAGE *page, char *cTitle, char *cValue, int type, int eventID)
{
    if(page->numItemsUsed >= MENUPAGE_ITEMS_MAX) return NULL;
    MENUITEM *item = &page->items[page->numItemsUsed];
    item->ID = page->numItemsUsed++;
    item->type = type;
    item->eventID = eventID;
    item->isActive = 1;
    item->isYesNo = -1;
    vec_set(&item->color, COLOR_WHITE);
    if(cTitle) item->strTitle = str_create(cTitle);
    if(cValue) item->strValue = str_create(cValue);
    else item->strValue = str_create("");
    return item;
}

MENUITEM* menuPageItemAddArrows(MENUPAGE *page, char *cTitle, char *cValue, int type, int eventID, void *pVar, var varMin, var varMax)
{
    MENUITEM *item = menuPageItemAdd(page, cTitle, cValue, type, eventID);
    if(!item) return NULL;
    item->pVar = pVar;
    item->varMin = varMin;
    item->varMax = varMax;
    return item;
}


void menuPageEncodeStringToBitmap(BMAP* bmp, COLOR* color, STRING *str, int y, int right)
{
    if(y >= bmp->height) return;
    char *finalbits = bmp->finalbits;
    int bpp = bmp->finalbytespp;
    int dataPerRow = 32*bpp;
    int dataY = y*dataPerRow;
    finalbits += dataY;
    //finalbits[0] = color->red;
    //finalbits[1] = color->green;
    //finalbits[2] = color->blue;
    int len = str->length;
    char *chars = str->chars;
    if(len > 24) len = 24;
    int i;
    for(i = 0; i < 32; i++)
    {
        int idx = i*bpp;
        if(right) idx = (31-len+i)*bpp;
        if(i < len)
        {
            int converted = 0;
            int character = chars[i];
            if(character >= 48 && character <= 57) converted = character-48;
            if(character >= 65 && character <= 90) converted = character-65+19;
            if(character == 32) converted = 10;
            if(character == 45) converted = 14;
            if(character == 47 || character == 58) converted = 13;
            if(character == 60) converted = 17;
            if(character == 62) converted = 18;
            if(i == len-1) converted = 10;
            finalbits[idx + 0] = converted;
            finalbits[idx + 1] = 0;
        }
        else
        {
            if(right)
            {
                finalbits[idx + 2] = 0;
                return;
            }
            finalbits[idx + 0] = 10;
            finalbits[idx + 1] = 255;
        }
        finalbits[idx + 2] = 0;
    }
}

        /*else
        {
            int converted = 10;
            if(i == 24) converted = 17;
            if(i == 30) converted = 18;
            finalbits[idx + 0] = converted;
            finalbits[idx + 1] = 255;
        }*/

void menuPageUpdateStrings(MENUPAGE *page)
{
    BMAP* bmp = bmpMenuStringsEncoded;
    var fomat = bmap_lock(bmp, 0);
    int bpp = bmp->finalbytespp;
    char *finalbits = bmp->finalbits;
    static int memsetOnce = 0;
    if(finalbits && !memsetOnce)
    {
        memset(finalbits, 10, bpp*bmp->width*bmp->height);
        memsetOnce = 1;
    }
    int i;
    for(i = 0; i < page->numItemsUsed; i++)
    {
        MENUITEM *item = &page->items[i];
        /*item->strTitleID = (page->ID * 16 + i*2) + 0;
        item->strValueID = (page->ID * 16 + i*2) + 1;
        if(item->strTitle) menuPageEncodeStringToBitmap(bmp, item->color, item->strTitle, item->strTitleID);
        if(item->strValue) menuPageEncodeStringToBitmap(bmp, item->color, item->strValue, item->strValueID);*/
        item->strTitleID = (page->ID * 16 + i);
        item->strValueID = item->strTitleID;
        if(item->strTitle) menuPageEncodeStringToBitmap(bmp, item->color, item->strTitle, item->strTitleID, 0);
        if(item->strValue) menuPageEncodeStringToBitmap(bmp, item->color, item->strValue, item->strValueID, 1);
    }
    bmap_unlock(bmp);
}

void gameQuit();
void mapLoad();

void itemEventExecute(MENUPAGE *page, MENUITEM *item, int eventID)
{
    switch(eventID)
    {
        case 0:
        gameQuit();
        break;

        case 1:
        if(menuIntroState < 100)
        {
            cprintf1("\n itemEventExecute: NEW GAME at frame %d", ITF);
            menuIntroState = 20;
        }
        else
        {
            menuClose();
            mapLoad();
        }
        break;

        case 2:
        menuClose();
        break;

        case 3:
        menuPagePerc = 0;
        menuManager.pageIDActive = MENUPAGE_OPTIONS;
        MENUPAGE *page2 = &menuManager.pages[MENUPAGE_OPTIONS];
        page2->itemSelected = 0;
        multiMonitorGetInfo();
        break;

        case 4:
        menuPagePerc = 0;
        menuManager.pageIDActive = page->previousID;
        MENUPAGE *page2 = &menuManager.pages[MENUPAGE_MAIN];
        page2->itemSelected = !menuClosedOnce;
        break;
    }
}

void pp_reset_from_menu();
void pp_clear_viewstages();
void pp_remove_rendertargets();

void menuAdjustHWnd()
{
    MYMONITORINFO *pinfo = MMInfoGetMonitorInfo(primMonitorID);
	if(pinfo)
	{
		int windowPosX = pinfo->monitorRect.left;
		int windowPosY = pinfo->monitorRect.top;
		var width = (pinfo->monitorRect.right-pinfo->monitorRect.left);
		var height = (pinfo->monitorRect.bottom-pinfo->monitorRect.top);
		if(width > 1) windowPosX += (width-screen_size.x)*0.5;
		if(height > 0) windowPosY += (height-screen_size.y)*0.5;

        RECT sizeRect;
        sizeRect.left = 0;
        sizeRect.right = screen_size.x;
        sizeRect.top = 0;
        sizeRect.bottom = screen_size.y;
        AdjustWindowRect(&sizeRect,(DWORD)GetWindowLong(hWnd, GWL_STYLE),0);
        int newWindowSizeX = sizeRect.right-sizeRect.left;
        int newWindowSizeY = sizeRect.bottom-sizeRect.top;
		SetWindowPos(hWnd,HWND_TOP,windowPosX,windowPosY,newWindowSizeX,newWindowSizeY,SWP_SHOWWINDOW);
	}
}

void menuSetMonitorMode(int monitorWanted)
{
    if(proc_status(menuSetMonitorMode) > 0) return;
	cprintf2("\nmenuSetMonitorMode(%d) at frame %d - START", monitorWanted, (int)total_frames);
    //pp_clear_viewstages();
    //pp_remove_rendertargets();
    if(monitorWanted)
    {
	    //video_set(screenSizeWantedX, screenSizeWantedY, 0, 0);
        wait(1);
        video_window(NULL,NULL,244,NULL); // 128+64+16+32+4 = 244
    }
    else
    {
        video_window(NULL,NULL,1,NULL);
        wait(1);
        screenSizeWantedX = sys_metrics(0);
        screenSizeWantedY = sys_metrics(1);
	    video_set(screenSizeWantedX, screenSizeWantedY, 0, 0);
    }
    /*pp_clear_viewstages();
    wait(3);*/
    menuAdjustHWnd();
    wait(1);
    pp_reset_from_menu();
}

void menuApplyResolution()
{
    if(proc_status(menuApplyResolution) > 0) return;
	cprintf1("\n menuApplyResolution at frame %d - START", ITF);
    pp_clear_viewstages();
    pp_remove_rendertargets();
    video_set(screenSizeWantedX, screenSizeWantedY, 0, 0);
    menuAdjustHWnd();
    wait(1);
    pp_reset_from_menu();
}

void menuPageUpdate( MENUPAGE *page)
{
    int i;
    if(inputJustPressed(KUINPUT_BUTTON_UP)) page->itemSelected--;
    if(inputJustPressed(KUINPUT_BUTTON_DOWN)) page->itemSelected++;
    if(page->ID == 0 && !menuClosedOnce)
    {
    if(page->itemSelected < 1) page->itemSelected = page->numItemsUsed-1;
    if(page->itemSelected >= page->numItemsUsed) page->itemSelected = 1;
    }
    else
    {
    if(page->itemSelected < 0) page->itemSelected = page->numItemsUsed-1;
    if(page->itemSelected >= page->numItemsUsed) page->itemSelected = 0;
    }
    menuPageItemSelected = page->itemSelected;

    MENUITEM *itemSelected = &page->items[page->itemSelected];

    if(inputJustPressed(KUINPUT_BUTTON_JUMP) && itemSelected->type == MENUITEM_TYPE_LEFT)
    {
        if(itemSelected->eventID >= 0) itemEventExecute(page, itemSelected, itemSelected->eventID);
    }
    int isDirty = menuManager.pageIDActive * (inputJustPressed(KUINPUT_BUTTON_LEFT) || inputJustPressed(KUINPUT_BUTTON_RIGHT));
    isDirty += page->isDirty;
    //if(page->isDirty) inputSetButtonsOff();
    //COLOR *color = COLOR_WHITE;
    //if(sinv(total_ticks*10) > 0) color = vector(128,40,220);
    for(i = 0; i < page->numItemsUsed; i++)
    {
        MENUITEM *item = &page->items[i];
        if(item->type == MENUITEM_TYPE_ARROWS)
        {
            if(1) //page->isDirty)) // item == itemSelected
            {
                if(item->isYesNo >= 0)
                {
                    int oldValue = item->isYesNo;
                    if(item == itemSelected && inputJustPressed(KUINPUT_BUTTON_LEFT)) item->isYesNo = 0;
                    if(item == itemSelected && inputJustPressed(KUINPUT_BUTTON_RIGHT)) item->isYesNo = 1;
                    if(item->isYesNo) str_cpy(item->strValue, "<YES>");
                    else str_cpy(item->strValue, "< NO>");
                    if(oldValue != item->isYesNo)
                    {
                        if(item->eventID == 100) menuSetMonitorMode(!item->isYesNo);
                        if(item->eventID == 104) playerLeftRightInverted = item->isYesNo;
                        if(item->eventID == 105) showTimer = item->isYesNo;
                    }
                }
                else
                {
                    if(item->eventID == 101)
                    {
                        int resolutionCountTotal = MMInfoGetDisplayNumModes(-1);
                        if(resolutionIDWanted == -2) resolutionIDWanted = resolutionCountTotal-1;
                        if(item == itemSelected && inputJustPressed(KUINPUT_BUTTON_LEFT)) resolutionIDWanted = maxv(resolutionIDWanted-1, 0);
                        if(item == itemSelected && inputJustPressed(KUINPUT_BUTTON_RIGHT)) resolutionIDWanted = minv(resolutionIDWanted+1, resolutionCountTotal-1);
                        resolutionIDWanted = clamp(resolutionIDWanted, 0, resolutionCountTotal-1);
                        if(resolutionIDWanted < 0) resolutionIDWanted = 0;
                        DISPLAYMODE* pMode = MMInfoGetDisplayMode(-1, resolutionIDWanted);
			            if(pMode)
			            {
				            screenSizeWantedX = pMode->Width;
				            screenSizeWantedY = pMode->Height;
                            str_printf(item->strValue, "%dX%d <%d/%d>", pMode->Width, pMode->Height, (resolutionIDWanted+1), resolutionCountTotal);
                            if(item == itemSelected && inputJustPressed(KUINPUT_BUTTON_JUMP)) menuApplyResolution();
                        }
                        else str_cpy(item->strValue, "<ERROR>");
                        /*if(item == itemSelected)
                        {
                            str_cpy(tmpStr, "Press Space/ Enter or (A) on the controller to change resolution.");
                            draw_text_via_txt(tmpStr, cached_fnt, screen_size.x*0.5, screen_size.y*0.2, color, CENTER_X | OUTLINE);
                        }*/
                    }
                    if(item->eventID == 102)
                    {
                        if(item == itemSelected && inputJustPressed(KUINPUT_BUTTON_LEFT)) fpsCapModeActive = maxv(fpsCapModeActive-1, 0);
                        if(item == itemSelected && inputJustPressed(KUINPUT_BUTTON_RIGHT)) fpsCapModeActive = minv(fpsCapModeActive+1, 5);
                        fps_max = fpsCapModes[fpsCapModeActive];
                        str_printf(item->strValue, "<%3d>", (int)fps_max);
                    }
                    if(item->eventID == 103)
                    {
                        if(item == itemSelected && inputJustPressed(KUINPUT_BUTTON_LEFT)) kuSoundMasterVolume = maxv(kuSoundMasterVolume-1, 0);
                        if(item == itemSelected && inputJustPressed(KUINPUT_BUTTON_RIGHT)) kuSoundMasterVolume = minv(kuSoundMasterVolume+1, 10);
                        str_printf(item->strValue, "<%3d>", (int)kuSoundMasterVolume );
                        if(item == itemSelected)
                        {
                            str_cpy(tmpStr, "There is no sound at all in the game currently, sorry!");
                            draw_text_via_txt(tmpStr, cached_fnt, screen_size.x*0.5, 20, COLOR_WHITE, CENTER_X | OUTLINE);
                        }
                    }
                    if(item->eventID == 107)
                    {
                        if(item == itemSelected && inputJustPressed(KUINPUT_BUTTON_LEFT)) backgroundRenderMode = maxv(backgroundRenderMode-1, 0);
                        if(item == itemSelected && inputJustPressed(KUINPUT_BUTTON_RIGHT)) backgroundRenderMode = minv(backgroundRenderMode+1, 3);
                        if(backgroundRenderMode == 0) str_cpy(item->strValue, "<OFF>");
                        if(backgroundRenderMode == 1) str_cpy(item->strValue, "<SIMPLIFIED>");
                        if(backgroundRenderMode == 2) str_cpy(item->strValue, "<DEFAULT>");
                        if(backgroundRenderMode == 3) str_cpy(item->strValue, "<HIGH-RES>");
                    }
                }
                //item->isDirty = 1;
            }
        }
    }
    if(isDirty)
    {
        menuPageUpdateStrings(page);
        page->isDirty = 0;
    }

    ENTITY* ent = page->ent;
    if(!ent)
    {
        if(page->ID == 0) ent = page->ent = ent_for_name("menuPage0");
        if(page->ID == 1)
        {
            ent = page->ent = ent_for_name("menuPage1");
            ent->skill41 = floatd(1, 2);
        }
        ent->skill42 = floatv(menuClosedOnce);
    }
    reset(ent, INVISIBLE);
    vec_fill(ent->scale_x, getSmoothGrow180(menuPagePerc*1.8, 1.35)*2);
    ent_bonereset_all(ent);
    for(i = 0; i < page->numItemsUsed; i++)
    {
        var scale = clamp(getSmoothGrow180(menuPagePerc*2-45*i-100, 1.35), 0, 1.25);
        ent_bonescale(ent, 1+i, vector(scale, scale, scale));
    }

    if(inputJustPressed(KUINPUT_BUTTON_BACKMENU) && page->previousID >= 0)
    {
        menuPagePerc = 0;
        menuManager.pageIDActive = page->previousID;
        MENUPAGE *page2 = &menuManager.pages[menuManager.pageIDActive];
        page2->itemSelected = 0;
    }
}

void menuManagerInit()
{
    //cprintf1("\n menuManagerInit at frame %d", ITF);
    menuManager.isInitialized = 1;
    //cprintf0("\nMENUPAGE_MAIN now...");
    {
        MENUPAGE *page = &menuManager.pages[MENUPAGE_MAIN];
        page->ID = MENUPAGE_MAIN;
        page->previousID = -1;
        page->itemSelected = 1;
    
        MENUITEM *item = menuPageItemAdd(page, "CONTINUE", NULL, MENUITEM_TYPE_LEFT, 2);
        item->isGreyedOut = 1;

        MENUITEM *item = menuPageItemAdd(page, "NEW GAME", NULL, MENUITEM_TYPE_LEFT, 1);

        MENUITEM *item = menuPageItemAdd(page, "OPTIONS", NULL, MENUITEM_TYPE_LEFT, 3);

        MENUITEM *item = menuPageItemAdd(page, "QUIT GAME", NULL, MENUITEM_TYPE_LEFT, 0);

        //cprintf0("\nmenuPageUpdate now...");
        menuPageUpdateStrings(page);
    }
    //return;
    {
        MENUPAGE *page = &menuManager.pages[MENUPAGE_OPTIONS];
        page->ID = MENUPAGE_OPTIONS;
        page->previousID = MENUPAGE_MAIN;
        page->isDirty = 1;
    
        //MENUITEM* menuPageItemAddArrows(MENUPAGE *page, char *cTitle, char *cValue, int type, int eventID, void *pVar, var varMin, var varMax)
        MENUITEM *item = menuPageItemAdd(page, "FULLSCREEN", NULL, MENUITEM_TYPE_ARROWS, 100);
        if(item) item->isYesNo = 1;

        MENUITEM *item = menuPageItemAdd(page, "RESOLUTION", NULL, MENUITEM_TYPE_ARROWS, 101);

        MENUITEM *item = menuPageItemAdd(page, "FPS CAP", NULL, MENUITEM_TYPE_ARROWS, 102);

        MENUITEM *item = menuPageItemAdd(page, "BACKGROUND STYLE", NULL, MENUITEM_TYPE_ARROWS, 107);

        MENUITEM *item = menuPageItemAdd(page, "VOLUME", "< 10>", MENUITEM_TYPE_ARROWS, 103);
        item->isGreyedOut = 1;

        MENUITEM *item = menuPageItemAdd(page, "LEFT-RIGHT INVERTED", "<YES>", MENUITEM_TYPE_ARROWS, 104);
        if(item) item->isYesNo = 0;
        
        //MENUITEM *item = menuPageItemAdd(page, "AMBIENCE", NULL, MENUITEM_TYPE_LEFT, -1);

        MENUITEM *item = menuPageItemAdd(page, "SHOW TIMER", NULL, MENUITEM_TYPE_ARROWS, 105);
        if(item) item->isYesNo = 1;

       // MENUITEM *item = menuPageItemAdd(page, "SHOW SPLITS", NULL, MENUITEM_TYPE_ARROWS, 106);
        //if(item) item->isYesNo = 0;

        MENUITEM *item = menuPageItemAdd(page, "BACK", NULL, MENUITEM_TYPE_LEFT, 4);

        // left right inverted

        //cprintf0("\nmenuPageUpdate now...");
        menuPageUpdateStrings(page);
    }
    //cprintf0("\nmenuManagerInit DONE");
}

/*

Oh Honey, you deserve so much better!
Forget him, he was only rank 6 anyway.
You'll get over him in no time!

*/

void menuUpdate()
{
    //draw_text2("menuUpdate", 20, 20, COLOR_RED);
    if(menuIntroState < 11)
    {
        FONT* font = introFont;
        if(screen_size.x < 1400) font = cached_fnt;
        if(menuIntroState == 0)
        {
            menuIntroStateProgress = minv(menuIntroStateProgress+time_step, 48);
            if(menuIntroStateProgress > 16)
            {
                str_cpy(introStr, "\"Oh Honey, you deserve so much better!\"");
                draw_text_via_txt(introStr, font, screen_size.x*0.5, screen_size.y*0.5-20, COLOR_WHITE, CENTER_X);
             menuOpenPerc = minv(menuOpenPerc+time_step, 100);
            }
            if(menuIntroStateProgress >= 48)
            {
                menuIntroStateProgress = 0;
                menuIntroState = 1;
            }
        }
        if(menuIntroState == 1)
        {
            menuOpenPerc = minv(menuOpenPerc+time_step, 100);
            menuIntroStateProgress = minv(menuIntroStateProgress+time_step, 70);
            if(menuIntroStateProgress > 24)
            {
                str_cpy(introStr, "\"Forget him, he was only rank 6 anyway.\"");
                draw_text_via_txt(introStr, font, screen_size.x*0.5, screen_size.y*0.5-20, vector(60,10,60), CENTER_X | SHADOW);
            }
            if(menuIntroStateProgress >= 70)
            {
                menuIntroStateProgress = 0;
                menuIntroState = 10;
            }
        }
        if(menuIntroState == 10)
        {
            menuOpenPerc = minv(menuOpenPerc+time_step, 100);
            menuIntroStateProgress = minv(menuIntroStateProgress+time_step, 70);
            if(menuIntroStateProgress > 24)
            {
                str_cpy(introStr, "\"You'll get over him in no time!\"");
                draw_text_via_txt(introStr, font, screen_size.x*0.5, screen_size.y*0.5-20, vector(60,10,60), CENTER_X | SHADOW);
            }
            if(menuIntroStateProgress >= 70)
            {
                menuIntroStateProgress = 0;
                menuIntroState = 11;
            }
        }
        //draw_text2(str_printf(NULL,"menuIntroState: %d",(int)menuIntroState),80,80,COLOR_WHITE);
        //draw_text2(str_printf(NULL,"menuIntroStateProgress: %d",(int)menuIntroStateProgress),80,100,COLOR_WHITE);
        //draw_text2(str_printf(NULL,"menuOpenPerc: %d",(int)menuOpenPerc),80,120,COLOR_WHITE);
        if(inputJustPressed(KUINPUT_BUTTON_OPENMENU) || inputJustPressed(KUINPUT_BUTTON_JUMP))
        {
            cprintf1("\n intro skip at frame %d", ITF);
            menuIntroStateProgress = 0;
            menuIntroState = 11;
        }
        return;
    }
    inputDisabled = 0;
    if(menuIntroState == 20)
    {
        inputDisabled = 1;
        menuIntroStateProgress = minv(menuIntroStateProgress+(2+1*(menuIntroStateProgress > 60))*time_step, 100);
        if(menuIntroStateProgress >= 100)
        {
            menuIntroStateProgress = 0;
            menuIntroState = 100;
            menuClose();
            inputDisabled = 0;
        }
    }
    menuOpenPerc = minv(menuOpenPerc+10*time_step, 400);
    menuPagePerc = minv(menuPagePerc+10*time_step, 400);
    menuTitleScreenPerc = minv(menuTitleScreenPerc+10*time_step, 400);
    //if(menuManager.pageIDActive == 0) menuTitleScreenPerc = minv(menuTitleScreenPerc+10*time_step, 400);
    //else menuTitleScreenPerc = maxv(menuTitleScreenPerc-10*time_step, 400);
    if(menuManager.pageIDActive == 0) menuTitleHidePerc = minv(menuTitleHidePerc+10*time_step, 100);
    else menuTitleHidePerc = maxv(menuTitleHidePerc-30*time_step, 0);
    
    //draw_quad(bmpMenuFont,vector(20,40,0),NULL,NULL,NULL,NULL,100,0);
    //draw_quad(bmpMenuStringsEncoded,vector(20,70,0),NULL,NULL,vector(4,4,4),NULL,100,0);
    if(!menuManager.isInitialized) menuManagerInit();
    //if(key_q) menuPageUpdateEnt();
    int i;
    for(i = 0; i < 2; i++)
    {
        MENUPAGE *page = &menuManager.pages[i];
        if(page->ent)
        {
            if(i == menuManager.pageIDActive) reset(page->ent, INVISIBLE);
            else set(page->ent, INVISIBLE);
            page->ent->skill42 = floatv(menuClosedOnce);
        }
    }

    //if(menuIntroState < 100) inputSetButtonsOff();
    menuPageUpdate( &menuManager.pages[menuManager.pageIDActive] );
    if(!menuActive) return;

    //static var menuIdleTimer = 0;
    static var menuSuperkuFallGo = 0;
    static var menuSuperkuFallPerc = 0;
    if(inputIdleTimer > 16*60) menuSuperkuFallGo = 1;
    else menuSuperkuFallGo = menuSuperkuFallPerc = 0;
    if(menuSuperkuFallGo)
    {
        menuSuperkuFallPerc = minv(menuSuperkuFallPerc+ 3* time_step, 100);
    }

    ENTITY* entBackground = menuManager.entBackground;
	#ifdef DEVTRUE
        if(key_e) entBackground.y = camera.y+512;
    #endif
    ENTITY* entTitlescreen = menuManager.entTitlescreen;
    entTitlescreen.emask |= DYNAMIC;
    entTitlescreen.x = entBackground.x;
    entTitlescreen.z = entBackground.z;
    reset(entTitlescreen, INVISIBLE);
    ent_bonereset_all(entTitlescreen);
    float hideFac = pow(1-menuTitleHidePerc*0.01, 4);
    int i;
    for(i = 0; i < 3; i++)
    {
        float scale = clamp(getSmoothGrow180(menuTitleScreenPerc-45*i-45, 1.2), 0, 1.2);
        //scale *= hideFac;
        ent_bonescale(entTitlescreen, 1+i, vector(scale, scale, scale));
        ent_bonemove(entTitlescreen, 1+i, vector(0, 0, 256*hideFac * (1-2*(i >= 2))  ));
    }
    if(menuSuperkuFallPerc > 0)
    {
        ent_bonerotate(entTitlescreen, 2, vector(0, -90*getBounceGrow180(menuSuperkuFallPerc*1.8, 6, 1.1), 0));
    }

    return;
}

////////////////////////////////////////////
