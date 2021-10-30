//////////////////////////////
// kuSoundSpear.c
//////////////////////////////

//#define KUSOUND_DEBUG

void kuSoundFreeBuffer(int i)
{
	if(i < 0 || i >= KUSOUND_SOURCES_MAX) return;

	KUSOUND_SOURCE *source = &kuSoundSources[i];
	if(source->buffer && source->isLoaded == 2 && !source->numInstances)
	{
		source->preLoadWanted = 0;
		source->isLoaded = 0;
		sys_free(source->buffer);
		source->buffer = NULL;
	}
	if(source->dsb8) kuSoundDSB8Release(source->dsb8);
	source->dsb8 = NULL;
}


KUSOUND_SOURCE* kuSoundSourceInit_IDTypeNamePreload(int id, int type, char* cfile, int preLoadWanted, int maxInstances)
{
	KUSOUND_SOURCE *source = &kuSoundSources[id];
	source->isInitialized = 1;
	source->type = type;
	strcpy(source->cfileName,cfile);
	source->preLoadWanted = preLoadWanted;
	source->isOgg = 0;
	source->maxInstances = 0;
	int len = strlen(cfile);
	if(len > 0)
	{
		if(cfile[len-1] == 'g') source->isOgg = 1;
	}
	if(preLoadWanted == KUSOUND_PRELOAD_LOAD_ON_DEMAND) source->flags |= KUSOUND_FLAG_LOAD_ON_DEMAND;
	return source;
	//cprintf3("\nkuSoundSourceInit_IDTypeNamePreload(%d): cfile(%s), source->cfileName(%s)",(int)id,cfile,source->cfileName);
}


void kuSoundDestroy(KUSOUND* sound, int mode)
{
	if(!sound) return;

	if(mode)
	{
		IDirectSoundBuffer8* dsb8 = sound->dsb8;
		if(dsb8) kuSoundDSB8Release(dsb8);
	}
	KUSOUND_SOURCE *source = &kuSoundSources[sound->sourceID];

	#ifdef KUSOUND_DEBUG
		cprintf6("\nkuSoundDestroy(%p, %d) at frame %d: sourceId(%d) filename(%s), sourceNumInstances(%d)",sound,mode,(int)total_frames,(int)sound->sourceID,source->cfileName,source->numInstances);
		cprintf2(" (source->pFirstInstance = %p ==? %p = sound)",source->pFirstInstance,sound);
	#endif

	if(source->pFirstInstance == sound) source->pFirstInstance = sound->nextInstance;
	else
	{
		KUSOUND* prev = NULL;
		KUSOUND *current = source->pFirstInstance;
		int num = 0;
		while(current)
		{
			KUSOUND_SOURCE *source2 = &kuSoundSources[current->sourceID];
			#ifdef KUSOUND_DEBUG
				cprintf4("\nREMOVELOOP(%d/%d, %p, source %s)",num,source->numInstances,current,source2->cfileName);
			#endif
			
			if(current->sourceID != sound->sourceID) error("kuSoundDestroy: current->sourceID != sound->sourceID, that's impossible!"); 
			if(current == sound)
			{
				if(!prev) error("kuSoundDestroy: prev == NULL, that's impossible!"); 
				prev->nextInstance = current->nextInstance;
				break;
			}
			prev = current;
			current = current->nextInstance;
			num++;
		}
		#ifdef KUSOUND_DEBUG
			cprintf0(" - left loop.");
		#endif
		if(!current) error("kuSoundDestroy: sound not found in instance list!?");
	}
	if(sound->pointer)
	{
		//cprintf2(" (set sound->pointer(%p, %p) to NULL)",sound->pointer,*sound->pointer);
		//if(sound->pointer == sound) *sound->pointer = NULL;
		//sound->pointer = NULL;
		*sound->pointer = NULL;
	}

	source->numInstances--;
	//if(source->numInstances <= 0 && (sound->flags & KUSOUND_FLAG_IS_MUSIC) && !(sound->flags & KUSOUND_FLAG_DONT_FREE_BUFFER)) kuSoundFreeBuffer(sound->sourceID);
	if(source->numInstances <= 0 && source->pFirstInstance != NULL) error("kuSoundDestroy: source->numInstances <= 0 && source->pFirstInstance != NULL"); //source->dsb8Instance = NULL;
	sys_free(sound);
}

