//////////////////////////////
// spearPlayer.c
//////////////////////////////

var gravityAngle = 0;
// in early days of prototyping the spear would set itself upright automatically on wall contact,
// meaning it would rotate itself to a 90° angle perpendicular to the wall surface normal
// the game was super easy then.
// logic of this remains in the code here and there
VECTOR rotationPrevBone1Pos, rotationPrevBone1Pos2;
typedef struct _SPEARPLAYER
{
    // pos is based on ENTITY position (because we use the engine's native collision detection function to move it)
	ENTITY* entContact;
    VECTOR speed;
    VECTOR contactPos;
	VECTOR surfaceNormal;
	var surfaceAngle;
    var turnSpeed, slowTiltTimer;
    var angle;
    var groundContact, contactTime, groundContactHandled;
    var boostReady, boostDuration, boostProgress, boostTimer;
	var justLaunchedTimer;
	var loadPower;
	var spearAnimDo;
	var moveTimer;
} SPEARPLAYER;
SPEARPLAYER *playerDataPointer = NULL;

#define BACKGROUND_OBJECTS_MAX 2
typedef struct _ENTITYCONTAINER
{
    ENTITY* entDummy; // this is the main object we control
    ENTITY* entSpear;
    ENTITY* entSpear2;
    ENTITY* entCharacter; // this is decoration
    ENTITY* entSpearWoosh;
	ENTITY* entBackgroundPlane;
	ENTITY* entThickHullTrace;
	ENTITY* editorBrushEnt;
	ENTITY* entParticlesFG;
	ENTITY* entHoneyCombo;
	ENTITY* entBackgroundObjects[2];
	int entBackgroundObjectCount;
} ENTITYCONTAINER;
ENTITYCONTAINER entityContainer;

typedef struct _SPEARGAME
{
    SPEARPLAYER playerData;
    VECTOR spawnPos;
    int timeInteger;
    double timeFloat;
    var timeCombined;
    int numJumps, numBoosts;
} SPEARGAME;
SPEARGAME gameData;  // all data that needs to be reset when hitting the reset/ retry key



void mapReset()
{
    memset(gameData, 0, sizeof(SPEARGAME));
	ENTITY* entSpawn = ent_for_name("spawn");
	if(!entSpawn) error("mapReset: spawn position not found, crash!");
    vec_set(&gameData.spawnPos, &entSpawn->x);

    playerDataPointer = &gameData.playerData;
	if(entityContainer.entDummy) vec_set(entityContainer.entDummy.x, &gameData.spawnPos);
    playerDataPointer->angle = 0;
    playerDataPointer->spearAnimDo = 1;
    playerDataPointer->justLaunchedTimer = 2;
    playerDataPointer->slowTiltTimer = 12;
    playerDataPointer->boostProgress = 180;
	playerDataPointer->speed.z = 45;
	vec_set(&playerPosLocal, &gameData.spawnPos);
	camera.x = playerPosLocal.x;
	camera.z = playerPosLocal.z;
	outroState = 0;
	outroStateProgress = 0;
	if(entityContainer.entHoneyCombo) vec_set(entityContainer.entHoneyCombo.pan, nullvector);
}

void spearPlayerInit()
{
    playerDataPointer = &gameData.playerData;
	/*if(!playerSpawnPosition.x) I already coded in a gameData.spawnPos...
	{
		ENTITY* entSpawn = ent_for_name("spawn");
		if(!entSpawn) error("spearPlayerInit: spawn position not found, crash!");
		vec_set(&playerSpawnPosition, &entSpawn->x);
	}*/
	//printf("%d", (int)gameData.spawnPos.x);
	if(!entityContainer.entDummy)
    {
        ENTITY* entDummy = ent_create(CUBE_MDL, gameData.spawnPos, NULL);
        set(entDummy, UNLIT | INVISIBLE);
        entityContainer.entDummy = entDummy;
    }
    if(!entityContainer.entSpear)
    {
        ENTITY* entSpear = ent_create("spearTest3.mdl", gameData.spawnPos, NULL);
        set(entSpear, UNLIT | PASSABLE);
        //entSpear->material = ...;
        entityContainer.entSpear = entSpear;
    }
	if(!entityContainer.entSpear2)
    {
        ENTITY* entSpear2 = ent_create("spearTest4.mdl", gameData.spawnPos, NULL);
        set(entSpear2, UNLIT | PASSABLE);
        entityContainer.entSpear2 = entSpear2;
    }
    if(!entityContainer.entCharacter)
    {
        ENTITY* entCharacter = ent_create(CUBE_MDL, gameData.spawnPos, NULL);
        set(entCharacter, UNLIT | PASSABLE);
        entityContainer.entCharacter = entCharacter;
    }
    if(!entityContainer.entSpearWoosh)
    {
        ENTITY* entSpearWoosh = ent_create(CUBE_MDL, gameData.spawnPos, NULL);
        set(entSpearWoosh, UNLIT | PASSABLE | INVISIBLE);
        entityContainer.entSpearWoosh = entSpearWoosh;
    }
	if(!entityContainer.entHoneyCombo)
    {
        ENTITY* entHoneyCombo = ent_create("honey.mdl", gameData.spawnPos, NULL);
        set(entHoneyCombo, UNLIT | PASSABLE);
        entHoneyCombo->material = characterHoney_mat;
        entityContainer.entHoneyCombo = entHoneyCombo;
    }
	
}

