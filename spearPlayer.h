//////////////////////////////
// spearPlayer.h
//////////////////////////////

typedef struct _SPEARPLAYERDATA
{
    // pos is based on ENTITY position (because we use the engine's native collision detection function to move it)
    VECTOR speed;
    VECTOR contactPos;
    var turnSpeed;
    var angle;
    var contactTime;
    var boostReady, boostDuration;
} SPEARPLAYERDATA;
SPEARPLAYERDATA *playerDataPointer = NULL;

typedef struct _SPEARPLAYER
{
    ENTITY* entSpear; // this is the main object we control
    ENTITY* entCharacter; // this is decoration
    ENTITY* entSpearWoosh;
} SPEARPLAYER;
SPEARPLAYER spearPlayer;

typedef struct _SPEARGAME
{
    SPEARPLAYERDATA playerData;
    VECTOR spawnPos;
    int timeInteger;
    float timeFloat;
    var timeCombined;
    int numJumps, numBoosts;
    // other level states that need to be reset
} SPEARGAME;
SPEARGAME gameData;