KUSOUND* kuSoundCreate(IDirectSoundBuffer8 *dsb8, int sourceID, int flags, VECTOR *pos, float volume, float speed, float pan, float range)
{
	if(!kuSoundInitialized) return NULL;
	KUSOUND* sound;

	sound = (KUSOUND*)sys_malloc(sizeof(KUSOUND));
	sound->dsb8 = dsb8;
	sound->sourceID = sourceID;
	sound->flags = flags;
	if(pos)
	{
		vec_set(sound->pos,pos);
		sound->flags |= KUSOUND_FLAG_LEVELSOUND;
	}
	else sound->flags &= ~KUSOUND_FLAG_LEVELSOUND;
	sound->volume = volume;
	sound->speed = speed;
	sound->pan = pan;
	sound->range = range;
	sound->fadePerc = 100;
	sound->fadeSpeed = 0;
	sound->playTime = 0;
	sound->playPosWanted = -1;
	sound->timerDuration = 0;
	sound->timerFadeoutSpeed = 0;
	sound->playStatus = 0;
	sound->length = 0;
	sound->next = pKuSoundFirst;
	sound->pointer = NULL;
	pKuSoundFirst = sound;
	#ifdef KUSOUND_DEBUG
		cprintf3("\nkuSoundCreate at frame %d: pKuSoundFirst = sound = %p, sound->next(=oldFirstSnd) = %p",(int)total_frames,sound,sound->next);
	#endif

	return sound;
}

KUSOUND* kuSoundPlayAtPos(int sourceID, VECTOR *pos, bool loop, float volume, float speed, float range)
{
	if(!kuSoundInitialized) return NULL;
	if(sourceID >= KUSOUND_SOURCES_MAX)
	{
		printf("kuSoundPlayAtPos: invalid sourceID(%d)!", sourceID);
		return NULL;
	}
	KUSOUND_SOURCE *source = &kuSoundSources[sourceID];
	#ifdef KUSOUND_DEBUG
		cprintf7("\nkuSoundPlayAtPos(id %d -> %s) at frame %d: buffer: %p, initialized(%d), isLoaded(%d), preLoadWanted(%d)",(int)sourceID, source->cfileName, ITF, source->buffer,(int)source->isInitialized,(int)source->isLoaded,(int)source->preLoadWanted);
	#endif
	if(!source->isInitialized) return NULL;
	source->preLoadWanted = 1;
	if(source->isLoaded < 2) return NULL;
	//cprintf0(" -DSB8 NOW");

	sys_marker("K1A");
	IDirectSoundBuffer8* dsb8 = NULL;
	if(source->dsb8)
	{
		sys_marker("K9A");
		int errorCode = 0;
		dsb8 = kuSoundDuplicateDSB8(source->dsb8, &errorCode);
		sys_marker("K9B");
		if(!dsb8)
		{
			static int shownFailA = 0;
			if(!shownFailA)
			{
				shownFailA = 1;
				printf("kuSoundPlayAtPos(ID %d): source->dsb8(%p) duplicate (A) failed! errorCode(%d)", sourceID, source->dsb8, errorCode);
			}
			cprintf4("\nkuSoundPlayAtPos(ID %d): source->dsb8(%p) duplicate (A) failed! | errorCode(%d) | at frame %d", sourceID, source->dsb8, ITF, errorCode);
			return NULL;
		}
		sys_marker("K9C");
		kuSoundDSB8SetVolume(dsb8, 1.1);
		kuSoundDSB8SetVolume(dsb8, 1);
		kuSoundDSB8SetSpeed(dsb8, speed);
		sys_marker("K9D");
	}
	else
	{
		if(source->pFirstInstance)
		{
			KUSOUND *sndFirst = source->pFirstInstance;
			#ifdef KUSOUND_DEBUG
				cprintf2(" - numInstances(%d), first(%p)",source->numInstances,sndFirst);
			#endif
			sys_marker("K2A");
			int errorCode = 0;
			dsb8 = kuSoundDuplicateDSB8(sndFirst->dsb8, &errorCode);
			sys_marker("K3A");
			if(!dsb8)
			{
				static int shownFailB = 0;
				if(!shownFailB)
				{
					shownFailB = 1;
					printf("kuSoundPlayAtPos(ID %d): source->dsb8(%p) duplicate (B) failed! errorCode(%d)", sourceID, source->dsb8, errorCode);
				}
				cprintf4("\nkuSoundPlayAtPos(ID %d): source->dsb8(%p) duplicate (B) failed! | errorCode(%d) | at frame %d", sourceID, source->dsb8, ITF, errorCode);
				return NULL;
			}
			sys_marker("K4A");
			kuSoundDSB8SetVolume(dsb8, 1.1); //); // 0.01 bad hack because of documented DirectSound bug volume-10
			kuSoundDSB8SetVolume(dsb8, 1); //); // 0.01 bad hack because of documented DirectSound bug
			kuSoundDSB8SetSpeed(dsb8, speed);
		}
		else
		{
			sys_marker("K5A");
			#ifdef KUSOUND_DEBUG
				cprintf0(" - no first instance, create it!");
			#endif
			sys_marker("K5B");
			if(source->isOgg) dsb8 = kuSoundCreateDSB8FromOggBuffer(source->buffer, source->size);
			else dsb8 = kuSoundCreateDSB8FromBuffer(source->buffer);
			//dsb8 = kuSoundCreateDSB8FromBuffer(source->buffer);
			kuSoundDSB8SetVolume(dsb8, 1);
			if(!dsb8) cprintf6("\n COULD NOT PLAY SOUND(ID %d, %s, ogg %d, buffer %p, %d) at frame %d!!!", (int)sourceID, source->cfileName, (int)source->isOgg, source->buffer, source->size, ITF);
			sys_marker("K5C");
		}
	}
	sys_marker(NULL);
	if(!dsb8) return NULL;
	sys_marker("K6A");
	KUSOUND* sound = kuSoundCreate(dsb8, sourceID, KUSOUND_FLAG_LOOPS*loop | source->flags, pos, volume, speed, 0, range);
	#ifdef KUSOUND_DEBUG
		cprintf3(" -kuSoundCreated(%p) at frame %d, total_ticks(%.3f)",sound,(int)total_frames,(double)total_ticks);
	#endif
	sound->playStatus = 0;
	int length;
	kuSoundDSB8GetCurrentPosition(dsb8, &length);
	sound->length = length;
	if(source->type == KUSOUND_TYPE_MUSIC) sound->flags |= KUSOUND_FLAG_IS_MUSIC;
	if(source->type == KUSOUND_TYPE_AMBIENT) sound->flags |= KUSOUND_FLAG_IS_AMBIENT;

	source->numInstances++;
	sound->nextInstance = source->pFirstInstance;
	source->pFirstInstance = sound;

	sys_marker(NULL);

	return sound;
}

