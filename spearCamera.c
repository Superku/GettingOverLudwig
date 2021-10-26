//////////////////////////////
// spearCamera.c
//////////////////////////////

var camera_left = -100; // for shader usage
var camera_right = 100;
var camera_bottom = -100;
var camera_top = 100;

void cameraWidthAdjust(var newWidth)
{
    if(newWidth) cameraWidth = newWidth;
    float fac;
    if(!screen_size.y) fac = video_aspect;
    else fac = screen_size.x/screen_size.y;
    //fac = fclamp(fac, 1, 2);
    #ifdef DEVTRUE
        if(key_o)
        {
            cameraWidth += (key_u-key_i)*50*time_step;
            DEBUG_VAR(cameraWidth, 20);
        }
    #endif
    //var cameraMenuZoom = varSmootherLerp(0, 200, menuOpenPerc*0.01);
    float cameraMenuZoom = pow(0.01*(float)minv(100, menuOpenPerc), 0.5);
    if(menuIntroState == 20) cameraMenuZoom += 2*pow(0.01*(float)menuIntroStateProgress, 2);
    var cameraWidthCombo = cameraWidth - cameraMenuZoom*100; //(100 + 50*!menuClosedOnce);
    float scaleAdjustment = 1+fclamp(1.444-fac,0,1); // we need this so the camera does not zoom out too far on wider monitors
    camera_left = camera.left = -cameraWidthCombo*fac*scaleAdjustment;
    camera_right = camera.right = cameraWidthCombo*fac*scaleAdjustment;
    camera_bottom = camera.bottom = -cameraWidthCombo*scaleAdjustment;
    camera_top = camera.top = cameraWidthCombo*scaleAdjustment;
    camera_left += camera.x;
    camera_right += camera.x;
    camera_bottom += camera.z;
    camera_top += camera.z;
}
    
void cameraMove(int cameraMode)
{
    set(camera, ISOMETRIC);
    camera.y = -4096;
    camera.pan = 90;
    camera.tilt = 0;
    camera.roll = 0;
    camera.clip_near = 64;
	camera.clip_far = 10000;
    cameraWidthAdjust(0);
    if(playerFrozen) return;
    if(!playerDataPointer)
    {
        error("cameraMove: no playerDataPointer!");
        return;
    }
    float xShiftFac = menuActive;
    if(menuIntroState == 20) xShiftFac *= (1-menuIntroStateProgress*0.01);
    var xTarget = playerPosLocal.x+300*xShiftFac + playerDataPointer->speed.x*0.15*!menuActive;
    var zTarget = playerPosLocal.z + playerDataPointer->speed.z*0.2*!menuActive;
    camera.x += (xTarget-camera.x)*0.75*time_step;
	camera.z += (zTarget-camera.z)*0.75*time_step;
    camera.x = clamp(camera.x, -4036, 4130+10000*(camera.z < 4000));

    ////////////////////////

    ENTITY* entBackgroundPlane = entityContainer.entBackgroundPlane;
    if(!entBackgroundPlane)
    {
        collision_mode = 0;
        entBackgroundPlane = ent_create("backgroundPlane.mdl", nullvector, NULL);
        collision_mode = 2; // otherwise the collision hull internally will be recalculated every time scale_ changes - which is slow
        set(entBackgroundPlane, UNLIT | PASSABLE);
        entBackgroundPlane.material = backgroundPlane_mat;
        entityContainer.entBackgroundPlane = entBackgroundPlane;
    }
    if(backgroundRenderMode >= 2)
    {
        reset(entBackgroundPlane, INVISIBLE);
        entBackgroundPlane.x = camera.x;
        entBackgroundPlane.y = 4096;
        entBackgroundPlane.z = camera.z;
        entBackgroundPlane.scale_x = camera.right/256.0*1.001;
		entBackgroundPlane.scale_z = camera.top/256.0*1.001;
    }
    else
    {
        set(entBackgroundPlane, INVISIBLE);
    }

    ENTITY* entParticlesFG = entityContainer.entParticlesFG;
    if(!entParticlesFG)
    {
        /*collision_mode = 0; FAILED attempt, rotating individual FG flakes, particles, leaves
        entParticlesFG = ent_create("particlesFG.mdl", nullvector, NULL);
        collision_mode = 2;
        set(entParticlesFG, UNLIT | PASSABLE);
        entParticlesFG.material = particlesFG_mat;
        entityContainer.entParticlesFG = entParticlesFG;*/
        collision_mode = 0;
        entParticlesFG = ent_create("particlesFGquad.mdl", nullvector, NULL);
        collision_mode = 2;
        set(entParticlesFG, UNLIT | PASSABLE);
        entParticlesFG.material = particlesFGquad_mat;
        entityContainer.entParticlesFG = entParticlesFG;
    }
    entParticlesFG.x = camera.x;
    entParticlesFG.y = -1184;
    entParticlesFG.z = camera.z;
    entParticlesFG.scale_x = camera.right/256.0*1.001;
    entParticlesFG.scale_z = camera.top/256.0*1.001;

    int i;
    for(i = 0; i < entityContainer.entBackgroundObjectCount; i++)
    {
	    ENTITY* ent = entityContainer.entBackgroundObjects[i];
        ent.x = ent.skill60 + (camera.x-ent.skill60)*ent.skill1;
        ent.z = ent.skill62 + (camera.z-ent.skill62)*ent.skill2;
    }
}

