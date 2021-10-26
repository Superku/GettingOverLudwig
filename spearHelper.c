//////////////////////////////
// spearHelper.c
// some helper functions, partially copied from other projects - usually already present in other game engines
//////////////////////////////


void draw_point4d(VECTOR* pos, COLOR* color, var alpha, var size)
{
	VECTOR temp;

	vec_set(temp,pos);
	if(vec_to_screen(temp,camera))
	{
		draw_quad(NULL,vector(temp.x-size*0.5,temp.y-size*0.5,0),NULL,vector(size,size,0),NULL,color,alpha,0);
	}
}

void draw_line3d2(VECTOR* v1, VECTOR* v2, COLOR* color, var alpha)
{
	draw_line3d(v1,NULL,alpha);
	draw_line3d(v1,color,alpha);
	draw_line3d(v2,color,alpha);
}

void draw_line2(VECTOR* v1, VECTOR* v2, COLOR* color, var alpha)
{
	draw_line(v1,NULL,alpha);
	draw_line(v1,color,alpha);
	draw_line(v2,color,alpha);
}


void draw_line4d(VECTOR* pos, VECTOR* color, var alpha)
{
	VECTOR ltemp;
	
	if(!camera.right || !camera.bottom) return;
	if(camera.roll)
	{
		vec_set(ltemp,pos);
		//if(!vec_to_screen(ltemp,camera)) return;
		vec_to_screen(ltemp,camera);
	}
	else
	{
		ltemp.x = ((pos.x-camera.x)/camera.right*0.5+0.5)*screen_size.x;
		ltemp.y = ((pos.z-camera.z)/camera.bottom*0.5+0.5)*screen_size.y;
		ltemp.z = 0;
	}
	draw_line(ltemp,color,alpha);
}

void draw_line4d2(VECTOR* v1, VECTOR* v2, VECTOR* color, var alpha)
{
	draw_line4d(v1,NULL,alpha);
	draw_line4d(v1,color,alpha);
	draw_line4d(v2,color,alpha);
}

STRING *tmp_str = "";

void draw_num3d(var num, VECTOR* pos, var yoffset, COLOR* color)
{
	var size;
	VECTOR tmpPos;

	vec_set(tmpPos,pos);
	if(vec_to_screen(tmpPos,camera))
	{
		str_printf(tmp_str,"%.1f",(double)num);
		size = str_width(tmp_str,NULL);
		tmpPos.x -= size*0.5;
		tmpPos.y -= 10-yoffset;
		draw_text(tmp_str,tmpPos.x+1,tmpPos.y+1,COLOR_BLACK);
		draw_text(tmp_str,tmpPos.x,tmpPos.y,color);
	}
}

void draw_text2(STRING* str, var pos_x, var pos_y, COLOR* color)
{
	draw_text(str,pos_x+1,pos_y+1,COLOR_BLACK);
	draw_text(str,pos_x,pos_y,color);
}


void draw_text2center(STRING* str, var pos_x, var pos_y, COLOR* color)
{
	var width = str_width(str,NULL);
	draw_text(str,pos_x+1-width*0.5,pos_y+1,COLOR_BLACK);
	draw_text(str,pos_x-width*0.5,pos_y,color);
}

void draw_text2aright(STRING* str, var pos_x, var pos_y, COLOR* color)
{
	var width = str_width(str,NULL);
	draw_text(str,pos_x+1-width,pos_y+1,COLOR_BLACK);
	draw_text(str,pos_x-width,pos_y,color);
}

#define draw_text3(str, num, pos_x, pos_y)  draw_text2(str_printf(NULL,str,num), pos_x, pos_y, COLOR_WHITE)

void draw_text3d(STRING* str, VECTOR* pos, var yOffset, COLOR* color)
{
	VECTOR temp;
	
	vec_set(temp,pos);
	if(vec_to_screen(temp,camera))
	{
		draw_text(str,temp.x+1,temp.y+yOffset+1,COLOR_BLACK);
		draw_text(str,temp.x,temp.y+yOffset,color);
	}
}

VECTOR draw3dLastPos;
void draw_text3dcenter(STRING* str, VECTOR* pos, var yOffset, COLOR* color)
{
	VECTOR temp;
	
	vec_set(temp,pos);
	if(vec_to_screen(temp,camera))
	{
		var width = str_width(str,NULL);
		draw_text(str,temp.x+1-width*0.5,temp.y+yOffset+1,COLOR_BLACK);
		draw_text(str,temp.x-width*0.5,temp.y+yOffset,color);
		draw3dLastPos.x = temp.x-width*0.5;
		draw3dLastPos.y = temp.y+yOffset;
		draw3dLastPos.z = 0;
	}
}