KUSOUND* kuSoundPlay2D(int sourceID, bool loop, float volume, float speed, float pan)
{
	if(!kuSoundInitialized) return NULL;
	KUSOUND* sound;

	sound = kuSoundPlayAtPos(sourceID, NULL, loop, volume, speed, 1); // pos NULL => (no) level sound flag automatically set
	if(sound) sound->pan = pan;

	return sound;
}


void kuSoundUpdatePos(KUSOUND* sound,VECTOR *pos)
{
	if(!sound) return;
	vec_set(sound->pos,pos);
}

void kuSoundSetVolume(KUSOUND* sound, float volume)
{
	if(!sound) return;
	sound->volume = volume;
}

void kuSoundSetSpeed(KUSOUND* sound, float speed)
{
	if(!sound) return;
	sound->speed = speed;
}

void kuSoundSetPointer(KUSOUND* sound, void **p)
{
	if(!sound) return;
	sound->pointer = p;
}

void kuSoundSetFlag(KUSOUND* sound, int flagsNew)
{
	if(!sound) return;
	sound->flags |= flagsNew;
}

void kuSoundPause(KUSOUND* sound, bool paused)
{
	//cprintf3("\n XXXXXXXX calling kuSoundPause(%p, %d) at frame %d:", sound, paused, ITF);
	if(!sound) return;
	if(!paused && !(sound->flags & KUSOUND_FLAG_IS_PAUSED)) return;
	if(paused && (sound->flags & KUSOUND_FLAG_IS_PAUSED)) return;
	IDirectSoundBuffer8* dsb8 = sound->dsb8;
	KUSOUND_SOURCE *source = &kuSoundSources[sound->sourceID];
	int status = kuSoundDSB8GetStatus(dsb8);
	//cprintf6("flags(%d) status(%d) isPaused(%d) source(%d, %s) dsb8(%p)", sound->flags, status, !!(sound->flags & KUSOUND_FLAG_IS_PAUSED), sound->sourceID, source->cfileName, dsb8);
	if(!dsb8) return;
	if(paused)
	{
		kuSoundDSB8Stop(dsb8);
		sound->flags |= KUSOUND_FLAG_IS_PAUSED;
	}
	else
	{
		kuSoundDSB8Play(dsb8, !!(sound->flags & KUSOUND_FLAG_LOOPS));
		sound->flags &= ~KUSOUND_FLAG_IS_PAUSED;
	}
}

void kuSoundStop(KUSOUND* sound, int speed)
{
	if(!sound) return;
	sound->flags &= ~KUSOUND_FLAG_PAUSES_ON_MENU;
	if(sound->flags & KUSOUND_FLAG_IS_PAUSED) kuSoundPause(sound, false);
	//sound->fadePerc = 100;
	sound->fadeSpeed = speed;
}

int kuSoundSetPlayPos(KUSOUND* sound, int playPos)
{
	if(!sound) return -2;
	sound->playPosWanted = playPos;
	IDirectSoundBuffer8* dsb8 = sound->dsb8;
	if(!dsb8) return -3;
	return kuSoundDSB8SetCurrentPosition(dsb8, playPos);
}

int kuSoundGetPlayPos(KUSOUND* sound)
{
	if(!sound) return -1;
	IDirectSoundBuffer8* dsb8 = sound->dsb8;
	if(!dsb8) return;
	return kuSoundDSB8GetCurrentPosition(dsb8, NULL);
}

