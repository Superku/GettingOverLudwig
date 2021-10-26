
WINDOW WINSTART
{
	TITLE "Getting over... Ludwig";
	SIZE 630,500;
	MODE IMAGE;	 // IMAGE
	BG_COLOR RGB(0,0,0);
	COMMAND "-nx 128"; // -nwnd  -ns
	PICTURE <splash.bmp>,OPAQUE,0,0;
	TEXT_STDOUT "Arial",RGB(80,50,150),6,432,220,68;
} 


PATH "models";
PATH "shaders";