var cameraDist(VECTOR* vpos)
{
	var dx = abs(vpos.x-camera.x)-camera.right;
	var dz = abs(vpos.z-camera.z)-camera.top;
	if(dx < 0) dx = 0;
	if(dz < 0) dz = 0;
	return vec_length(vector(dx,0,dz));
}


var vec_dist2d(VECTOR* v1, VECTOR* v2)
{
	return vec_length(vector(v1.x-v2.x,0,v1.z-v2.z));
}

var vec_limit(VECTOR* v, var range)
{
	var length;
	
	length = vec_length(v);
	if(length > range)
	{
		vec_normalize(v,range);
		length = range;
	}
	
	return length;
}

var vec_limit2(VECTOR* v, var range)
{
	var length;
	
	length = vec_length(v);
	if(length > range) vec_normalize(v,range);
	
	return length;
}

void vec_randomize2(VECTOR* v, var a, var b)
{
	var length;
	
	length = a+random(b-a);
	vec_set(v,vector(random(2)-1,random(2)-1,random(2)-1));
	vec_normalize(v,length);
}


var sign2(var x)
{
	if(x < 0) return -1;
	return 1;
}

var ceil(var x)
{
	return -floor(-x);
}


var getFastYfromX2Deg180(var x)
{
	var y;
	
	if(x < 0) x = 0;
	if(x > 180) x = 180;
	if(x <= 22.5) y = pow(sinv(x*4),0.5);
	else
	{
		x = (x-22.5)*1.142857;
		y = cosv(x)*0.5+0.5;
	}
	
	return y;
}

var getSmoothYfromX2Deg180(var x)
{
	var y;
	
	if(x < 0) x = 0;
	if(x > 180) x = 180;
	if(x <= 45) y = pow(sinv(x*2),0.5);
	else
	{
		x = (x-45)*1.3333333;
		y = cosv(x)*0.5+0.5;
	}
	
	return y;
}

var getSmoothYfromX2Deg180b(var x)
{
	var y;
	
	if(x < 0) x = 0;
	if(x > 180) x = 180;
	if(x <= 90) y = pow(sinv(x),0.75);
	else
	{
		x = (x-90)*2;
		y = cosv(x)*0.5+0.5;
	}
	
	return y;
}

var getSmoothGrow180(var x, var fac)
{
	var y;
	
	if(x < 0) x = 0;
	if(x > 180) x = 180;
	if(x <= 45) y = pow(sinv(x*2),0.5)*fac;
	else
	{
		x = (x-45)*1.3333333;
		y = 1+(fac-1)*(cosv(x)*0.5+0.5);
	}
	
	return y;
}

var getFastSmoothGrow180(var x, var fac)
{
	var y;
	
	//x = clamp(x,0,180);
	if(x < 0) x = 0;
	if(x > 180) x = 180;
	if(x <= 22.5) y = pow(sinv(x*4),0.5)*fac;
	else
	{
		x = (x-22.5)*1.142857;
		y = 1+(fac-1)*(cosv(x)*0.5+0.5);
	}
	
	return y;
}

var getSmoothBounce180(var x, var waves)
{
	var y;
	
	y = sinv(x*waves)*(180-x)/180.0;
	
	return y;
}

var getBounceGrow180(var x, var waves, var fac)
{
	var y;
	
	if(x < 0) x = 0;
	if(x > 180) x = 180;
	if(x <= 60) y = pow(x/60.0, 2); //pow(sinv(x),0.75);
	else
	{
		x = (x-60)*1.5;
		y = 1+(fac-1)*sinv(x*waves)*(180-x)/180.0;
	}
	
	return y;
}


var varLerp(var a, var b, float t) 
{
	return (a + t*(b-a));
}

var varCosLerp(var a, var b, float t) 
{
	if(t < 0) t = 0;
	if(t > 1) t = 1;
	t = cosv(t*180)*0.5+0.5;
	return (b + t*(a-b));
}

var varSmoothLerp(var a, var b, float t) 
{
	if(t < 0) t = 0;
	if(t > 1) t = 1;
	t = t*t*(3-2*t); // default hlsl smoothstep
	return (a + t*(b-a));
}

var varSmootherLerp(var a, var b, float t) 
{
	if(t < 0) t = 0;
	if(t > 1) t = 1;
	t = (t*t*6 - t*15+10)*t*t*t;
	return (a + t*(b-a));
}