void backgroundObjectAdd(ENTITY* ent)
{
	if(!ent) return;
	if(entityContainer.entBackgroundObjectCount < BACKGROUND_OBJECTS_MAX) entityContainer.entBackgroundObjects[entityContainer.entBackgroundObjectCount++] = ent;
}



void spearUIDo()
{
    FONT *font = cached_fnt;
	var drawPosX = 14;
	var drawPosY = 14;
	var drawPosYStepSize = font->dy;
	if(showTimer) //showFPS)
	{
    	static float fpsSmoothed = 60;
        fpsSmoothed = 0.9*fpsSmoothed+0.1/time_frame;
        if(fpsSmoothed > fps_max ) fpsSmoothed = fps_max;
        str_printf(tmpStr, "FPS: %d", (int)(fpsSmoothed*16+0.1));
   		draw_text2cached(tmpStr, font, drawPosX, drawPosY, COLOR_WHITE, CACHED_TXT_FPS);
        drawPosY += drawPosYStepSize;
	}
   	if(showTimer)
	{
        str_cpy(tmpStr, "Time: ");
        strForTime(tmpStr2, gameData.timeCombined, 1);
        str_cat(tmpStr, tmpStr2);
   		draw_text2cached(tmpStr, font, drawPosX, drawPosY, COLOR_WHITE, CACHED_TXT_TIMER);
        drawPosY += drawPosYStepSize;
	}
}

void outroDo()
{
    if(outroState < 1) return;
    FONT* font = introFont;
    if(screen_size.x < 1400) font = cached_fnt;
    if(outroState == 1)
    {
        outroStateProgress = minv(outroStateProgress+time_step, 48);
        if(outroStateProgress > 0)
        {
            str_cpy(tmpStr, "TIME: ");
            strForTime(tmpStr2, gameData.timeCombined, 1);
            str_cat(tmpStr, tmpStr2);
            draw_text_via_txt(tmpStr, font, screen_size.x*0.5, screen_size.y*0.5-20, COLOR_WHITE, CENTER_X);
        }
        if(outroStateProgress >= 48)
        {
            outroStateProgress = 0;
            outroState = 2;
        }
    }
    if( outroState >= 2)
    {
        ENTITY* entBlackPlane = menuManager.entBlackPlane;
        if(entBlackPlane)
        {
            entBlackPlane.x = camera.x;
            entBlackPlane.z = camera.z;
            entBlackPlane.emask |= DYNAMIC;
            reset(entBlackPlane, INVISIBLE);
        }
    }
    if(outroState == 2)
    {
        outroStateProgress = minv(outroStateProgress+time_step, 60);
        if(outroStateProgress > 16)
        {
            str_cpy(introStr, "Congratulations, you did it!");
            draw_text_via_txt(introStr, font, screen_size.x*0.5, screen_size.y*0.5-20, COLOR_WHITE, CENTER_X);
        }
        if(outroStateProgress >= 60)
        {
            outroStateProgress = 0;
            outroState = 3;
        }
    }
    if(outroState == 3)
    {
        outroStateProgress = minv(outroStateProgress+time_step, 108);
        if(outroStateProgress > 16)
        {
            str_cpy(introStr, "Imagine an outro where you, \"Honey\",\ngo spear-first for Ludwig's Heart.");
            draw_text_via_txt(introStr, font, screen_size.x*0.5, screen_size.y*0.5-45, COLOR_WHITE, CENTER_X);
        }
        if(outroStateProgress >= 108)
        {
            outroStateProgress = 0;
            outroState = 0;
            menuClosedOnce = 0;
            mapReset();
            menuIntroState = 11;
            menuIntroStateProgress = 0;
            //menuOpenPerc = 0;
            menuPagePerc = 0;
            menuOpen();
        }
    }
}