int kuSoundGetPlayLength(KUSOUND* sound)
{
	int length;
	if(!sound) return -1;
	IDirectSoundBuffer8* dsb8 = sound->dsb8;
	if(!dsb8) return;
	kuSoundDSB8GetCurrentPosition(dsb8, &length);
	return length;
}

void kuSoundSetTimeout(KUSOUND* sound, var duration, var fadeoutSpeed)
{
	if(!sound) return;
	sound->flags |= KUSOUND_FLAG_USETIMER;
	sound->timerDuration = duration;
	sound->timerFadeoutSpeed = fadeoutSpeed;
}

void kuSoundRequestPreload(int id)
{
	KUSOUND_SOURCE *source = &kuSoundSources[id];
	source->preLoadWanted = 1;
}

int kuSoundGetPreloadRequestStatus(int id)
{
	KUSOUND_SOURCE *source = &kuSoundSources[id];
	if(source->isLoaded < 2) return 0;
	return 1;
}

void kuSoundSetMasterVolumeIn100(float volume)
{
	if(volume < 0.01) volume = 0;
	kuSoundMasterVolume = volume*0.01;
}

void kuSoundUpdateFrame(VIEW* view)
{
	if(!kuSoundInitialized) return;
	float f,deltaTime;
	var dist, maxDist = 1024;
	int currentPos,pauseUnpauseDo = 0;
	VECTOR temp,temp2;
	KUSOUND* prev = NULL, *current;

	#ifdef KUSOUND_DEBUG_FRAME
		cprintf1("\n kuSoundUpdateFrame at frame %d: START", ITF);
	#endif

	deltaTime = time_step;
	if(!view) // ??
	{
		view = camera;
		deltaTime = time_step*0.5; // lerping ?? <- what the shit
	}

	// ambient logic here

	var music_volume = 100;
	static var musicToAmbientFade = 0;
	if(menuActive) musicToAmbientFade = maxv(musicToAmbientFade-5*time_step, 0);
	else musicToAmbientFade = minv(musicToAmbientFade+5*time_step, 100);
	
	if(musicToAmbientFade > 0)
	{
		if(!KUSOUND_INSTANCE_AMBIENT0) KUSOUND_INSTANCE_AMBIENT0 = kuSoundPlay2D(KS_AMBIENT_WIND0, true, 100, 1, 0);
		if(!KUSOUND_INSTANCE_AMBIENT_WATER) KUSOUND_INSTANCE_AMBIENT_WATER = kuSoundPlay2D(KS_AMBIENT_WATER0, true, 50, 1, 0);
		if(!KUSOUND_INSTANCE_AMBIENT_DRONE) KUSOUND_INSTANCE_AMBIENT_DRONE = kuSoundPlay2D(KS_AMBIENT_DRONE0, true, 1, 1, 0);
		if(KUSOUND_INSTANCE_AMBIENT0 && KUSOUND_INSTANCE_AMBIENT_WATER && KUSOUND_INSTANCE_AMBIENT_DRONE)
		{
			var droneVolume = clamp((camera.z-2150)*0.02, 0, 100);
			float droneVolumeFac = droneVolume*0.01;
			float droneVolumeFacInv = 1-droneVolumeFac;
			kuSoundSetVolume(KUSOUND_INSTANCE_AMBIENT_WATER, clamp(100-(camera.z+4300)*0.0875, 0, 100)*0.5*droneVolumeFacInv);
			kuSoundSetVolume(KUSOUND_INSTANCE_AMBIENT0, (100-0.5*clamp(100-(camera.z+4300)*0.0875, 0, 100))*droneVolumeFacInv);
			kuSoundSetVolume(KUSOUND_INSTANCE_AMBIENT_DRONE, droneVolume*0.75);
		}
	}


	float leftRightFac = 1;
	//if(camera.roll) leftRightFac = cosv(camera.roll);
	var view_right_limited = minv(view.right, 1500);
	KUSOUND* sound = pKuSoundFirst; // might get changed by previous functions, don't move
	kuSoundUpdateNumSounds = 0;	
	#ifdef KUSOUND_DEBUG_FRAME
		cprintf0(" mainLoopNow");
	#endif
	float ambient_vol_f = 1;
	while(sound)
	{
		int prevStatus = sound->playStatus;
		if(!(sound->flags & KUSOUND_FLAG_IS_INVALID))
		{
			IDirectSoundBuffer8* dsb8 = sound->dsb8;
			if(!dsb8) error("kuSoundUpdateFrame(): sound without valid IDirectSoundBuffer8!");
			
			sound->playTime += time_step;
			if(sound->flags & KUSOUND_FLAG_USETIMER)
			{
				if(sound->playTime >= sound->timerDuration) sound->fadeSpeed = sound->timerFadeoutSpeed;
			}
			
			if(sound->fadeSpeed > 0) sound->fadePerc = minv(sound->fadePerc+sound->fadeSpeed*deltaTime,100);
			if(sound->fadeSpeed < 0)
			{
				sound->fadePerc = maxv(sound->fadePerc+sound->fadeSpeed*deltaTime,0);
				if(!sound->fadePerc) sound->playStatus = 3;
			}
			f = 1;
			if(!(sound->flags & KUSOUND_FLAG_NOAUTOUPDATE))
			{
				if(sound->flags & KUSOUND_FLAG_LEVELSOUND)
				{
					if(is(view,ISOMETRIC) && view.right)
					{
						float d = leftRightFac*(sound->pos.x-view.x)/(float)view_right_limited;
						sound->pan = clamp(d*0.2,-1,1);
						float dist = vec_length(vector(sound->pos.x-(view.x),0,(sound->pos.z-(view.z))*video_aspect*0.875));
						if(sound->flags & KUSOUND_FLAG_SMOOTH_FALLOFF)
						{
							f = clamp((1.25+sound->range*0.0) - dist/(view_right_limited*sound->range), 0, 1);
							float f2 = f;
							f = f*f*0.5+f*0.5;
							//draw_text2(str_printf(NULL,"blade: f(%.3f)->%.3f | %d/%d", (double)f, (double)(f*f),(int)dist,(int)(view_right_limited*sound->range)),800,400,COLOR_WHITE);
						}		
						else
						{
							f = clamp(sound->range*1.25 - dist/(float)view_right_limited, 0, 1);
						}
					}
					else
					{
						// perspective
						vec_diff(temp,sound->pos,view.x);
						dist = vec_to_angle(temp2,temp);
						f = clamp((maxDist-dist/sound->range)/maxDist*1.25,0,1);
						if(view.arc) sound->pan = -ang(temp2.x-view.pan)/view.arc*0.25;
					}
					f *= sound_vol*0.01;
				}
				/*else
				{
					//if(sound->flags & KUSOUND_FLAG_LEVELSOUND) f = 1;
					if(sound->flags & KUSOUND_FLAG_IS_MUSIC)
					{
						KUSOUND_SOURCE *source = &kuSoundSources[sound->sourceID];
						//draw_text(str_printf(NULL,"music: %s",source->cfileName),400,10,COLOR_RED);
						f = music_volume*0.01;
						if(!(sound->flags & KUSOUND_FLAG_NOSPEEDOVERWRITE)) sound->speed = 1; //kuSoundMusicSpeed*0.01;
					}
					else
					{
						if(sound->flags & KUSOUND_FLAG_IS_AMBIENT)
						{
							//ambientSoundHelperList
							//f = ambient_vol*0.01*(float)(100-ambient_vol_music_fac_smooth)*0.01;
							f = ambient_vol_f;
						}
					}
				}*/
				sound->volFacPrev = f;
			}
			else f = sound->volFacPrev;
			if(sound == KUSOUND_INSTANCE_MUSIC) sound->volFacPrev = f = 1-musicToAmbientFade*0.01;
			if(sound->flags & KUSOUND_FLAG_IS_AMBIENT) sound->volFacPrev = f = musicToAmbientFade*0.01;
			double f0 = f;
			f *= sound->fadePerc*0.01;
			double f1 = f;
			f = kuSoundMasterVolume*f*sound->volume; // 1 * 1 * 100
			if(f < 0.1) f = 0.1;
			double f2 = f;
			kuSoundDSB8SetVolume(dsb8, f);
			kuSoundDSB8SetPan(dsb8, sound->pan);
			//float speedFac = 1.0;
			//if(!(sound->flags & KUSOUND_FLAG_NOTAFFECTEDBYFOCUS)) speedFac = kuSoundFocusSoundFactor;
			//speedFac *= 0.01*(float)kuSoundWaterSpeed;
			if(sound->fadeSpeed < 0 && (sound->flags & KUSOUND_FLAG_FADEOUT_W_SPEED)) sound->speed = 0.1+0.9*sound->fadePerc*0.01;
			kuSoundDSB8SetSpeed(dsb8, sound->speed); // speedFac*
			
			int status = kuSoundDSB8GetStatus(dsb8);
			#ifdef KUSOUND_DEBUG
				KUSOUND_SOURCE *source = &kuSoundSources[sound->sourceID];
				draw_text2(str_printf(NULL,"%d) %s %p speed(%.1f) flags(%d) dsb8status(%d) playStatus(%d) f(%.2f,%.2f,%.2f)",(int)kuSoundUpdateNumSounds,source->cfileName,sound,(double)sound->speed, sound.flags, status, sound->playStatus, f0,f1,f2),800,400+20*kuSoundUpdateNumSounds,COLOR_WHITE);
				//cprintf3("\nkuSoundUpdateFrame: LOOP: sound(%d, %p, %s)",kuSoundUpdateNumSounds, sound, source->cfileName);
			#endif

			if(!sound->playStatus) // || (sound->playStatus == 1 && !(status & DSBSTATUS_PLAYING)) )
			{
				sound->playStatus = 1;
				kuSoundDSB8Play(dsb8, !!(sound->flags & KUSOUND_FLAG_LOOPS));		
			}
			
			if(!(sound->flags & KUSOUND_FLAG_LOOPS))
			{
				currentPos = kuSoundDSB8GetCurrentPosition(dsb8, NULL);
				if(sound->playStatus < 2)
				{
					if(currentPos > 0) sound->playStatus = 2;
				}
				else 
				{
					if(currentPos <= 0)
					{
						sound->playStatus = 3;
						sound->flags |= KUSOUND_FLAG_IS_INVALID;
						//cprintf2("\n KUSOUND: set sound(%d) to INVALID at frame %d", sound->sourceID, ITF);
					}
				}
			}
			
			if(sound->flags & KUSOUND_FLAG_PAUSES_ON_MENU)
			{
				if(menuActive) kuSoundPause(sound, true);
				else kuSoundPause(sound, false);
			}
		}
		if(prevStatus >= 3)
		{
			#ifdef KUSOUND_DEBUG
				cprintf3(" - STOP sound(%p), prev(%p), first(%p)", sound, prev, pKuSoundFirst);
			#endif
			if(prev) prev->next = sound->next;
			else
			{
				if(sound == pKuSoundFirst) pKuSoundFirst = sound->next;
				else printf("prev 0 but sound(%p) != pKuSoundFirst(%p), impossible!",sound,pKuSoundFirst);
			}
			current = sound;
			sound = sound->next;
			#ifdef KUSOUND_DEBUG
				cprintf3("\ncalling kuSoundDestroy from kuSoundUpdateMainLoop for sound(%p, %d) at frame %d",current,kuSoundUpdateNumSounds,(int)total_frames);
			#endif
			kuSoundDestroy(current,1);
		}
		else
		{
			prev = sound;
			sound = sound->next;
		}
		
		kuSoundUpdateNumSounds++;
	}
	#ifdef KUSOUND_DEBUG_FRAME
		cprintf1(" soundloopEND(%d)", ITF);
	#endif

	//kuSoundUpdateTime = dtimer();
	/*kuSoundUpdateNumSounds = 0;	
	while(sound)
	{
		KUSOUND_SOURCE *source = &kuSoundSources[sound->sourceID];
		cprintf3("\nkuSoundUpdateFrame: POST LOOP: sound(%d, %p, %s)",kuSoundUpdateNumSounds, sound, source->cfileName);
		sound = sound->next;
		kuSoundUpdateNumSounds++;
	}
	sound = pKuSoundFirst;*/

	/*int i;
	for(i = 0; i < KUSOUND_SOURCES_MAX; i++)
	{
		KUSOUND_SOURCE *source = &kuSoundSources[i];
		draw_text(str_printf(NULL,"source %d) instances: %d",i,source->numInstances),screen_size.x-300,40+20*i,COLOR_RED);
	}*/
}