void debugArrowAction()
{
	my->skill1 = 48;
	while(my->skill1 > 0)
	{
		my->skill1 -= time_step;
		wait(1);
	}
	ptr_remove(me);
}

void debugArrow(VECTOR* vPos, VECTOR* vSpeed, COLOR* color)
{
	ENTITY* ent = ent_create("debugArrow.mdl", vPos, debugArrowAction);
	set(ent, PASSABLE | UNLIT | LIGHT);
	ent->tilt = atan2v(vSpeed->z, vSpeed->x);
	ent->ambient = 75;
	ent->scale_x = vec_length(vSpeed)/96.0+0.2;
	vec_set(ent->blue, color);
}

// this helper function was copied over from pogostuck, a raycast with thickness, independent of the caller's collision hull if any
var thickHullTrace(ENTITY* ent, VECTOR* vstart, VECTOR* vtarget, var mode, var radius)
{
	if(!entityContainer.entThickHullTrace) entityContainer.entThickHullTrace = ent_create(CUBE_MDL, nullvector, NULL);
	set(entityContainer.entThickHullTrace, INVISIBLE);
	reset(entityContainer.entThickHullTrace, PASSABLE);
	entityContainer.entThickHullTrace->eflags |= NARROW | FAT;
	vec_set(entityContainer.entThickHullTrace->x, vstart); // probably unnecessary
	if(radius >= 0)
	{
		vec_set(entityContainer.entThickHullTrace->pan, nullvector);
		vec_fill(entityContainer.entThickHullTrace->min_x, -radius);
		vec_fill(entityContainer.entThickHullTrace->max_x, radius);
	}
	if((mode & IGNORE_ME) && ent) set(ent,PASSABLE);
	var prevPushValue = entityContainer.entThickHullTrace->push;
	if(ent) entityContainer.entThickHullTrace->push = ent->push;
	ENTITY* old_me = me;
	me = entityContainer.entThickHullTrace;
	var dist = c_trace(vstart, vtarget, mode | USE_BOX | IGNORE_ME);
	me = old_me;
	entityContainer.entThickHullTrace->push = prevPushValue;
	if((mode & IGNORE_ME) && ent) reset(ent,PASSABLE);
	set(entityContainer.entThickHullTrace,PASSABLE);

	return dist;
}

void spearUIDo();

