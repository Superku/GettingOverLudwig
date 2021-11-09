//////////////////////////////
// spearTypes.h
//////////////////////////////

#define GAMESTATE_LAUNCH 0
#define GAMESTATE_PLAY 1
int gameState = GAMESTATE_PLAY;

int menuActive = 0;
int menuClosedOnce = 0;
int menuIntroState = 0;
var menuIntroStateProgress = 0;
var menuOpenPerc = 0;
var menuPagePerc = 0;
var menuTitleScreenPerc = 0;
var menuTitleHidePerc = 100;
var menuFullscreenWanted = 1;
var screenSizeWantedX = 1280;
var screenSizeWantedY = 720;
var showFPS = 1;
var showTimer = 1;
var backgroundRenderMode = 2;
var playerFrozen = 0; // for in-game editing/ painting
var cameraWidth = 520; // was: 500 (24.10.)  |  540 (25.10.)
var playerLeftRightInverted = 0;
var deathPercent = 0;
int outroState = 0;
var outroStateProgress = 0;

ENTITY* entThickHullTrace = NULL;

#define ITF (int)total_frames

VECTOR playerPosLocal; // save in global vector for easy access and use in shaders

FONT* cached_fnt = "Courier New#30b";
//FONT *UIFont = "Courier New#20b";
FONT* introFont = "Courier New#50b";

STRING *tmpStr = "";
STRING *tmpStr2 = "";
STRING *introStr = "";