void kuSoundStopAllLevelSounds(int mode)
{
	KUSOUND* sound = pKuSoundFirst;

	//dtimer();
	kuSoundUpdateNumSounds = 0;	
	while(sound)
	{
		if(sound->fadeSpeed >= 0 && (sound->flags & (KUSOUND_FLAG_IS_MUSIC | KUSOUND_FLAG_LEVELSOUND | KUSOUND_FLAG_STOP_ON_LEVELCHANGE)) )
		{
			sound->flags &= ~KUSOUND_FLAG_LEVELSOUND;
			sound->flags &= ~KUSOUND_FLAG_STOP_ON_LEVELCHANGE;
			sound->flags &= ~KUSOUND_FLAG_IS_PAUSED;
			sound->flags |= KUSOUND_FLAG_NOAUTOUPDATE;
			kuSoundStop(sound,KUSOUND_STOP_SPEED_NORMAL);
		}
		sound = sound->next;
	}
}


void kuSoundDestroyDevice()
{
	int i;

	cprintf1("\nkuSoundDestroyDevice() at frame %d - START",ITF);
	KUSOUND* sound = pKuSoundFirst;
	while(sound)
	{
		KUSOUND* current = sound;
		sound = sound->next;
		kuSoundDestroy(current,1);
	}
	for(i = 0; i < KUSOUND_SOURCES_MAX; i++)
	{
		KUSOUND_SOURCE *source = &kuSoundSources[i];
		if(source->dsb8)
		{
			kuSoundDSB8Release(source->dsb8);
			source->dsb8 = NULL;
		}
		kuSoundFreeBuffer(i);
	}
	cprintf0("\nkuSoundDestroyDevice() - END");
}