void playerMove()
{
    playerDataPointer = &gameData.playerData;
    if(!playerDataPointer)
    {
        error("playerMove: !playerDataPointer");
        return;
    }
    ENTITY* entDummy = entityContainer.entDummy;
    ENTITY* entSpear = entityContainer.entSpear;
    ENTITY* entSpear2 = entityContainer.entSpear2;
    ENTITY* entCharacter = entityContainer.entCharacter;
    ENTITY* entSpearWoosh = entityContainer.entSpearWoosh; // remove unused objects some day... some other day
    ENTITY* entHoneyCombo = entityContainer.entHoneyCombo;
    if(!entSpear || !entDummy)
    {
		error("playerMove: !entSpear");
        return;
    }
	#ifdef DEVTRUE
		if(key_t || ackXInputGetTriggerState3(1))
		{
			float fac = 1;
			if(key_t)
			{
				entDummy.x += (key_d-key_a)*320*fac*time_step;
				entDummy.z += (key_w-key_s)*320*fac*time_step;
			}
			else
			{
				var thumbLX = ackXInputGetThumbState3(0, 0);
				var thumbLY = ackXInputGetThumbState3(0, 1);
				entDummy.x += thumbLX*1.5*fac*time_step;
				entDummy.z += thumbLY*1.5*fac*time_step;
			}
			vec_set(entSpear2->x, entDummy.x);
			camera.x = entDummy.x;
			camera.z = entDummy.z;
			rotationPrevBone1Pos.x = entDummy.x;
			rotationPrevBone1Pos.z = entDummy.z;
			playerDataPointer->speed.x = 0;
			playerDataPointer->speed.z = 0;
			return;
		}
	#endif

	entSpear2.y = entSpear2.skill77;
	entCharacter.y = entCharacter.skill77;
	entSpearWoosh.y = entSpearWoosh.skill77;
	entHoneyCombo.y = entHoneyCombo.skill77;
	if(menuActive)
	{
		var yShift = -1800;
		if(menuIntroState < 100)
		{
			if(menuIntroState < 10) yShift = 0;
			else
			{
				if(menuIntroState < 11) yShift = -1500-300*menuIntroStateProgress/70.0;
			}
		}
		entSpear2.y += yShift;
		entCharacter.y += yShift;
		entSpearWoosh.y += yShift;
		entHoneyCombo.y += yShift;

		if(menuIntroState < 100)
		{
			entHoneyCombo.x = entDummy.x;
			entHoneyCombo.z = entDummy.z;
			var animPerc = 0;
			if(menuIntroState == 20) animPerc = menuIntroStateProgress;
			ent_animate(entHoneyCombo, "intro", animPerc, 0);
			entHoneyCombo.pan = -25;
			entHoneyCombo.tilt = 0;
			entHoneyCombo.roll = 0;
		}
		else
		{
			entHoneyCombo.pan = 0;
			ent_animate(entHoneyCombo, "loadLeft", 0, 0);
		}
		set(entSpear,INVISIBLE);
		set(entCharacter,INVISIBLE);
		set(entSpear2,INVISIBLE);

	#ifdef DEVTRUE
        if(key_e) entHoneyCombo.y = camera.y+512-256; // screenshot
    #endif
		return;
	}
    if(playerFrozen || menuIntroState < 100) return;

	VECTOR temp,temp2,temp3,temp4,temp5;
	entDummy->min_x = -13;
	entDummy->max_x = 13;
	entDummy->min_y = -8;
	entDummy->max_y = 8;
	entDummy->min_z = -108;
	entDummy->max_z = 54;
	
	float turnFac = 1.0/(1.0+2*!!playerDataPointer->groundContact);
	int leftRightInverted = 1-2*(playerLeftRightInverted == 1);
	/* this was the old behavior up until 26.10.2021, 00:10am:
	playerDataPointer->turnSpeed += ((inputGetValue(KUINPUT_BUTTON_LEFT)-inputGetValue(KUINPUT_BUTTON_RIGHT))*20*leftRightInverted-playerDataPointer->turnSpeed)*0.525/(1.0+playerDataPointer->slowTiltTimer)*time_step; //*turnFac //thumbLX*0.125*/
	playerDataPointer->turnSpeed += ((inputGetValue(KUINPUT_BUTTON_LEFT)-inputGetValue(KUINPUT_BUTTON_RIGHT))*22*leftRightInverted-playerDataPointer->turnSpeed)*0.55/(1.0+playerDataPointer->slowTiltTimer)*time_step; //*turnFac //thumbLX*0.125
	//playerDataPointer->turnSpeed += clamp(((inputGetValue(KUINPUT_BUTTON_LEFT)-inputGetValue(KUINPUT_BUTTON_RIGHT))*20-playerDataPointer->turnSpeed)*0.525,-5,5)/(1.0+playerDataPointer->slowTiltTimer)*time_step;
	playerDataPointer->slowTiltTimer = maxv(playerDataPointer->slowTiltTimer-time_step,0);

	/*draw_text2(str_printf(NULL,"playerDataPointer->surfaceAngle: %d",(int)playerDataPointer->surfaceAngle),80,80,COLOR_WHITE);
	draw_text2(str_printf(NULL,"playerDataPointer->angle: %d",(int)playerDataPointer->angle),80,100,COLOR_WHITE);
	draw_text2(str_printf(NULL,"playerDataPointer->groundContact: %d",(int)playerDataPointer->groundContact),100,120,COLOR_WHITE);
	draw_text2(str_printf(NULL,"playerDataPointer->turnSpeed: %.1f",(double)playerDataPointer->turnSpeed),100,140,COLOR_WHITE);
	draw_text2(str_printf(NULL,"playerDataPointer->slowTiltTimer: %.1f",(double)playerDataPointer->slowTiltTimer),100,160,COLOR_WHITE);
	draw_text2(str_printf(NULL,"playerDataPointer->loadPower: %.1f",(double)playerDataPointer->loadPower),100,180,COLOR_WHITE);
	if(playerDataPointer->entContact) draw_text2(str_printf(NULL,"playerDataPointer->entContact: %s",_chr(str_for_entname(NULL,playerDataPointer->entContact))),100,200,COLOR_WHITE);
	else draw_text2("playerDataPointer->entContact == NULL!",100,200,COLOR_WHITE);
	draw_text2(str_printf(NULL,"playerDataPointer->justLaunchedTimer: %d",(int)playerDataPointer->justLaunchedTimer),100,240,COLOR_WHITE);*/

	vec_for_bone(rotationPrevBone1Pos,entSpear,"Bone1");
	//draw_point4d(rotationPrevBone1Pos,COLOR_GREEN,100,24);
	int useGlide = 0;
	if(playerDataPointer->justLaunchedTimer > 0)
	{
		playerDataPointer->groundContact = 0;
		useGlide = GLIDE;
	}
	playerDataPointer->justLaunchedTimer = maxv(playerDataPointer->justLaunchedTimer-time_step,0);
	vec_set(entDummy.pan,vector(0,playerDataPointer->angle,0));
	var rotResult = 0;
	if(!playerDataPointer->groundContact)
	{
		rotResult = c_rotate(entDummy, vector(0,turnFac*playerDataPointer->turnSpeed*time_step,0), IGNORE_PASSABLE | IGNORE_SPRITES | IGNORE_PUSH | GLIDE); //
		playerDataPointer->loadPower += -playerDataPointer->loadPower*0.8*time_step;
	}
	else
	{
		//var k = ang(playerDataPointer->surfaceAngle-90-entDummy.tilt)*0.5*time_step;
		var angDiff = ang(playerDataPointer->surfaceAngle-90-entDummy.tilt);
		var k = angDiff*0.5*(abs(angDiff) > 430)*time_step;
		rotResult = c_rotate(entDummy, vector(0,k,0), IGNORE_PASSABLE | IGNORE_SPRITES | IGNORE_PUSH | GLIDE); //
		
		//playerDataPointer->loadPower += clamp((-(inputGetValue(KUINPUT_BUTTON_LEFT)-inputGetValue(KUINPUT_BUTTON_RIGHT))*32-playerDataPointer->loadPower)*0.5,-5,5)*time_step;
		var maxSpeed = 5 - 0*2 * (inputTypeActive == 0); // load up slower on keyboard - or not!
		playerDataPointer->loadPower += clamp((-(inputGetValue(KUINPUT_BUTTON_LEFT)-inputGetValue(KUINPUT_BUTTON_RIGHT))*32*leftRightInverted-playerDataPointer->loadPower)*0.5,-maxSpeed,maxSpeed)*time_step;

		if(abs(playerDataPointer->loadPower) > 0.5) // emulate some sort of collision detection for loading up the spear
		{
			var angle = -playerDataPointer->loadPower*2.75; //*3.1415;
			vec_set(temp,vector(0,0,93+8));
			vec_rotate(temp,vector(0,playerDataPointer->angle,0));
			vec_add(temp,entDummy.x);
			vec_set(temp2,vector(-sinv(angle)*78,0,cosv(angle*0.999)*93+8)); // was 86 for x
			vec_rotate(temp2,vector(0,playerDataPointer->angle,0));
			vec_add(temp2,entDummy.x);
			vec_diff(temp3,temp2,temp);
			vec_normalize(temp3,maxv(vec_length(temp3),30)*2.1);
			vec_set(temp2,temp);
			vec_add(temp2,temp3);
			//draw_line4d2(temp, temp2, COLOR_WHITE, 100);
			var dist = thickHullTrace(entDummy, temp, temp2, IGNORE_PASSABLE | IGNORE_SPRITES | IGNORE_PUSH | IGNORE_ME | USE_BOX | IGNORE_FLAG2, 24);
			if(trace_hit)
			{
				//draw_point4d(target,vector(255,0,255),100,14);
				//var maxPower = maxv(0.5,pow(dist,0.85)*0.285); //maxv(dist-20,0)*0.3;
				var maxPower = maxv(0.5,maxv(dist-15,0)*0.3);
				//draw_text2(str_printf(NULL,"maxPower: %d",(int)maxPower),700,340,COLOR_WHITE);
				//draw_text2(str_printf(NULL,"playerDataPointer->loadPower: %d",(int)playerDataPointer->loadPower),700,360,COLOR_WHITE);
				playerDataPointer->loadPower = clamp(playerDataPointer->loadPower,-maxPower,maxPower);
			}
		}
	}
	playerDataPointer->angle = entDummy.tilt;
	vec_set(entSpear.x,entDummy.x);
	vec_set(entSpear.pan,entDummy.pan);
	
	if(playerDataPointer->groundContact)
	{
		if(!playerDataPointer->justLaunchedTimer)
		{
			vec_for_bone(temp2,entSpear,"Bone1");
			vec_diff(temp3,rotationPrevBone1Pos,temp2); // cheap hack to correct the position after non center rotation
			if(vec_length(temp3) < 128)
			{
				temp3.y = 0;
				vec_add(entDummy->x,temp3);
				playerDataPointer->speed.x = 0;
				playerDataPointer->speed.z = 0;
			}
		}
	}
	else
	{
		vec_set(normal,vector(1,0,0));
		vec_rotate(normal,vector(0,gravityAngle,0));
		var dotValue = vec_dot(normal, playerDataPointer->speed) ;
		vec_set(temp,vector(-0.04*dotValue*time_step,0,-8.5*time_step)); // was: -7.5
		vec_rotate(temp,vector(0,gravityAngle,0));
		playerDataPointer->speed.x += temp.x;
		playerDataPointer->speed.z += temp.z;
		
		/*if(!playerDataPointer->boostTimer && inputJustPressed(KUINPUT_BUTTON_BOOST)) new game plus mode
		{
			vec_set(temp,vector(0,0,-42));
			vec_rotate(temp,vector(0,playerDataPointer->angle,0));
			playerDataPointer->speed.x += temp.x;
			playerDataPointer->speed.z += temp.z;
			playerDataPointer->boostTimer = 9999;
			vec_normalize(temp,1);
			playerDataPointer->moveTimer = 8;
			//playerDataPointer->boostProgress = 100;
		}
		else
		{
			if(!inputIsDown(KUINPUT_BUTTON_BOOST) || !playerDataPointer->moveTimer) playerDataPointer->moveTimer = 0;
			else
			{
				vec_set(temp,vector(0,0,-sqrt(playerDataPointer->moveTimer)*2*time_step));
				vec_rotate(temp,vector(0,playerDataPointer->angle,0));
				playerDataPointer->speed.x += temp.x;
				playerDataPointer->speed.z += temp.z;
			}
		}
		playerDataPointer->moveTimer = maxv(playerDataPointer->moveTimer-time_step,0);*/
	}
	if(vec_length(playerDataPointer->speed) > 280) vec_normalize(playerDataPointer->speed,280);

	var oldGroundContact = playerDataPointer->groundContact;
	//playerDataPointer->groundContact = 0;
	move_friction = 1;
	
	var c_move_HIT_TARGET = 1; //HIT_TARGET;
	if(!playerDataPointer->groundContact)
	{
		c_move(entDummy, nullvector, vector(playerDataPointer->speed.x*time_step,0,playerDataPointer->speed.z*time_step), IGNORE_PASSABLE | IGNORE_SPRITES | IGNORE_PUSH | useGlide); //   | GLIDE
		c_move_HIT_TARGET = HIT_TARGET;
	}
	vec_set(entSpear.x,entDummy.x);
	vec_set(entSpear.pan,entDummy.pan);
	VECTOR oldNormal,oldTarget;
	vec_set(oldNormal,normal);
	vec_set(oldTarget,target);

	VECTOR springPos;
	vec_for_bone(temp2,entSpear,"Bone1");
	vec_set(springPos,temp2);
	vec_for_bone(temp,entSpear,"Bone2");
	vec_diff(temp3,temp2,temp);
	vec_scale(temp3,1.25);
	vec_add(temp3,temp2);
	//draw_line4d2(temp, temp3, COLOR_WHITE, 100);
	if(playerDataPointer->justLaunchedTimer > 0)
	{
		playerDataPointer->groundContact = 0;
		playerDataPointer->entContact = NULL;
	}
	else
	{
		thickHullTrace(entDummy, temp, temp3, IGNORE_PASSABLE | IGNORE_SPRITES | IGNORE_PUSH | IGNORE_ME | IGNORE_FLAG2, 2);
		playerDataPointer->entContact = you;
		playerDataPointer->surfaceAngle = atan2v(normal.z,normal.x);
		if(c_move_HIT_TARGET && trace_hit)
		{
			var angDiff = ang(playerDataPointer->surfaceAngle-90-entDummy.tilt);
			if(abs(angDiff) < 60) playerDataPointer->groundContact = 1; // was 58 up until 26.10.2021, 0:12am - makes a big difference surprisingly
			else
			{
				effect(p_spearImpactStreaks, 12, target, normal);
				effect(p_spearImpactGlow, 1, target, vector(1,1,0));
			}
		}
		if(!trace_hit) playerDataPointer->groundContact = 0;
	}
	vec_set(normal,oldNormal);
	vec_set(target,oldTarget);
	VECTOR normalNotRotated;
	vec_set(normalNotRotated,normal);
	vec_rotate(normal,vector(0,gravityAngle,0)); // gravityAngle was completely pointless

	vec_set(temp2,vector(0,0,72));
	vec_rotate(temp2,vector(0,playerDataPointer->angle,0));
	vec_add(temp2,entDummy->x);
	//draw_line4d2(entCharacter.x, temp2, vector(255,255,0), 100);
	static var p_characterHitStars_cooldown = 0;
	if(!playerDataPointer->groundContact) playerDataPointer->groundContactHandled = 0;
	if(c_move_HIT_TARGET)
	{
		if(playerDataPointer->groundContact && !playerDataPointer->justLaunchedTimer)
		{
			if(!inputJustPressed(KUINPUT_BUTTON_JUMP) ) // use recently released?
			{
				if(!playerDataPointer->groundContactHandled)
				{
					playerDataPointer->groundContactHandled = 1;
					effect(p_spearImpactGlow, 1, target, vector(1,1,0));
					effect(p_spearImpactSmoke, 6, target, NULL);
					// no sparks? <- no, they signal a bad impact... hm!
					int num = 1+random(4);
					if(num > 0) effect(p_spearImpactStreaks, num, target, normal); // maybe after all...
				}
			}
			else
			{
				playerDataPointer->groundContactHandled = 0;
				playerDataPointer->angle = ang(playerDataPointer->angle);
				if(1)
				{
					vec_set(temp2,vector(0,0,72));
					vec_rotate(temp2,vector(0,playerDataPointer->angle-playerDataPointer->loadPower*1.25,0));
					vec_add(temp2,entDummy->x);
					//draw_point4d(temp2,vector(255,0,255),100,14);
					vec_diff(temp,temp2,entCharacter.x);
					vec_normalize(temp,clamp(vec_length(temp2)*1.2,20,70) * pow(abs(playerDataPointer->loadPower)/32.0,1.5) + 20 ); // *pow(abs(playerDataPointer->loadPower)/32.0,0.75)
					if(0)
					{
						vec_set(normal,vector(1,0,0));
						vec_rotate(normal,vector(0,playerDataPointer->surfaceAngle,0));
						var dotValue = vec_dot(temp,normal);
						VECTOR normalScaled;
						vec_set(normalScaled, normal);
						vec_scale(normalScaled, 128);
						if(dotValue < 0)
						{
							debugArrow(entDummy->x, normalScaled, COLOR_BLUE);
							vec_scale(normal,-1.5*dotValue);
							vec_add(temp,normal);
						}
						else debugArrow(entDummy->x, normalScaled, COLOR_GREEN);
					}
					//debugArrow(entDummy->x, temp, COLOR_RED);
					//vec_rotate(temp, vector(0,  -cosv(playerDataPointer->surfaceAngle)*30, 0));
					//debugArrow(entDummy->x, temp, COLOR_GREEN);
					playerDataPointer->speed.x = temp.x;
					playerDataPointer->speed.z = temp.z+5+5*(normalNotRotated.z > -0.45); // 1+15*pow(maxv(temp.z,0)*0.01,0.35);
				}
				else
				{
					vec_set(temp,vector(-pow(abs(playerDataPointer->loadPower)/32.0,0.75) * 75*sign(playerDataPointer->loadPower) ,0, 10*(normalNotRotated.z > -0.45)));
					vec_rotate(temp,vector(0,playerDataPointer->angle+playerDataPointer->loadPower*0,0));
					debugArrow(entDummy->x, temp, COLOR_RED);
					vec_rotate(temp, vector(0,  -playerDataPointer->surfaceAngle*0, 0));
					debugArrow(entDummy->x, temp, COLOR_GREEN);
					//vec_normalize(temp, pow(abs(playerDataPointer->loadPower)/32.0,0.75) );
					playerDataPointer->speed.x = temp.x;
					playerDataPointer->speed.z = temp.z+10*(normalNotRotated.z > -0.45); // 15*pow(maxv(temp.z,0)*0.01,0.35);
				}
				//cprintf2("\nspear2 jump: (%.1f, %.1f)",(double)temp.x,(double)temp.z);
				//playerDataPointer->turnSpeed += clamp(ang(playerDataPointer->angle-gravityAngle),-40,40)*0.3-asinv(normal.x)*0.1;
				//playerDataPointer->turnSpeed *= -0.6;
				playerDataPointer->groundContact = 0;
				playerDataPointer->spearAnimDo = 1;
				playerDataPointer->justLaunchedTimer = 2;
				playerDataPointer->slowTiltTimer = 5.75; // was: 10
				playerDataPointer->boostProgress = 180;
				playerDataPointer->turnSpeed = clamp(playerDataPointer->turnSpeed*1.25 - asinv(normal.x)*0.125, -30, 30);
			}
		}
		else
		{
			int justLaunched = (playerDataPointer->justLaunchedTimer > 0);
			if(normal.z > 0.7 && playerDataPointer->speed.z < 0)
			{
				var len = vec_length(vector(playerDataPointer->speed.x,0,playerDataPointer->speed.z));
				normalNotRotated.y = 0;
				vec_set(bounce,normalNotRotated);
				vec_normalize(bounce,maxv(30,len*(1-0.5*!justLaunched)) );
				playerDataPointer->speed.x = bounce.x;
				playerDataPointer->speed.z = bounce.z;
				playerDataPointer->turnSpeed = ang(gravityAngle-playerDataPointer->angle)*0.35-asinv(normal.x)*0.2; //-sign(normal.x)*20
			}
			else
			{
				bounce.y = 0;
				var len = vec_length(vector(playerDataPointer->speed.x,0,playerDataPointer->speed.z));
				/*if(len < 50)
				{
					len = 50;
					normal.y = 0;
					vec_set(bounce,normal);
				}*/
				vec_normalize(bounce,len*(1-0.3*!justLaunched));
				playerDataPointer->turnSpeed = -sign(normal.x)*10; //-asinv(normal.x)*0.3; //
				playerDataPointer->speed.x = bounce.x;
				playerDataPointer->speed.z = bounce.z;
				playerDataPointer->slowTiltTimer = minv(playerDataPointer->slowTiltTimer+6,10);
			}
			if(p_characterHitStars_cooldown <= 0)
			{
				effect(p_characterHitStars, 10, target, normal);
				effect(p_spearImpactGlow, 1, target, vector(1,2,0)); // 2nd vel parameter is alpha fac
				p_characterHitStars_cooldown = 5;
			}
		}
	}
	p_characterHitStars_cooldown = maxv(p_characterHitStars_cooldown-time_step, 0);
	entDummy->min_z = -75;
	entDummy->y = 0;

	if(playerDataPointer->groundContact) vec_for_bone(rotationPrevBone1Pos,entSpear,"Bone1");
	VECTOR bone1Pos2;
	vec_for_bone(bone1Pos2,entSpear,"Bone1");
	vec_set(rotationPrevBone1Pos2,bone1Pos2);
	//draw_num3d(playerDataPointer->angle,entDummy.x,-100,COLOR_WHITE);
	//draw_num3d(playerDataPointer->loadPower,entDummy.x,100,COLOR_WHITE);
	entSpear.skill42 = floatd(playerDataPointer->loadPower,16);
	vec_set(entCharacter.pan,entSpear.pan);
	var angle = -playerDataPointer->loadPower*2.25; //*3.1415;
	ang_add(entCharacter.pan,vector(0,angle+180,0));

	if(playerDataPointer->groundContact) playerDataPointer->spearAnimDo = 0;
	
	if(playerDataPointer->groundContact || playerDataPointer->spearAnimDo) entSpear2.skill10 += (16-entSpear2.skill10)*0.5*time_step;
	else entSpear2.skill10 += (0-entSpear2.skill10)*time_step;
	if(playerDataPointer->moveTimer && !playerDataPointer->groundContact)
	{
		playerDataPointer->boostProgress += (0-playerDataPointer->boostProgress)*0.5*time_step;
		entSpear2.skill11 += (16-entSpear2.skill11)*time_step;
		playerDataPointer->spearAnimDo = 0;
	}
	else
	{
		playerDataPointer->boostProgress = maxv(playerDataPointer->boostProgress-32*time_step,90*playerDataPointer->spearAnimDo);
		entSpear2.skill11 += (0-entSpear2.skill11)*0.35*time_step;
	}
	ent_bonereset_all(entSpear2);
	float fac = 1+pow(entSpear2.skill11/16.0,2)*0.25;
	ent_bonescale(entSpear2,"Bone1",vector(fac,fac,fac));
	fac = 1+entSpear2.skill11/32.0;
	ent_bonescale(entSpear2,"Bone2",vector(fac,fac,fac));
	
	entSpear.skill41 = floatv(sinv(total_ticks*10));
	entSpear2.skill41 = floatv(fixv(entSpear.skill41)+entSpear2.skill11*0.1);
	entSpear2.skill42 = floatv(fixv(entSpear.skill42));
	entSpear2.skill43 = floatd(entSpear2.skill10,16);
	entSpear2.skill44 = floatv(fixv(entSpear.skill44));
	entSpear2.skill45 = floatd(entSpear2.skill11,32);
	
	//draw_num3d(playerDataPointer->surfaceAngle,entDummy.x,40,COLOR_WHITE);

	vec_set(entSpear2.pan,entSpear.pan);
	vec_set(entSpear2.x,entSpear.x);
	vec_for_bone(temp3,entSpear2,"Bone3");
	vec_set(entCharacter.x,temp3);
	vec_set(temp5,entCharacter.x);
	vec_to_ent(temp5,entSpear2);
	
	float effectF = -(-93.5-50)*0.002575; //saturate((48-InPos.x)/256.0); -(-93.5-temp5.z)
	float alpha = -effectF*fixv(entSpear.skill42); //sin(vecSkill41.x)*saturate((InPos.y-00)/37)*0.25*vecSkill41.y;
	float cosa = cos(alpha);
	float sina = sin(alpha);
	temp5.z += 93.5;
	float znew = temp5.z*cosa + temp5.x*sina;
	temp5.x = temp5.x*cosa - temp5.z*sina;
	temp5.z = znew-93.5;
	temp5.y -= 4;
	vec_for_ent(temp5,entSpear2);
	vec_set(entCharacter.x,temp5); // this is emulating old shader based spear deformation - we need to keep this as the logic references the (now invisible) character position...
	
	/*
	vec_set(entSpearWoosh.x,entSpear2.x); // was for a new game plus mode (or experimental game mechanic at the beginning)
	vec_set(entSpearWoosh.pan,entSpear2.pan);
	entSpearWoosh.skill42 = floatv(fixv(entSpear2.skill42));
	entSpearWoosh.skill41 = floatd(entSpear2.skill11,16);
	//set(entSpearWoosh, INVISIBLE); // for now !! debug error !!
	*/
	
	vec_set(playerPosLocal, entDummy.x);
	set(entSpear,INVISIBLE);
	set(entCharacter,INVISIBLE);
	set(entSpear2,INVISIBLE); // objects from prototyping, used by the gameplay logic so we just hide them... it's game jam code!

	vec_set(entHoneyCombo.x, entSpear2.x);
	vec_set(entHoneyCombo.pan, entSpear2.pan);
	
	float t = -playerDataPointer->loadPower/32.0;
	if(t < 0)
	{
		var animPerc = 100*-t;
		ent_animate(entHoneyCombo, "loadRight", animPerc, 0);
	}
	else
	{
		var animPerc = 100*t;
		ent_animate(entHoneyCombo, "loadLeft", animPerc, 0);
	}

	entSpear2.skill77 = entSpear2.y;
	entCharacter.skill77 = entCharacter.y;
	entSpearWoosh.skill77 = entSpearWoosh.y;
	entHoneyCombo.skill77 = entHoneyCombo.y;

	if(!playerDataPointer->groundContact)
	{
		//float speed = vec_length(playerDataPointer->speed);
		VECTOR spearDirection;
		vec_set(spearDirection, vector(0,0,-1));
		vec_rotate(spearDirection, vector(0, playerDataPointer->angle, 0));
		float speed = vec_dot(spearDirection, playerDataPointer->speed);
		if(speed < 0) speed = 0;
		speed = minv(pow(speed,0.875), 125)*0.2;
		if(speed > entHoneyCombo.skill70) entHoneyCombo.skill70 += (speed-entHoneyCombo.skill70)*0.75*time_step;
		else entHoneyCombo.skill70 += clamp((speed-entHoneyCombo.skill70)*0.5,-4,4)*time_step;
		var fabricRot = clamp(-playerDataPointer->turnSpeed*1.25 - speed*2.5, -40, 40);
		if(abs(fabricRot) > entHoneyCombo.skill71) entHoneyCombo.skill71 += (fabricRot-entHoneyCombo.skill71)*0.875*time_step;
		else entHoneyCombo.skill71 += clamp((fabricRot-entHoneyCombo.skill71)*0.5,-6,6)*time_step;
	}
	else
	{
		entHoneyCombo.skill70 += clamp(-entHoneyCombo.skill70*0.25,-4,4)*time_step;
		entHoneyCombo.skill71 += clamp((33-entHoneyCombo.skill71)*0.4,-6,6)*time_step;
	}
	entHoneyCombo.skill41 = floatd(entHoneyCombo.skill70, 20);
	var fabricAngle = entHoneyCombo.skill71*0.7;
	VECTOR* fabricVec = vector(0, fabricAngle, 0);
	ent_bonerotate(entHoneyCombo, "spearFabric0", fabricVec);
	fabricVec.y *= 1.1;
	ent_bonerotate(entHoneyCombo, "spearFabric1", fabricVec);
	fabricVec.y *= 1.1;
	ent_bonerotate(entHoneyCombo, "spearFabric2", fabricVec);
	fabricVec.y *= 1.1;
	ent_bonerotate(entHoneyCombo, "spearFabric3", fabricVec);

	///////////////////////// timer logic

	double d = (double)time_frame/(double)16.0;
	gameData.timeFloat += d;
	while(gameData.timeFloat > 1)
	{
		gameData.timeInteger += 1;
		gameData.timeFloat -= 1;
	}
	gameData.timeCombined = gameData.timeInteger;
	gameData.timeCombined += (var)gameData.timeFloat;

	spearUIDo();

	if(entDummy.z < -4580)
	{
		deathPercent = 100;
		mapReset();
	}
	if(entDummy.x > 4128 && entDummy.z > 5270)
	{
		outroState = 1;
		outroStateProgress = 0;
	}
}