float floatSmootherLerp(float a, float b, float t) 
{
	if(t < 0) t = 0;
	if(t > 1) t = 1;
	t = (t*t*6 - t*15+10)*t*t*t;
	return (a + t*(b-a));
}

float fclamp(float x, float a, float b)
{
	if(x < a) return a;
	if(x > b) return b;
	return x;
}


float fmax(float a, float b)
{
	if(a > b) return a;
	return b;
}
float fmin(float a, float b)
{
	if(a < b) return a;
	return b;
}

STRING* strForTime(STRING* str, var time, int doFraction)
{
	static STRING* _lstr = NULL;
	if(!str)
	{
		if(!_lstr) _lstr = str_create("");
		str = _lstr;
	}
	int timeInt = time;
	int timeSecs = timeInt%60;
	int timeMinutes = (timeInt/60)%60;
	int timeHours = timeInt/3600;
	str_cpy(str,"");
	if(timeHours > 0) str_cat(str,str_printf(NULL,"%dh ",timeHours));
	if(timeMinutes > 0 || timeHours > 0) str_cat(str,str_printf(NULL,"%02dm ",timeMinutes));
	str_cat(str,str_printf(NULL,"%02ds ",timeSecs));
	if(doFraction && !timeHours) str_cat(str,str_printf(NULL,"%03dms ",(int)(fraction(time)*1000)));
	
	return str;	
}

/////////////////////////////////////////////////////////
// cached drawing I developed for pogostuck because the engine's text rendering is super slow

MATERIAL * mat_alpha_fill;

#define CACHED_TXT_MAX 4
#define CACHED_TXT_FPS 0
#define CACHED_TXT_TIMER 1
TEXT* cached_txt[CACHED_TXT_MAX];

TEXT* tmp_txt =
{
	string("");
}

typedef struct
{
	int type;
	void *obj;
	BMAP* bmp;
	int bmpIsExternal;
	int forceUpdate;
	int lastFrameUpdate;
	long flags;
	STRING *pstr;
	char *charsCopy;
	char *pchars;
	int length;
	COLOR color;
	FONT* font;
	var size_x,size_y;
} DRAWCONTAINER;
//LIST *drawContainerList = NULL;
DRAWCONTAINER* drawContainerArray[CACHED_TXT_MAX];

void drawContainerRefresh()
{
	/*LIST* current = drawContainerList;
	while(current)
	{
		DRAWCONTAINER* container = (DRAWCONTAINER*)current->data;
		container->forceUpdate = 1;
		current = current->next;
	}*/
	int i;
	for(i = 0; i < CACHED_TXT_MAX; i++)
	{
		DRAWCONTAINER* container = drawContainerArray[i];
		if(container) container->forceUpdate = 1;
	}
}

