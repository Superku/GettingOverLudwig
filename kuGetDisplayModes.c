//////////////////////////////
// kuGetDisplayModes.c
//////////////////////////////
// helper functions

MONITOR_HELPER_LIST* monitorHelperListAdd(MONITOR_HELPER_LIST* list, void* data)
{
	MONITOR_HELPER_LIST *listNew = (MONITOR_HELPER_LIST*)sys_malloc(sizeof(MONITOR_HELPER_LIST));
	listNew->data = data;
	listNew->next = list;
	return listNew;
}

void monitorHelperListDestroy(MONITOR_HELPER_LIST* list, int mode)
{
	while(list)
	{
		if(mode) sys_free(list->data);
		MONITOR_HELPER_LIST* prev = list;
		list = list->next;
		sys_free(prev);
	}
}

int MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, RECT *lprcMonitor, LPARAM dwData)
{
	int *Count = (int*)dwData;
	
	//MONITORINFO iMonitor;
	//iMonitor.cbSize = sizeof(MONITORINFO);
	MONITORINFOEX iMonitor;
	iMonitor.cbSize = sizeof(MONITORINFOEX);
	GetMonitorInfo(hMonitor, &iMonitor);
	//printf("monitor %d: %s, hMonitor(%d), rect(%d,%d,%d,%d)", (*Count), iMonitor.szDevice, hMonitor, lprcMonitor->left, lprcMonitor->right, lprcMonitor->top, lprcMonitor->bottom); // returns DISPLAY6, DISPLAY7, DISPLAY8
	if(iMonitor.dwFlags & DISPLAY_DEVICE_MIRRORING_DRIVER) return 1;
	if(iMonitor.dwFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP) multiMonitorInfo.currentAdapterNum = *Count;
	
	//MYMONITORINFO *pinfo = &((multiMonitorInfo.monitorInfo)[*Count]);
	MYMONITORINFO *pinfo = NULL;
	int j;
	for(j = 0; j < multiMonitorInfo.numMonitors; j++)
	{
		MYMONITORINFO *pinfo2 = &((multiMonitorInfo.monitorInfo)[j]);
		if(str_cmpi(iMonitor.szDevice, pinfo2->DeviceName))
		{
			//printf("found monitor %s in slot j(%d) at count(%d)", iMonitor.szDevice, j, (*Count));
			pinfo = pinfo2;
			break;
		}
	}
	if(pinfo)
	{
		memcpy(&pinfo->monitorRect, lprcMonitor, sizeof(RECT));
		pinfo->flags = iMonitor.dwFlags;
		pinfo->hmonitor = hMonitor;
		int thisWidth = lprcMonitor->right - lprcMonitor->left;
		if( thisWidth > 0) pinfo->appScaling = pinfo->width * 100 / thisWidth;
		else pinfo->appScaling = 100;
	}

	(*Count)++;
	if(*Count >= multiMonitorInfo.numMonitors) return 0;
	return 1;
}

//////////////////////////////

#define ITF (int)total_frames

char* MMInfoGetGraphicscardName(int i)
{
	if(i < 0 || i >= multiMonitorInfo.numGraphicscards || !multiMonitorInfo.cGraphicscards)
	{
		if(i < 0) mmInfoLastError = 1;
		if(i >= multiMonitorInfo.numGraphicscards) mmInfoLastError = 2;
		if(!multiMonitorInfo.cGraphicscards) mmInfoLastError = 3;
		cprintf3("\n MMInfoGetGraphicscardName(%d) at frame %d: NULL! errorCode(%d)", i, ITF, mmInfoLastError);
		return NULL;
	}
	return (multiMonitorInfo.cGraphicscards)[i];
}

MYMONITORINFO* MMInfoGetMonitorInfo(int monitorID)
{
	if(monitorID < 0 || monitorID >= multiMonitorInfo.numMonitors || !multiMonitorInfo.monitorInfo)
	{
		if(monitorID < 0) mmInfoLastError = 4;
		if(monitorID >= multiMonitorInfo.numMonitors) mmInfoLastError = 5;
		if(!multiMonitorInfo.monitorInfo) mmInfoLastError = 6;
		cprintf3("\n MMInfoGetMonitorInfo(%d) at frame %d: NULL! errorCode(%d)", monitorID, ITF, mmInfoLastError);
		return NULL;
	}
	return &(multiMonitorInfo.monitorInfo)[monitorID];
}