#ifdef DEVTRUE
// copied from pogostuck, in-game editor stuff, not relevant to actual game

void cameraWidthAdjust(var newWidth);

BMAP *bmp_mouse = "mouse.tga";
var editorBrushSize = 256;
var editorBrushOpacity = 4;
var editorBrushOpacityMax = 100;
void playerFrozenToggle()
{
	playerFrozen = !playerFrozen;
}

void frozen_startup()
{
	on_f = playerFrozenToggle;
}

void draw_slider(STRING* str, STRING* str2, var* pvar, var vmin, var vmax, var pos_x, var pos_y)
{
	var value = *pvar;
	var range = vmax-vmin;
	float perc = (value-vmin)/(float)range;
	var width = 200;
	
	draw_text2(str,pos_x-str_width(str,NULL)-20,pos_y,COLOR_WHITE);
	draw_quad(NULL,vector(pos_x,pos_y+8,0),NULL,vector(width,4,0),NULL,COLOR_BLACK,50,0);
	draw_quad(NULL,vector(pos_x+width*perc-2,pos_y,0),NULL,vector(4,20,0),NULL,COLOR_WHITE,100,0);
	if(mouse_left && mouse_pos.x > pos_x-2 && mouse_pos.x <= pos_x+width+2 && mouse_pos.y > pos_y && mouse_pos.y < pos_y+20)
	{
		*pvar = clamp((mouse_pos.x-pos_x),0,width)/(float)width*range+vmin;
	}
	if(str2) draw_text2(str2,pos_x+width+20,pos_y,COLOR_WHITE);
}