//////////////////////////////


//#define KUSOUND_SOURCES_MAX 16 define this in kuSoundSpear.h


void kuSoundSourcesInit_filesetup()
{
	kuSoundSourceInit_IDTypeNamePreload(KS_MUSIC_MENU, KUSOUND_TYPE_SFX, "sounds\\menuMusic.wav", KUSOUND_PRELOAD_ALWAYS, 1);
	
	kuSoundSourceInit_IDTypeNamePreload(KS_AMBIENT_WIND0, KUSOUND_TYPE_AMBIENT, "sounds\\ambientWind0.wav", KUSOUND_PRELOAD_ALWAYS, 1);
	kuSoundSourceInit_IDTypeNamePreload(KS_AMBIENT_WIND1, KUSOUND_TYPE_AMBIENT, "sounds\\ambientWind1.wav", KUSOUND_PRELOAD_ALWAYS, 1);
	kuSoundSourceInit_IDTypeNamePreload(KS_AMBIENT_WATER0, KUSOUND_TYPE_AMBIENT, "sounds\\ambientWater0.wav", KUSOUND_PRELOAD_ALWAYS, 1);
	kuSoundSourceInit_IDTypeNamePreload(KS_AMBIENT_DRONE0, KUSOUND_TYPE_AMBIENT, "sounds\\ambientDrone0.wav", KUSOUND_PRELOAD_ALWAYS, 1);
	
	kuSoundSourceInit_IDTypeNamePreload(KS_SFX_DEATHSCARE, KUSOUND_TYPE_SFX, "sounds\\deathScare.wav", KUSOUND_PRELOAD_ALWAYS, 1);
	kuSoundSourceInit_IDTypeNamePreload(KS_SFX_MENU_ACCEPT, KUSOUND_TYPE_SFX, "sounds\\menuAccept.wav", KUSOUND_PRELOAD_ALWAYS, 2);
	kuSoundSourceInit_IDTypeNamePreload(KS_SFX_MENU_BACK, KUSOUND_TYPE_SFX, "sounds\\menuTone4.wav", KUSOUND_PRELOAD_ALWAYS, 2); //menuBack
	kuSoundSourceInit_IDTypeNamePreload(KS_SFX_MENU_CHANGE, KUSOUND_TYPE_SFX, "sounds\\menuChange.wav", KUSOUND_PRELOAD_ALWAYS, 2);
	kuSoundSourceInit_IDTypeNamePreload(KS_SFX_MENU_SELECT, KUSOUND_TYPE_SFX, "sounds\\menuSelect.wav", KUSOUND_PRELOAD_ALWAYS, 2);
	kuSoundSourceInit_IDTypeNamePreload(KS_SFX_MENU_UPDOWN, KUSOUND_TYPE_SFX, "sounds\\menuTone0.wav", KUSOUND_PRELOAD_ALWAYS, 2); // menuUpDown
	kuSoundSourceInit_IDTypeNamePreload(KS_SFX_MENU_OPEN, KUSOUND_TYPE_SFX, "sounds\\menuOpen2.wav", KUSOUND_PRELOAD_ALWAYS, 2);
	kuSoundSourceInit_IDTypeNamePreload(KS_SFX_MENU_CLOSE, KUSOUND_TYPE_SFX, "sounds\\menuClose2.wav", KUSOUND_PRELOAD_ALWAYS, 2); // menuOpen3
	kuSoundSourceInit_IDTypeNamePreload(KS_SFX_MENU_TONE0, KUSOUND_TYPE_SFX, "sounds\\menuTone0.wav", KUSOUND_PRELOAD_ALWAYS, 2);
	kuSoundSourceInit_IDTypeNamePreload(KS_SFX_MENU_TONE1, KUSOUND_TYPE_SFX, "sounds\\menuTone1.wav", KUSOUND_PRELOAD_ALWAYS, 2);
	kuSoundSourceInit_IDTypeNamePreload(KS_SFX_MENU_TONE2, KUSOUND_TYPE_SFX, "sounds\\menuTone2.wav", KUSOUND_PRELOAD_ALWAYS, 2);
	kuSoundSourceInit_IDTypeNamePreload(KS_SFX_MENU_TONE3, KUSOUND_TYPE_SFX, "sounds\\menuTone3.wav", KUSOUND_PRELOAD_ALWAYS, 2);
	kuSoundSourceInit_IDTypeNamePreload(KS_SFX_MENU_TONE4, KUSOUND_TYPE_SFX, "sounds\\menuTone4.wav", KUSOUND_PRELOAD_ALWAYS, 2);
	kuSoundSourceInit_IDTypeNamePreload(KS_SFX_MENU_NEWGAME, KUSOUND_TYPE_SFX, "sounds\\menuOpen3.wav", KUSOUND_PRELOAD_ALWAYS, 2);
	kuSoundSourceInit_IDTypeNamePreload(KS_SFX_SPEAR_HIT0, KUSOUND_TYPE_SFX, "sounds\\spearHit0.wav", KUSOUND_PRELOAD_ALWAYS, 2);
	kuSoundSourceInit_IDTypeNamePreload(KS_SFX_SPEAR_HIT1, KUSOUND_TYPE_SFX, "sounds\\spearHit1.wav", KUSOUND_PRELOAD_ALWAYS, 2);
	kuSoundSourceInit_IDTypeNamePreload(KS_SFX_SPEAR_LAUNCH0, KUSOUND_TYPE_SFX, "sounds\\spearLaunch0c.wav", KUSOUND_PRELOAD_ALWAYS, 2);
	kuSoundSourceInit_IDTypeNamePreload(KS_SFX_SPEAR_LAUNCH1, KUSOUND_TYPE_SFX, "sounds\\spearLaunch1.wav", KUSOUND_PRELOAD_ALWAYS, 2);
	kuSoundSourceInit_IDTypeNamePreload(KS_SFX_INTRO_JUMP, KUSOUND_TYPE_SFX, "sounds\\introJumpb.wav", KUSOUND_PRELOAD_ALWAYS, 1);
	kuSoundSourceInit_IDTypeNamePreload(KS_SFX_PLAYER_IMPACT0, KUSOUND_TYPE_SFX, "sounds\\playerImpact0.wav", KUSOUND_PRELOAD_ALWAYS, 1);
	kuSoundSourceInit_IDTypeNamePreload(KS_SFX_PLAYER_IMPACT1, KUSOUND_TYPE_SFX, "sounds\\playerImpact1.wav", KUSOUND_PRELOAD_ALWAYS, 1);
	kuSoundSourceInit_IDTypeNamePreload(KS_SFX_PLAYER_IMPACT2, KUSOUND_TYPE_SFX, "sounds\\playerImpact2.wav", KUSOUND_PRELOAD_ALWAYS, 1);
	kuSoundSourceInit_IDTypeNamePreload(KS_SFX_WIN, KUSOUND_TYPE_SFX, "sounds\\win.wav", KUSOUND_PRELOAD_ALWAYS, 1);
	
}

