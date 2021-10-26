//////////////////////////////
// spearInput.h
//////////////////////////////

var inputDeadzone = 0.075;
var inputDisabled = 0;
var inputTypeActive = 0;
var inputIdleTimer = 0;
var inputAnyKeyPressed = 0;

#define KUINPUT_BUTTON_UP 0
#define KUINPUT_BUTTON_DOWN 1
#define KUINPUT_BUTTON_LEFT 2
#define KUINPUT_BUTTON_RIGHT 3
#define KUINPUT_BUTTON_JUMP 4
#define KUINPUT_BUTTON_RETRY 5
#define KUINPUT_BUTTON_OPENMENU 6
#define KUINPUT_BUTTON_CAM_UP 7 // unused
#define KUINPUT_BUTTON_CAM_DOWN 8
#define KUINPUT_BUTTON_CAM_LEFT 9
#define KUINPUT_BUTTON_CAM_RIGHT 10
#define KUINPUT_BUTTON_BOOST 11 // experimental stuff, "new game plus content"
#define KUINPUT_BUTTON_BACKMENU 12
#define KUINPUT_BUTTON_NUM 13

typedef struct _KUINPUT_BUTTON
{
	int isDown, justPressed, previousDown;
	var value, recentlyPressed;
} KUINPUT_BUTTON;

typedef struct _KUINPUT
{
	KUINPUT_BUTTON buttons[KUINPUT_BUTTON_NUM];
} KUINPUT;

KUINPUT kuInputInstance;