action geoPainter_act()
{
	int numVerts = ent_status(my,1);
	cprintf1("\n <WARNING!> geoPainter_act: numVerts: %d",numVerts);
	on_f = playerFrozenToggle;
	while(1)
	{
		proc_mode = PROC_LATE;
		mouse_mode = 4;
		mouse_map = bmp_mouse;

		if(playerFrozen)
		{
			if(mickey.z)
			{
				if(key_ctrl || key_f)
				{
					editorBrushSize = clamp(editorBrushSize-16*sign(mickey.z),8,512);
				}
				else
				{
					cameraWidth = clamp(cameraWidth-mickey.z, 200, 4000);
					cameraWidthAdjust(0);
				}
			}
			float fac = 30+cameraWidth*0.03;
			camera.x += (key_d-key_a)*fac*time_step;
			camera.z += (key_w-key_s)*fac*time_step;
		}
		
		draw_slider(my->string1, NULL, &editorBrushSize,8,512,screen_size.x-300,460);
		draw_slider("Brush Size:",str_printf(NULL,"%d",(int)editorBrushSize),&editorBrushSize,8,512,screen_size.x-300,500);
		draw_slider("Brush Speed:",str_printf(NULL,"%d",(int)editorBrushOpacity),&editorBrushOpacity,0,100,screen_size.x-300,520);
		draw_slider("Brush Max Opacity:",str_printf(NULL,"%d",(int)editorBrushOpacityMax),&editorBrushOpacityMax,0,100,screen_size.x-300,540);
		if(!entityContainer.editorBrushEnt)
		{
			entityContainer.editorBrushEnt = ent_create("editorBrush1.mdl",camera.x,NULL);
			set(entityContainer.editorBrushEnt,PASSABLE | UNLIT | TRANSLUCENT | FLAG1);
			entityContainer.editorBrushEnt.flags2 |= UNTOUCHABLE; 
			entityContainer.editorBrushEnt.ambient = 100;
			entityContainer.editorBrushEnt.material = editorBrush_mat;
		}
		reset(entityContainer.editorBrushEnt,INVISIBLE);	
		entityContainer.editorBrushEnt.x = mouse_pos3d.x;
		entityContainer.editorBrushEnt.y = camera.y+100;
		entityContainer.editorBrushEnt.z = mouse_pos3d.z;
		//editorBrushSize = clamp(editorBrushSize+64*sign(mickey.z),64,512);
		//draw_text2(str_printf(NULL,"editorBrushSize: %d",(int)editorBrushSize),300,100,COLOR_WHITE);
		//draw_text2(str_printf(NULL,"editorBrushOpacity: %d",(int)editorBrushOpacity),300,120,COLOR_WHITE);
		vec_fill(entityContainer.editorBrushEnt.scale_x,editorBrushSize/128.0);
		if(mouse_left-mouse_right && mouse_pos.x < screen_size.x-300)
		{
			float changeFac = (mouse_left-mouse_right)*(float)time_step*(0.01*(float)editorBrushOpacity);
			VECTOR pos;
			vec_set(pos,entityContainer.editorBrushEnt.x);
			vec_to_ent(pos,my);
			int i;
			D3DVERTEX *vertexBuffer;
			short *indexBuffer;
			LPD3DXMESH mesh = ent_getmesh(my,0,0);
			mesh->LockVertexBuffer(0, &vertexBuffer);
			mesh->LockIndexBuffer(0, &indexBuffer);
			for(i = 0; i < numVerts; i++)
			{
				//if(vertexBuffer[i].nz > -0.15) continue;
				var dist = vec_length(vector(vertexBuffer[i].x-pos.x,0,vertexBuffer[i].y-pos.z));
				if(dist < editorBrushSize)
				{
					float distFac = floatSmootherLerp(0, 1, (editorBrushSize-dist) / (float) editorBrushSize);
					distFac = (distFac*distFac+distFac)*0.5;
					vertexBuffer[i].u2 = fclamp(vertexBuffer[i].u2+changeFac*distFac, -1.5, 1+editorBrushOpacityMax*0.01);
				}
			}
			mesh->UnlockVertexBuffer();
			mesh->UnlockIndexBuffer();
		}
		if(1)
		{
			static int key_enter_off = 0;
			if(key_bksp && key_enter_off)
			{
				STRING *str = str_create("models\\");
				str_cat(str, my->string1);
				mdlSaveEntity(my, str, "colorPalette.tga", 0);
				ptr_remove(str);
				beep();
			}
			key_enter_off = !key_bksp;
		}
		
		wait(1);	
	}
}
#endif