int MMInfoGetDisplayNumModes(int monitorID)
{
	if(monitorID < 0)
	{
		MYMONITORINFO *combInfo = multiMonitorInfo.monitorInfoCombined;
		if(combInfo) return combInfo->numModes;
	}
	if(monitorID < 0 || monitorID >= multiMonitorInfo.numMonitors || !multiMonitorInfo.monitorInfo)
	{
		if(monitorID < 0) mmInfoLastError = 7;
		if(monitorID >= multiMonitorInfo.numMonitors) mmInfoLastError = 8;
		if(!multiMonitorInfo.monitorInfo) mmInfoLastError = 9;
		cprintf3("\n MMInfoGetDisplayNumModes(%d) at frame %d: NULL! errorCode(%d)", monitorID, ITF, mmInfoLastError);
		return 0;
	}
	MYMONITORINFO *pinfo = &(multiMonitorInfo.monitorInfo)[monitorID];
	return pinfo->numModes;
}

DISPLAYMODE* MMInfoGetDisplayMode(int monitorID, int modeID)
{
	if(monitorID < 0)
	{
		MYMONITORINFO *combInfo = multiMonitorInfo.monitorInfoCombined;
		if(combInfo)
		{
			modeID = minv(modeID, combInfo->numModes-1);
			if(modeID < 0 || modeID >= combInfo->numModes)
			{
				if(modeID < 0) mmInfoLastError = 13;
				if(modeID >= combInfo->numModes) mmInfoLastError = 14;
				return NULL;
			}
			return &(combInfo->displayModes)[modeID];
		}
	}
	if(monitorID < 0 || monitorID >= multiMonitorInfo.numMonitors || !multiMonitorInfo.monitorInfo)
	{
		if(monitorID < 0) mmInfoLastError = 10;
		if(monitorID >= multiMonitorInfo.numMonitors) mmInfoLastError = 11;
		if(!multiMonitorInfo.monitorInfo) mmInfoLastError = 12;
		cprintf4("\n MMInfoGetDisplayMode(%d, %d) at frame %d: NULL! errorCode(%d)", monitorID, modeID, ITF, mmInfoLastError);
		return NULL;
	}
	MYMONITORINFO *pinfo = &(multiMonitorInfo.monitorInfo)[monitorID];
	modeID = minv(modeID, pinfo->numModes-1);
	if(modeID < 0 || modeID >= pinfo->numModes)
	{
		if(modeID < 0) mmInfoLastError = 13;
		if(modeID >= pinfo->numModes) mmInfoLastError = 14;
		cprintf4("\n MMInfoGetDisplayMode(%d, %d) at frame %d: NULL! errorCode(%d)", monitorID, modeID, ITF, mmInfoLastError);
		return NULL;
	}
	return &(pinfo->displayModes)[modeID];
}

void MMInfoSortDisplayModes(MYMONITORINFO *pinfo, int mode_unused)
{
	int i,j;
	if(!pinfo->numModes || !pinfo->displayModes) return;
	
	for(i = 0; i < pinfo->numModes; i++)
	{
		int currentWidth = (pinfo->displayModes)[i].Width;
		int currentHeight = (pinfo->displayModes)[i].Height;
		for(j = i; j > 0; j--)
		{
			if((pinfo->displayModes)[j-1].Height <= currentHeight) break;
			(pinfo->displayModes)[j].Width = (pinfo->displayModes)[j-1].Width;
			(pinfo->displayModes)[j].Height = (pinfo->displayModes)[j-1].Height;
		}			
		(pinfo->displayModes)[j].Width = currentWidth;
		(pinfo->displayModes)[j].Height = currentHeight;
	}
}

BOOL EnumDisplaySettingsExA(LPCSTR   lpszDeviceName, DWORD    iModeNum, DEVMODEA *lpDevMode, DWORD    dwFlags);

