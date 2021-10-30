//////////////////////////////
// kuSoundSpear.h
// stripped down version of my custom sound engine (DirectSound8 implementation)
// because the engine sound is terrible (way worse than this implementation) and does not even support stereo sounds!!!
//////////////////////////////

#ifndef KUSOUND_H
	#define KUSOUND_H

	//#define KUSOUND void
	#define IDirectSoundBuffer8 void
	#define bool int
	#define true 1
	#define false 0
	#define DWORD int

	//////////////////////////////
	// DLL functions

	var kuSoundTest();

	int kuSoundInit(HWND hwnd, int *pSetCoopLevelResult);

	// must be 44.1k 16bit Stereo (or mono?) PCM Wave
	IDirectSoundBuffer8* kuSoundCreateDSB8FromFile(char* wavFileName);
	IDirectSoundBuffer8* kuSoundCreateDSB8FromBuffer(void* buffer);
	IDirectSoundBuffer8* kuSoundDuplicateDSB8(void *pDSBufferOriginal, int* pIErrorCode);
	IDirectSoundBuffer8* kuSoundCreateDSB8FromOggBuffer(void* oggBuffer, int oggBufferLen);

	void kuSoundDSB8Release(IDirectSoundBuffer8 *pSecondaryBuffer);

	void kuSoundDSB8Play(IDirectSoundBuffer8 *pSecondaryBuffer, bool loop);

	void kuSoundDSB8Stop(IDirectSoundBuffer8 *pSecondaryBuffer);

	#define DSBSTATUS_PLAYING           0x00000001
	#define DSBSTATUS_BUFFERLOST        0x00000002
	#define DSBSTATUS_LOOPING           0x00000004
	#define DSBSTATUS_LOCHARDWARE       0x00000008
	#define DSBSTATUS_LOCSOFTWARE       0x00000010
	#define DSBSTATUS_TERMINATED        0x00000020
	int kuSoundDSB8GetStatus(IDirectSoundBuffer8 *pSecondaryBuffer);

	int kuSoundDSB8GetCurrentPosition(IDirectSoundBuffer8 *pSecondaryBuffer, DWORD *pTotalLength);

	int kuSoundDSB8SetCurrentPosition(IDirectSoundBuffer8 *pSecondaryBuffer, DWORD pdwNewPosition);

	void kuSoundDSB8SetVolume(IDirectSoundBuffer8 *pSecondaryBuffer, float volume);

	void kuSoundDSB8SetPan(IDirectSoundBuffer8 *pSecondaryBuffer, float pan);

	void kuSoundDSB8SetSpeed(IDirectSoundBuffer8 *pSecondaryBuffer, float speed);

	int kuSoundReadBlockFromFile(char *cFileName, char* buffer, long size, long offset);
	
	int kuSoundReadFromFile(char *cFileName, char* buffer, long size);
	
	//////////////////////////////////////////////

	#define KUSOUND_SPEED_1 1.0
	#define KUSOUND_RANGE_1 1.0

	#define KUSOUND_STOP_SPEED_VERYSLOW -2
	#define KUSOUND_STOP_SPEED_SLOW -10
	#define KUSOUND_STOP_SPEED_NORMAL -25
	#define KUSOUND_STOP_SPEED_FAST -50

	#define KUSOUND_FLAG_LEVELSOUND (1<<0)
	#define KUSOUND_FLAG_AFFECTED_BY_SLOWMO (1<<1)
	#define KUSOUND_FLAG_LOOPS (1<<2)
	#define KUSOUND_FLAG_FADEOUT (1<<3)
	#define KUSOUND_FLAG_FADEIN (1<<4)
	#define KUSOUND_FLAG_NOTAFFECTEDBYFOCUS (1<<5)
	#define KUSOUND_FLAG_NOAUTOUPDATE (1<<6)
	#define KUSOUND_FLAG_IS_MUSIC (1<<7)
	#define KUSOUND_FLAG_IS_AMBIENT (1<<8)
	#define KUSOUND_FLAG_STOP_ON_LEVELCHANGE (1<<9)
	#define KUSOUND_FLAG_IS_INVALID (1<<10) // sound will be removed next frame - read only
	#define KUSOUND_FLAG_NOSPEEDOVERWRITE (1<<11)
	#define KUSOUND_FLAG_USETIMER (1<<12)
	#define KUSOUND_FLAG_IS_PAUSED (1<<13)
	//#define KUSOUND_FLAG_IS_OGG (1<<14)
	#define KUSOUND_FLAG_PAUSES_ON_MENU (1<<15)
	#define KUSOUND_FLAG_FADEOUT_W_SPEED (1<<16)
	#define KUSOUND_FLAG_SMOOTH_FALLOFF (1<<17)
	#define KUSOUND_FLAG_LOAD_ON_DEMAND (1<<18)
	#define KUSOUND_FLAG_DONT_FREE_BUFFER (1<<19)
	struct _KUSOUND
	{
		IDirectSoundBuffer8* dsb8;
		int sourceID;
		int playStatus;
		int flags;
		VECTOR pos;
		float volume,volFacPrev;
		float speed;
		float pan;
		float range;
		var fadePerc;
		var fadeSpeed;
		var playTime,timerDuration,timerFadeoutSpeed;
		int length, playPosWanted;
		void **pointer;
		struct _KUSOUND *next;
		struct _KUSOUND *nextInstance;
	};
	typedef struct _KUSOUND KUSOUND;

	struct _KUSOUND_SOURCE
	{
		int id;
		int type;
		int fileSize;
		int isLoaded;
		int preLoadWanted;
		int memoryType;
		int isInitialized;
		int lastError;
		int isOgg;
		int size, flags; // filesize / buffersize
		char cfileName[64];
		void* buffer;
		float pan;
		float volume;
		float frequency;
		IDirectSoundBuffer8* dsb8;
		int packID;
		int numInstances, maxInstances;
		KUSOUND *pFirstInstance;
	};
	typedef struct _KUSOUND_SOURCE KUSOUND_SOURCE;
	#define KUSOUND_PRELOAD_LOAD_ON_DEMAND -1
	#define KUSOUND_PRELOAD_ALWAYS 1 //(1<<0)
	#define KUSOUND_PRELOAD_WANTED 2 // (1<<1)
	#define KUSOUND_PRELOAD_MT 3 //(1<<2)
	#define KUSOUND_PRELOAD_MTACTIVE 4
	#define KUSOUND_PRELOAD_MTDONE 5
	
	#define KUSOUND_TYPE_MUSIC 1
	#define KUSOUND_TYPE_SFX 2
	#define KUSOUND_TYPE_AMBIENT 3

	KUSOUND* pKuSoundFirst = NULL;
	//KUSOUND* KUSOUND_MUSIC_ACTIVE = NULL;
	HANDLE kuSoundMutex, kuSoundWorkerHandle;
	int kuSoundUpdateNumSounds = 0;
	double kuSoundUpdateTime = 0;
	float kuSoundMasterVolume = 0.5;
	int kuSoundAllSoundsPaused = 0;
	int kuSoundInitErrorCode = 0;
	int kuSoundInitialized = 0;
	var kuSoundMenuSoundVolumePerc = 100;


	#define KS_MUSIC_MENU 0
	#define KS_SFX_INTRO_JUMP 1
	#define KS_SFX_DEATHSCARE 2
	#define KS_SFX_MENU_ACCEPT 3
	#define KS_SFX_MENU_BACK 4
	#define KS_SFX_MENU_CHANGE 5
	#define KS_SFX_MENU_SELECT 6
	#define KS_SFX_MENU_UPDOWN 7

	#define KS_AMBIENT_WIND0 8
	#define KS_AMBIENT_WIND1 9
	#define KS_AMBIENT_WATER0 10
	#define KS_AMBIENT_DRONE0 11

	#define KS_SFX_SPEAR_HIT0 12
	#define KS_SFX_SPEAR_HIT1 13
	#define KS_SFX_SPEAR_LAUNCH0 14
	#define KS_SFX_SPEAR_LAUNCH1 15
	#define KS_SFX_PLAYER_IMPACT0 16
	#define KS_SFX_PLAYER_IMPACT1 17
	#define KS_SFX_PLAYER_IMPACT2 18
	#define KS_SFX_WIN 19
	#define KS_SFX_MENU_TONE0 20
	#define KS_SFX_MENU_TONE1 21
	#define KS_SFX_MENU_TONE2 22
	#define KS_SFX_MENU_TONE3 23
	#define KS_SFX_MENU_TONE4 24
	#define KS_SFX_MENU_OPEN 25
	#define KS_SFX_MENU_CLOSE 26
	#define KS_SFX_MENU_NEWGAME 27

	#define KUSOUND_SOURCES_MAX 28
	KUSOUND_SOURCE kuSoundSources[KUSOUND_SOURCES_MAX];

	KUSOUND* KUSOUND_INSTANCE_MUSIC = NULL;
	KUSOUND* KUSOUND_INSTANCE_AMBIENT0 = NULL;
	KUSOUND* KUSOUND_INSTANCE_AMBIENT1 = NULL;
	KUSOUND* KUSOUND_INSTANCE_AMBIENT2 = NULL;
	KUSOUND* KUSOUND_INSTANCE_AMBIENT_WATER = NULL;
	KUSOUND* KUSOUND_INSTANCE_AMBIENT_DRONE = NULL;
	

	//////////////////////////////

	#define KUSOUND_PAN_MID 0
	KUSOUND* kuSoundPlay2D(int sourceID, bool loop, float volume, float speed, float pan);

	#define ENT_KUSOUND skill92

	//////////////////////////////////////////////
	// script function prototypes
	
	void kuSoundFreeBuffer(int i);

	void kuSoundPreloadFiles();

	DWORD kuSoundThreadWorker(void *unused);

	int kuSoundThreadStart();

	void kuSoundSourceInit_IDTypeNamePreload(int id, int type, char* cfile, int preLoadWanted);

	void kupackRegistryLoad(char *cPackName);

	int kupackFindFile(char* cFileName);

	//char* kupackLoadBuffer(char* cFileName);
	int kupackLoadBuffer(KUSOUND_SOURCE *source);

	void kuSoundDestroy(KUSOUND* sound, int mode);

	KUSOUND* kuSoundCreate(IDirectSoundBuffer8 *dsb8, int sourceID, int flags, VECTOR *pos, float volume, float speed, float pan, float range);

	KUSOUND* kuSoundPlayAtPos(int sourceID, VECTOR *pos, bool loop, float volume, float speed, float range);

	KUSOUND* kuSoundPlay2D(int sourceID, bool loop, float volume, float speed, float pan);

	void kuSoundUpdatePos(KUSOUND* sound,VECTOR *pos);

	void kuSoundSetVolume(KUSOUND* sound, float volume);

	void kuSoundSetSpeed(KUSOUND* sound, float speed);

	void kuSoundStop(KUSOUND* sound, int speed);

	void kuSoundSetPointer(KUSOUND* sound, void **p);

	void kuSoundSetFlag(KUSOUND* sound, int flagsNew);

	void kuSoundPause(KUSOUND* sound, bool paused);

	int kuSoundSetPlayPos(KUSOUND* sound, int playPos);

	int kuSoundGetPlayPos(KUSOUND* sound);

	int kuSoundGetPlayLength(KUSOUND* sound);

	void kuSoundSetTimeout(KUSOUND* sound, var duration, var fadeoutSpeed);

	void kuSoundRequestPreload(int id);

	int kuSoundGetPreloadRequestStatus(int id);

	void kuSoundSetMasterVolumeIn100(float volume);

	void kuSoundUpdateFrame(VIEW* view);

	void kuSoundStopAllLevelSounds(int mode);

	void kuSoundDestroyDevice();

	void kuSoundSourcesInit_filesetup();

	void kuSoundSourcesInit();

	void kuSoundAutoMusicDo();
	
	int kuSoundGetNumInstances(int sourceID);

	int kuSoundStopInstances(int sourceID, int numAllowed, var stopSpeed);

	KUSOUND_SOURCE* kuSoundPreloadFileSpecificByID(int ID, int statusWanted);

#endif

//////////////////////////////
// kuSoundPogo.h
//////////////////////////////