var draw_obj_cached_alpha = 100;
int draw_obj_cached(void *sourceObj, int containerSlot)
{
	TEXT* txt = (TEXT*)sourceObj;
	STRING* str = (txt->pstring)[0];
	DRAWCONTAINER* container = drawContainerArray[containerSlot];
	if(!container)
	{
		sys_marker("CCH");
		container = (DRAWCONTAINER*)sys_malloc(sizeof(DRAWCONTAINER));
		container->type = 0;
		container->obj = txt;
		container->bmp = NULL;
		container->bmpIsExternal = 0;
		container->forceUpdate = 1;
		//drawContainerList = listAdd(drawContainerList,container,0);
		drawContainerArray[containerSlot] = container;
		txt->skill_x = container;
		// everything else is set in the first update below
		sys_marker(NULL);
	}
	else
	{
		//if(container->pchars != str->chars) container->forceUpdate = 1;
		if(container->length != str->length) container->forceUpdate = 2;
		else
		{
			if(container->flags != txt->flags) container->forceUpdate = 3;
			else
			{
				if(container->font != txt->font || container->size_x != txt->size_x || container->size_y != txt->size_y) container->forceUpdate = 4;
				else
				{
					if(container->pstr != str) container->forceUpdate = 5;
					else
					{
						if(container->color.red-txt->red || container->color.green-txt->green || container->color.blue-txt->blue) container->forceUpdate = 5;
						else
						{
							int i;
							for(i = 0; i < container->length; i++)
							{
								if((container->charsCopy)[i] != (str->chars)[i])
								{
									container->forceUpdate = 6;
									break;
								}
							}
						}
					}
				}
			}
		}
	}
	
	if(container->forceUpdate)
	{
		//cprintf3("\ncontainer->forceUpdate(%d) for text %d at frame %d",container->forceUpdate,(int)txt->skill_y,(int)total_frames);
		container->forceUpdate = 0;
		int bmpSizeX = str_width(str,txt->font)+16;
		int bmpSizeY = txt->font.dy+2;
		if(!container->bmpIsExternal)
		{
			if(!container->bmp) container->bmp = bmap_createblack(bmpSizeX,bmpSizeY,32);
			else
			{
				if(container->bmp.width < bmpSizeX || container->bmp.height < bmpSizeY)
				{
					ptr_remove(container->bmp);
					container->bmp = bmap_createblack(bmpSizeX,bmpSizeY,32);
				}
			}
		}
		else
		{
			if(!container->bmp) return 1;
		}
		bmap_rendertarget(container->bmp,0,0);
		bmap_process(container->bmp,NULL,mat_alpha_fill);
		var pos_x = txt->pos_x;
		var pos_y = txt->pos_y;
		txt->pos_x = 0;
		txt->pos_y = 0;
		text_outline = 200;
		draw_obj(txt);
		txt->pos_x = pos_x;
		txt->pos_y = pos_y;
		bmap_rendertarget(NULL,0,0);
		container->lastFrameUpdate = total_frames;
		container->pstr = str;
		container->pchars = str->chars;
		container->flags = txt->flags;
		container->font = txt->font;
		container->size_x = txt->size_x;
		container->size_y = txt->size_y;
		vec_set(container->color,txt->blue);
		if(container->length != str->length)
		{
			if(container->charsCopy) sys_free(container->charsCopy);
			container->charsCopy = NULL;
		}
		if(str->chars && str->length)
		{
			sys_marker("CH2");
			if(!container->charsCopy) container->charsCopy = (char*)sys_malloc(sizeof(char)*str->length);
			memcpy(container->charsCopy,str->chars,sizeof(char)*str->length);
			sys_marker(NULL);
		}
		container->length = str->length;
	}
	draw_quad(container->bmp,vector(txt->pos_x,txt->pos_y,0),NULL,NULL,NULL,NULL, draw_obj_cached_alpha,0);
	draw_obj_cached_alpha = 100;
	
	return 0;	
}

void cached_startup()
{
	int i;
	for(i = 0; i < CACHED_TXT_MAX; i++)
	{
		TEXT* txt = txt_create(1,1);
		str_cpy((txt.pstring)[0],"");
		txt.font = cached_fnt;
		txt.flags = LIGHT | OUTLINE;
		txt.pos_x = 0;
		txt.pos_y = 0;
		txt.skill_x = 0;
		txt.skill_y = i;
		cached_txt[i] = txt;
	}
}

int drawCachedContainerGetSizeByID(int ID, var *size_x,  var* size_y)
{
	if(ID < 0 || ID >= CACHED_TXT_MAX) return 0;
	TEXT* txt = cached_txt[ID];
	DRAWCONTAINER* container = (DRAWCONTAINER*)txt->skill_x;
	if(!container) return 0;
	BMAP* bmp = container->bmp;
	if(!bmp) return 0;
	if(size_x) *size_x = bmp->width;
	if(size_y) *size_y = bmp->height;
	return bmp->width;
}

const long STRF_UNICODE = (1<<6);	// String contains 16-bit characters

void draw_text2cached(STRING* str, FONT* fnt, var pos_x, var pos_y, COLOR* color, int cachedID)
{
	if(cachedID < 0 || cachedID >= CACHED_TXT_MAX || !str) return;
	if(str->length <= 0) return;
	TEXT* txt = cached_txt[cachedID];
	STRING* targetStr = (txt.pstring)[0];
	targetStr.flags &= ~STRF_UNICODE;
	str_cpy((txt.pstring)[0],str);
	if(!fnt) txt->font = cached_fnt;
	else txt->font = fnt;
	txt->pos_x = pos_x;
	txt->pos_y = pos_y;
	vec_set(txt->blue,color);
	draw_obj_cached(txt, cachedID);
}

TEXT* draw_textCachedGetText(int cachedID)
{
	if(cachedID < 0 || cachedID >= CACHED_TXT_MAX) return NULL;
	return cached_txt[cachedID];
}	

void draw_text_via_txt(STRING* str, FONT* fnt, var pos_x, var pos_y, COLOR* color, int flags)
{
	vec_set(tmp_txt.blue,color);
	tmp_txt.pos_x = pos_x;
	tmp_txt.pos_y = pos_y;
	tmp_txt.font = fnt;
	tmp_txt.flags = (LIGHT | flags);
	str_cpy((tmp_txt.pstring)[0],str);
	draw_obj(tmp_txt);
}