void kuSoundPreloadFiles()
{
	int i;

	//SetCurrentDirectory(_chr(work_dir));
	cprintf1("\n kuSoundPreloadFiles at frame (%d)", ITF);
	if(!kuSoundInitialized) return;
	for(i = 0; i < KUSOUND_SOURCES_MAX; i++)
	{
		KUSOUND_SOURCE *source = &kuSoundSources[i];
		if(!source->isInitialized || source->buffer) continue;
		int size = 0;
		file_load(source->cfileName, &size, &size);
		cprintf2("\n - sound file(%s)... size(%d)", source->cfileName, size);
		if(size > 0)
		{
			source->size = size;
			source->buffer = (char*)sys_malloc( size );
			file_load(source->cfileName, source->buffer, &size);
			source->isLoaded = 2;
		}
	}
}

void kuSoundSourcesInit()
{
	int i,j;

	sys_marker("S1A");
	if(!kuSoundInitialized) return;
	// no kupack registry logic in this project
	for(i = 0; i < KUSOUND_SOURCES_MAX; i++)
	{
		KUSOUND_SOURCE *source = &kuSoundSources[i];
		source->id = i;
		source->type = 0;
		source->isLoaded = 0;
		source->preLoadWanted = 0;
		source->isInitialized = 0;
		source->lastError = 0;
		source->size = 0;
		source->buffer = NULL;
		source->pan = 0;
		source->volume = 100;
		source->frequency = 1;
		source->numInstances = 0;
		source->pFirstInstance = NULL;
	}
	sys_marker("S2A");
	kuSoundSourcesInit_filesetup();
	sys_marker("S3A");
	kuSoundPreloadFiles();
	//kuSoundThreadStart();
	sys_marker(NULL);
}