int multiMonitorGetInfo()
{
	int Count = 0;

	LPDIRECT3D9 pd3dKu = (LPDIRECT3D9)pd3d;
	if(!pd3dKu)
	{
		error("multiMonitorGetInfo(): (LPDIRECT3D9)pd3d == NULL!");
		multiMonitorInfo.numMonitors = -1;
		return multiMonitorInfo.numMonitors;
	}

	// reset

	if(multiMonitorInfo.monitorInfo)
	{
		int i;
		for(i = 0; i < multiMonitorInfo.numMonitors; i++)
		{
			if((multiMonitorInfo.monitorInfo)[i].displayModes) sys_free((multiMonitorInfo.monitorInfo)[i].displayModes);
		}
		sys_free(multiMonitorInfo.monitorInfo);
		multiMonitorInfo.monitorInfo = NULL;
	}
	/*if(multiMonitorInfo.monitorInfo2)
	{
		sys_free(multiMonitorInfo.monitorInfo2);
		multiMonitorInfo.monitorInfo2 = NULL;
	}*/
	// dont free monitorInfoCombined
	
	if(multiMonitorInfo.cGraphicscards)
	{
		int i;
		for(i = 0; i < multiMonitorInfo.numGraphicscards; i++)
		{
			if((multiMonitorInfo.cGraphicscards)[i]) sys_free((multiMonitorInfo.cGraphicscards)[i]);
		}
		sys_free(multiMonitorInfo.cGraphicscards);
		multiMonitorInfo.cGraphicscards = NULL;
		multiMonitorInfo.numGraphicscards = 0;
	}

	///////////////////////////////
	// setup base data

	multiMonitorInfo.numMonitors = GetSystemMetrics(SM_CMONITORS);
	if(multiMonitorInfo.numMonitors)
	{
		multiMonitorInfo.monitorInfo = (MYMONITORINFO*)sys_malloc(sizeof(MYMONITORINFO)*multiMonitorInfo.numMonitors);
		memset(multiMonitorInfo.monitorInfo, 0, sizeof(MYMONITORINFO)*multiMonitorInfo.numMonitors);

		//multiMonitorInfo.monitorInfo2 = (MYMONITORINFO*)sys_malloc(sizeof(MYMONITORINFO)*multiMonitorInfo.numMonitors);
		//memset(multiMonitorInfo.monitorInfo2, 0, sizeof(MYMONITORINFO)*multiMonitorInfo.numMonitors);
	}

	multiMonitorInfo.numGraphicscards = 0;
	MONITOR_HELPER_LIST *list = NULL;
	DISPLAY_DEVICEA device;
	device.cb = sizeof(DISPLAY_DEVICEA);
	int num = 0, monitorNum = 0; // keyword: primary
	while(EnumDisplayDevices(NULL,num,&device,0))
	{
		char *cname = device.DeviceString;
		// printf(cname); spuckt gtx 1080 aus
		//device.cb = sizeof(DISPLAY_DEVICEA);
		//EnumDisplayDevices(device.DeviceName, 0, &device, 0);
		//printf(device.DeviceString);
		int clen = strlen(cname);
		if(clen >= 2)
		{
			if(!(cname[0] == 'R' && cname[1] == 'D'))
			{
				if((device.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP) && monitorNum < multiMonitorInfo.numMonitors)
				{
					devmodeAInstance.dmSize = sizeof(DEVMODEA);
					devmodeAInstance.dmDriverExtra = 0;
					EnumDisplaySettings(device.DeviceName,ENUM_CURRENT_SETTINGS,&devmodeAInstance);
					//printf("DeviceString(%s) DeviceName(%s) w(%d)h(%d), pos(%d,%d)", device.DeviceString, device.DeviceName, devmodeAInstance.dmPelsWidth, devmodeAInstance.dmPelsHeight, devmodeAInstance.dmPosition.x, devmodeAInstance.dmPosition.y);
					//printf("%s: w(%d)h(%d), pos(%d,%d)", device.DeviceName, devmodeAInstance.dmPelsWidth, devmodeAInstance.dmPelsHeight, devmodeAInstance.dmPosition.x, devmodeAInstance.dmPosition.y);
					//printf("%s: w(%d)h(%d), pos(%d,%d), DPI?(%d)", device.DeviceName, devmodeAInstance.dmPelsWidth, devmodeAInstance.dmPelsHeight, devmodeAInstance.dmPosition.x, devmodeAInstance.dmPosition.y, (int)devmodeAInstance.);
					MYMONITORINFO *pinfo = &((multiMonitorInfo.monitorInfo)[monitorNum]);
					if(clen < 32) strcpy(pinfo->DeviceName, device.DeviceName);
					pinfo->frequency = devmodeAInstance.dmDisplayFrequency;
					pinfo->width = devmodeAInstance.dmPelsWidth;
					pinfo->height = devmodeAInstance.dmPelsHeight;
					pinfo->posX = devmodeAInstance.dmPosition.x;
					pinfo->posY = devmodeAInstance.dmPosition.y;
					// menu_monitor_identification
					pinfo->rect.left = pinfo->posX;
					pinfo->rect.right = pinfo->posX + pinfo->width;
					pinfo->rect.top = pinfo->posY;
					pinfo->rect.bottom = pinfo->posY + pinfo->height;
					if(device.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE)
					{
						primMonitorID = monitorNum;
					}
					monitorNum++;
				}
				int doAdd = 1;
				MONITOR_HELPER_LIST *tmpList = list;
				while(tmpList)
				{
					char *cTest = (char*)tmpList->data;
					if(str_cmpni(cname,cTest))
					{
						doAdd = 0;
						break;
					}
					tmpList = tmpList->next;
				}
				if(doAdd)
				{
					char *cGraNameNew = (char*)sys_malloc(sizeof(char)*(clen+1));
					memcpy(cGraNameNew,cname,clen+1);
					list = monitorHelperListAdd(list, cGraNameNew);
					multiMonitorInfo.numGraphicscards++;
				}
			}
		}
		num++;
	}
	if(multiMonitorInfo.numGraphicscards)
	{
		multiMonitorInfo.cGraphicscards = (char**)sys_malloc(sizeof(char*)*multiMonitorInfo.numGraphicscards);
		MONITOR_HELPER_LIST *tmpList = list;
		int i = 0;
		while(tmpList)
		{
			if(i >= multiMonitorInfo.numGraphicscards) error("i >= multiMonitorInfo.numGraphicscards!");
			(multiMonitorInfo.cGraphicscards)[i] = tmpList->data;
			i++;
			tmpList = tmpList->next;
		}
		monitorHelperListDestroy(list,0);
	}
	else monitorHelperListDestroy(list,1);
	list = NULL;


	///////////////////////////////

	LPDIRECT3DDEVICE9 pd3dDevKu = (LPDIRECT3DDEVICE9)pd3ddev;
	D3DDEVICE_CREATION_PARAMETERS creationParameters;
	pd3dDevKu->GetCreationParameters(creationParameters);
	multiMonitorInfo.launchAdapterNum = creationParameters.AdapterOrdinal; // -mon X

	///////////////////////////////

	resolution.dmSize = sizeof(SCRMODE);
	if(EnumDisplaySettings(NULL,ENUM_CURRENT_SETTINGS,&resolution))
	{
		multiMonitorInfo.currentWidth = resolution.dmPelsWidth;
		multiMonitorInfo.currentHeight = resolution.dmPelsHeight;
		multiMonitorInfo.currentFrequency = resolution.dmDisplayFrequency;
		//fullscreen_maxfps = multiMonitorInfo.currentFrequency = resolution.dmDisplayFrequency;
		//cprintf1("\nmultiMonitorGetInfo: resolution.dmDisplayFrequency(%d)",(int)resolution.dmDisplayFrequency);
	}
	else
	{
		multiMonitorInfo.currentWidth = -1;
		multiMonitorInfo.currentHeight = -1;
		multiMonitorInfo.currentFrequency = -1;
		//fullscreen_maxfps = 60;
		cprintf0("\nmultiMonitorGetInfo: WARNING: could not retrieve current display settings!");
	}



	#ifdef DISPLAY_MODES_COMBINED
	if(!multiMonitorInfo.monitorInfoCombined)
	{
		multiMonitorInfo.monitorInfoCombined = (MYMONITORINFO*)sys_malloc(sizeof(MYMONITORINFO));
		multiMonitorInfo.monitorInfoCombined->displayModes = (DISPLAYMODE*)sys_malloc(sizeof(DISPLAYMODE)*50);
	}
	#endif

	if(multiMonitorInfo.numMonitors)
	{
		int count = 0;
		// get screen rects
		EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, (LPARAM)&count);

		// get display modes
		D3DDISPLAYMODE displayMode;
		int i,j, monitorID, previousWidth = 0, previousHeight = 0;
		for(monitorID = 0; monitorID < multiMonitorInfo.numMonitors; monitorID++)
		{
			i = monitorID;
			pd3dKu->GetAdapterIdentifier( i, 0, &d3dAdapterIdentifier);
			//printf("d3d device name %d): %s", i, d3dAdapterIdentifier.DeviceName);
			for(j = 0; j < multiMonitorInfo.numMonitors; j++)
			{
				MYMONITORINFO *pinfo = &((multiMonitorInfo.monitorInfo)[j]);
				if(str_cmpi(d3dAdapterIdentifier.DeviceName, pinfo->DeviceName))
				{
					//printf("changing i(%d) to j(%d)", i, j);
					i = j;
					break;
				}
			}
			//printf("monitorID(%d) i(%d)", monitorID, i);
			/*if(!monitorID) i = primMonitorID;
			else
			{
				if(monitorID <= primMonitorID) i = monitorID-1;
				else i = monitorID;
			}*/
			//cprintf1("\n\n\n MONITOR %d: ", i);
			MYMONITORINFO *pinfo = &(multiMonitorInfo.monitorInfo)[monitorID];
			int maxModes = pd3dKu->GetAdapterModeCount(i,D3DFMT_X8R8G8B8);
			for(j = 0; j < maxModes; j++)
			{
				//if(!j) cprintf2(" pos(%d,%d)", i);
				if(D3D_OK != pd3dKu->EnumAdapterModes(i,D3DFMT_X8R8G8B8,j,&displayMode)) break;
				//if(displayMode.Width >= 1020 && displayMode.Height >= 600 && displayMode.RefreshRate == multiMonitorInfo.currentFrequency) pinfo->numModes++;
				if(displayMode.Width >= 1020 && displayMode.Height >= 600 && (previousWidth != displayMode.Width || previousHeight != displayMode.Height) )
				{
					previousWidth = displayMode.Width;
					previousHeight = displayMode.Height;
					pinfo->numModes++;
				}
			}
			previousWidth = previousHeight = 0;
			if(pinfo->numModes)
			{
				pinfo->displayModes = (DISPLAYMODE*)sys_malloc(sizeof(DISPLAYMODE)*pinfo->numModes);
				int k = 0;
				for(j = 0; j < maxModes; j++)
				{
					if(D3D_OK != pd3dKu->EnumAdapterModes(i,D3DFMT_X8R8G8B8,j,&displayMode)) break;
					if(displayMode.Width >= 1020 && displayMode.Height >= 600 && (previousWidth != displayMode.Width || previousHeight != displayMode.Height) )
					{
						previousWidth = displayMode.Width;
						previousHeight = displayMode.Height;
						DISPLAYMODE* pMode = &(pinfo->displayModes)[k];
						pMode->Width = displayMode.Width;
						pMode->Height = displayMode.Height;
						k++;
					}
				}
			}
			else pinfo->displayModes = NULL;
			MMInfoSortDisplayModes(pinfo,0);
		}
		#ifdef DISPLAY_MODES_COMBINED
			MYMONITORINFO *combInfo = multiMonitorInfo.monitorInfoCombined;
			combInfo->numModes = 0;
			for(i = 0; i < multiMonitorInfo.numMonitors; i++)
			{
				MYMONITORINFO *pinfo = &(multiMonitorInfo.monitorInfo)[i];
				for(j = 0; j < pinfo->numModes; j++)
				{
					DISPLAYMODE* newMode = &(pinfo->displayModes)[j];
					int k, addOkay = 1;
					for(k = 0; k < combInfo->numModes; k++)
					{
						DISPLAYMODE* testMode = &(combInfo->displayModes)[k];
						if(testMode->Width == newMode->Width && testMode->Height == newMode->Height)
						{
							addOkay = 0;
							break;
						}
					}
					if(addOkay)
					{
						DISPLAYMODE* pMode = &(combInfo->displayModes)[combInfo->numModes];
						pMode->Width = newMode->Width;
						pMode->Height = newMode->Height;
						combInfo->numModes++;
						if(combInfo->numModes >= 50) break;
					}
				}
				if(combInfo->numModes >= 50) break;
			}
			MMInfoSortDisplayModes(combInfo,0);
		#endif
	}

	//#ifdef DISPLAY_MODES_COMBINED
		MYMONITORINFO *combInfo = multiMonitorInfo.monitorInfoCombined;
		if(combInfo)
		{
			int width = sys_metrics(0);
			int height = sys_metrics(1);
			int i;
			for(i = 0; i < combInfo->numModes; i++)
			{
				DISPLAYMODE* mode = &(combInfo->displayModes)[i];
				if(mode->Width == width && mode->Height == height)
				{
					resolutionIDWanted = i;
					break;
				}
			}
		}
	//#endif

	

	D3DADAPTER_IDENTIFIER9 identifier;
	pd3dKu->GetAdapterIdentifier(multiMonitorInfo.currentAdapterNum,0,&identifier);
	//int adapterCount = pd3dKu->GetAdapterCount();
	//draw_text(str_printf(NULL,"adapterCount: %d",adapterCount),400,60,COLOR_RED);
	multiMonitorInfo.currentGraphicscard = -1;
	int i;
	for(i = 0; i < multiMonitorInfo.numGraphicscards; i++)
	{
		if(str_cmpni((multiMonitorInfo.cGraphicscards)[i],identifier.Description))
		{
			multiMonitorInfo.currentGraphicscard = i;
			break;
		}
	}

	return multiMonitorInfo.numMonitors;
}
