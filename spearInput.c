//////////////////////////////
// spearInput.c
//////////////////////////////


void inputUpdate(int inputAllowed, int menuActive)
{
	int j;
	var inputTypeActiveNew = -1;
	ackXInputGetState3(); // retrieve xbox gamepad button and analog stick states

	float forceX = (float)ackXInputGetThumbState3(0, 0)/255.0;
	float forceY = (float)ackXInputGetThumbState3(0, 1)/255.0;
	if(fabs(forceX) < inputDeadzone) forceX = 0;
	if(fabs(forceY) < inputDeadzone) forceY = 0;

	KUINPUT_BUTTON* button = &kuInputInstance.buttons[KUINPUT_BUTTON_UP];
	button->value = (key_w || key_cuu) + fmax(forceY, 0) + ackXInputGetButtonState3(0);

	KUINPUT_BUTTON* button = &kuInputInstance.buttons[KUINPUT_BUTTON_DOWN];
	button->value = (key_s || key_cud) + fmax(-forceY, 0) + ackXInputGetButtonState3(1);
	
	KUINPUT_BUTTON* button = &kuInputInstance.buttons[KUINPUT_BUTTON_LEFT];
	button->value = (key_a || key_cul) + fmax(-forceX, 0) + ackXInputGetButtonState3(2);
	
	KUINPUT_BUTTON* button = &kuInputInstance.buttons[KUINPUT_BUTTON_RIGHT];
	button->value = (key_d || key_cur) + fmax(forceX, 0) + ackXInputGetButtonState3(3);

	KUINPUT_BUTTON* button = &kuInputInstance.buttons[KUINPUT_BUTTON_JUMP];
	button->value = (key_space || key_enter*menuActive || mouse_left*!menuActive) + ackXInputGetButtonState3(12);

	KUINPUT_BUTTON* button = &kuInputInstance.buttons[KUINPUT_BUTTON_RETRY];
	button->value = (key_r) + ackXInputGetButtonState3(5);

	KUINPUT_BUTTON* button = &kuInputInstance.buttons[KUINPUT_BUTTON_BOOST];
	button->value = (key_shift || key_ctrl) + ackXInputGetButtonState3(14);
	
	KUINPUT_BUTTON* button = &kuInputInstance.buttons[KUINPUT_BUTTON_OPENMENU];
	button->value = (key_esc*!menuActive) + ackXInputGetButtonState3(4);

	KUINPUT_BUTTON* button = &kuInputInstance.buttons[KUINPUT_BUTTON_BACKMENU];
	button->value = (key_esc*menuActive) + ackXInputGetButtonState3(13);
	

	if(key_any || mouse_moving)
	{
		if(forceX > 0.5 || forceY > 0.5 || ackXInputGetButtonState3(5) || ackXInputGetButtonState3(12)) inputTypeActiveNew = 1;
		else inputTypeActiveNew = 0;
	}

	for(j = 0; j < KUINPUT_BUTTON_NUM; j++)
	{
		KUINPUT_BUTTON* button = &kuInputInstance.buttons[j];
		if(button->value > 1) button->value = 1;
		button->isDown = (button->value > 0);
		button->justPressed = (button->isDown && !button->previousDown);
		button->previousDown = button->isDown;
		if(button->isDown)
		{
			if(button->justPressed) button->recentlyPressed = 0;
			else button->recentlyPressed = minv(button->recentlyPressed+time_step, 64);
		}
		else button->recentlyPressed = 64;
	}

	inputAnyKeyPressed = (inputTypeActiveNew >= 0);
	if(inputTypeActiveNew != -1)
	{		
		inputTypeActive = inputTypeActiveNew;
		inputIdleTimer = 0;
	}
	else inputIdleTimer += time_frame;
}

void inputSetButtonsOff()
{
	int j;
	for(j = 0; j < KUINPUT_BUTTON_NUM; j++)
	{
		KUINPUT_BUTTON* button = &kuInputInstance.buttons[j];
		button->value = 0;
		button->isDown = 0;
		button->previousDown = button->isDown;
		button->recentlyPressed = 0;
	}
}

int inputIsDown(int buttonId)
{
	if(inputDisabled) return 0;
	return (kuInputInstance.buttons[buttonId].isDown);
}

int inputJustPressed(int buttonId)
{
	if(inputDisabled) return 0;
	return (kuInputInstance.buttons[buttonId].justPressed);
}

int inputJustPressed2(int buttonId)
{
	if(inputDisabled) return 0;
	int justPressed = kuInputInstance.buttons[buttonId].justPressed;
	kuInputInstance.buttons[buttonId].justPressed = 0;
	return justPressed;
}

int inputRecentlyPressed(int buttonId, var maxTime) // unused?
{
	if(inputDisabled) return 0;
	if(!kuInputInstance.buttons[buttonId].isDown) return 0;
	return (kuInputInstance.buttons[buttonId].recentlyPressed <= maxTime);
}

var inputRecentlyPressedGetTime(int buttonId)
{
	if(inputDisabled) return 0;
	if(!kuInputInstance.buttons[buttonId].isDown) return 0;
	return kuInputInstance.buttons[buttonId].recentlyPressed;
}

var inputGetValue(int buttonId)
{
	if(inputDisabled) return 0;
	return (kuInputInstance.buttons[buttonId].value);
}