int kuSoundGetNumInstances(int sourceID)
{
	if(sourceID < 0 || sourceID >= KUSOUND_SOURCES_MAX) return 0;
	KUSOUND_SOURCE *source = &kuSoundSources[sourceID];
	//if(!source->isInitialized) return NULL;
	return source->numInstances;
}

int kuSoundStopInstances(int sourceID, int numAllowed, var stopSpeed)
{
	if(sourceID < 0 || sourceID >= KUSOUND_SOURCES_MAX) return 0;
	KUSOUND_SOURCE *source = &kuSoundSources[sourceID];
	if(source->numInstances <= numAllowed) return 0;
	KUSOUND *sound = source->pFirstInstance;
	int numActive = 0;
	while(sound)
	{
		if(!(sound->flags & KUSOUND_FLAG_IS_INVALID))
		{
			if(sound->fadeSpeed > 0) numActive++;
		}
		sound = sound->next;
	}
	if(numActive <= numAllowed) return 0;
	int numStopWanted = numActive-numAllowed;
	int numStopped = 0;
	KUSOUND *sound = source->pFirstInstance;
	while(sound && numStopped < numStopWanted)
	{
		if(!(sound->flags & KUSOUND_FLAG_IS_INVALID))
		{
			if(sound->fadeSpeed > 0)
			{
				numStopped++;
				sound->fadeSpeed = stopSpeed;
			}
		}
		sound = sound->next;
	}
}
