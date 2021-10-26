//////////////////////////////
// spearPostprocessing.c
// partially copied and simplified from Pogostuck - this is fine as render chains and post processing are usually already part of the engine
// note: the engine messes up internal rendertargets after changing resolutions twice (from lower to higher and back, or in my case from regular to 1:1 resolutions)
// as a result FXAA is bugged and breaks the display/ rendering after 2 resolution changes (oh, background blurring is getting stretched also apparently... need to create all render targets manually, great)
//////////////////////////////

var pp_d3d_lost_true = 0;
var pp_d3d_repaint_textures = 0;
var renderchain_to_texture = 0;
var background_view_active = 1;
//var backgroundRenderMode = 2; // moved over to types
var backgroundRenderModeOld = 2;
float backgroundBlurScaleH = 0.001;
float backgroundBlurScaleV = 0.001;

BMAP* renderchain_bmap = NULL;
BMAP* background_blurred_bmap = NULL;
BMAP* background_unblurred_bmap = NULL;

//////////////////////////////
// some materials used to be here

//////////////////////////////

VIEW* background_view = 
{
	layer = -5;
}

VIEW* background_blur_h =
{
	material = pp_blur_h_mat;
	flags = PROCESS_TARGET;
}

VIEW* background_blur_v =
{
	material = pp_blur_v_mat;
	flags = PROCESS_TARGET;
}

VIEW* view_pp_fxaa_luma =
{
	material = pp_fxaa_luma;
	flags = PROCESS_TARGET;
}

VIEW* view_pp_fxaa =
{
	material = pp_fxaa;
	flags = PROCESS_TARGET;
}

//////////////////////////////


void pp_remove_rendertargets()
{
	cprintf1("\n pp_remove_rendertargets() at frame %d", ITF);

	if(renderchain_bmap) ptr_remove(renderchain_bmap);
	renderchain_bmap = NULL;
	cprintf0(".");
	
	if(background_blurred_bmap) ptr_remove(background_blurred_bmap);
	background_blurred_bmap = NULL;
	cprintf0(".");
	
	if(background_unblurred_bmap) ptr_remove(background_unblurred_bmap);
	background_unblurred_bmap = NULL;
	cprintf0(".END");
}

void pp_create_rendertargets()
{
	cprintf1("\n pp_create_rendertargets() at frame %d", ITF);

	if(!renderchain_bmap) renderchain_bmap = bmap_createblack(screen_size.x, screen_size.y, 24);
	cprintf0(".");
	
	if(!background_blurred_bmap)
	{
		if(backgroundRenderMode >= 3) background_blurred_bmap = bmap_createblack(screen_size.x, screen_size.y, 24);
		else background_blurred_bmap = bmap_createblack(screen_size.x/2, screen_size.y/2, 24);
	}
	cprintf0(".");
	
	if(!background_unblurred_bmap)
	{
		if(backgroundRenderMode >= 3) background_unblurred_bmap = bmap_createblack(screen_size.x, screen_size.y, 24);
		else background_unblurred_bmap = bmap_createblack(screen_size.x/2, screen_size.y/2, 24);
	}
	cprintf0(".END");
}



void pp_setup_viewstages()
{
	cprintf3("\npp_setup_viewstages at frame %d | background_view_active(%d) | backgroundRenderMode(%d)", ITF, background_view_active, (int)backgroundRenderMode);
	
	camera.bmap = NULL; // neu, seit 20.01.2021
	camera.target1 = NULL;
	set(camera, NOCAST);
	int doSimpleBG = (backgroundRenderMode < 2);
	if(doSimpleBG)
	{
		reset(background_view, SHOW);
		camera.bmap = NULL;
		camera.stage = NULL;
		camera.stage = NULL; //view_pp_fxaa_luma; // NULL
		view_pp_fxaa_luma.stage = view_pp_fxaa;
		view_pp_fxaa.stage = NULL;
		if(backgroundRenderMode) reset(camera,NOCAST);
		return;
	}
    set(background_view, SHOW);
    background_view.stage = background_blur_h;
    background_view.bmap = background_unblurred_bmap;
    background_blur_h.stage = background_blur_v;
    background_blur_v.bmap = background_blurred_bmap;
    background_blur_v.stage = NULL;
    camera.stage = NULL; //view_pp_fxaa_luma; // NULL
    view_pp_fxaa_luma.stage = view_pp_fxaa;
    view_pp_fxaa.stage = NULL;

}

void pp_clear_viewstages()
{
	cprintf1("\n pp_clear_viewstages at frame %d",ITF);
	background_view.stage = NULL;
	background_view.bmap = NULL;
	background_blur_h.stage = NULL;
	background_blur_v.bmap = NULL;
	camera.stage = NULL;
	camera.bmap = NULL;
	view_pp_fxaa_luma.stage = NULL;
	view_pp_fxaa.stage = NULL;
	renderchain_to_texture = 0;
	cprintf0(".END");
}


void pp_on_d3d_lost_event()
{
	cprintf1("\npp_on_d3d_lost_event() at frame %d", (int)total_frames);
	pp_d3d_lost_true = 1;
	cprintf0(" - remove rendertargets now");
	pp_remove_rendertargets();
	cprintf0(" - pp_on_d3d_lost_event: remove rendertargets OK. pp_clear_viewstages now...");
	pp_clear_viewstages();
	cprintf0(" - pp_on_d3d_lost_event: OK");
}

void pp_on_d3d_reset_event()
{
	cprintf3("\npp_on_d3d_reset_event() at frame %d, screen_size(%d,%d)",(int)total_frames,(int)screen_size.x,(int)screen_size.y);
	pp_d3d_repaint_textures = 1;
	pp_d3d_lost_true = 0;
	pp_create_rendertargets();
	pp_setup_viewstages();
	drawContainerRefresh();
	wait(1);
	pp_d3d_repaint_textures = 0;
}

void pp_reset_from_menu()
{
	//pp_remove_rendertargets();
	pp_create_rendertargets();
	pp_setup_viewstages();
	drawContainerRefresh();
}

void pp_init()
{
	on_d3d_lost = pp_on_d3d_lost_event;
	on_d3d_reset = pp_on_d3d_reset_event;
	set(background_view, SHOW | UNTOUCHABLE | SHADOW | NOSHADOW | NOPARTICLE | ISOMETRIC | NOWORLD);
	//camera.bg = pixel_for_vec(COLOR_BLACK,0,8888); 
	set(camera, NOSHADOW | NOCAST | SHOW | ISOMETRIC | NOWORLD); //UNTOUCHABLE
	pp_create_rendertargets();
	pp_setup_viewstages();
}

void pp_view_copy(VIEW* view_target, VIEW* view_source)
{
	view_target.clip_near = view_source.clip_near;
	view_target.clip_far = view_source.clip_far;
	view_target.arc = view_source.arc;
	view_target.aspect = view_source.aspect;
	view_target.left = view_source.left;
	view_target.right = view_source.right;
	view_target.bottom = view_source.bottom;
	view_target.top = view_source.top;
	vec_set(view_target.x,view_source.x);
	vec_set(view_target.pan,view_source.pan);
}

void pp_update()
{
	if(pp_d3d_lost_true) return;
	static int background_view_active_old = 1;
	background_view_active = (backgroundRenderMode >= 2);
	int requestViewStagesRedo = 0;
	static var screenSizesSetOnce = 0;
	static var screenSizeXOld = 0;
	static var screenSizeYOld = 0;
	if(!screenSizesSetOnce)
	{
		screenSizesSetOnce = 1;
		screenSizeXOld = screen_size.x;
		screenSizeYOld = screen_size.y;
	}
	if(backgroundRenderModeOld != backgroundRenderMode || screenSizeXOld != screen_size.x || screenSizeYOld != screen_size.y)
	{
		backgroundRenderModeOld = backgroundRenderMode;
		screenSizeXOld = screen_size.x;
		screenSizeYOld = screen_size.y;
		/*if(backgroundRenderModeOld >= 2)
		{
            if(background_blurred_bmap) ptr_remove(background_blurred_bmap);
            if(background_unblurred_bmap) ptr_remove(background_unblurred_bmap);
            if(backgroundRenderMode >= 3) //backgroundBlurredFullResolution)
            {
                background_blurred_bmap = bmap_createblack(screen_size.x, screen_size.y, 24);
                background_unblurred_bmap = bmap_createblack(screen_size.x, screen_size.y, 24);
            }
            else
            {
                background_blurred_bmap = bmap_createblack(screen_size.x/2, screen_size.y/2, 24);
                background_unblurred_bmap = bmap_createblack(screen_size.x/2, screen_size.y/2, 24);
            }
			background_view.bmap = background_unblurred_bmap;
			background_blur_v.bmap = background_blurred_bmap;
		}*/
		pp_remove_rendertargets();
		pp_create_rendertargets();
		//else background_view_active = 0;
		requestViewStagesRedo = 1;
	}

	camera.bmap = NULL; // 20.01.2021
	
	if(background_view_active_old != background_view_active || requestViewStagesRedo)
	{
		background_view_active_old = background_view_active;
		pp_setup_viewstages();
	}

	if(background_view_active)
	{
		pp_view_copy(background_view,camera);
		if(backgroundRenderMode >= 3)
		{
			background_view.size_x = screen_size.x;
			background_view.size_y = screen_size.y;
			if(screen_size.x && screen_size.y)
			{
				//backgroundBlurScaleF = 1.0/(float)screen_size.x;
				backgroundBlurScaleH = 2.0/(float)screen_size.x;
				backgroundBlurScaleV = 2.0/(float)screen_size.y;
			}
		}
		else
		{
			background_view.size_x = screen_size.x/2;
			background_view.size_y = screen_size.y/2;
			if(screen_size.x && screen_size.y)
			{
				backgroundBlurScaleH = 2.0/(float)screen_size.x;
				backgroundBlurScaleV = 2.0/(float)screen_size.y;
			}
		}
	}
    if(0)
    {
	    //if(!backgroundRenderMode) vec_set(sky_color, vector(backgroundColor_b,backgroundColor_g,backgroundColor_r));
	    //else vec_fill(sky_color, 0);
    }
	
	#ifdef DEVTRUE
		//if(key_f3) DEBUG_BMAP(depth_view_bmap,100,0.5);
		//if(key_1) DEBUG_BMAP(depth_view.bmap,100,0.5);
		//if(key_2) DEBUG_BMAP(shadowmap,100,0.5);
		if(key_2) DEBUG_BMAP(background_unblurred_bmap,100,0.5);
		if(key_3) DEBUG_BMAP(background_blurred_bmap,100,0.5);
		if(key_4) DEBUG_BMAP(renderchain_bmap,100,0.5);
		if(key_q) // && 1)
		{
			//draw_quad(NULL,vector(20,320,0),NULL,vector(1200,500,0),NULL,COLOR_BLACK,50,0);
			draw_text2(str_printf(NULL,"renderchain_to_texture: %d",(int)renderchain_to_texture),20,340,COLOR_WHITE);
			draw_text2(str_printf(NULL,"camera(%p).bmap: %p stage(%p)", camera, camera.bmap, camera.stage),20,360,COLOR_WHITE);
			draw_text2(str_printf(NULL,"view_pp_fxaa: %p bmap(%p) stage(%p)", view_pp_fxaa, view_pp_fxaa.bmap, view_pp_fxaa.stage),20,420,COLOR_WHITE);
			draw_text2(str_printf(NULL,"view_pp_fxaa_luma: %p bmap(%p) stage(%p)", view_pp_fxaa_luma, view_pp_fxaa_luma.bmap, view_pp_fxaa_luma.stage),20,440,COLOR_WHITE);
			draw_text2(str_printf(NULL,"is(background_view,SHOW): %d",(int)is(background_view,SHOW)),20,560,COLOR_WHITE);
			draw_text2(str_printf(NULL,"renderchain_bmap: %p",renderchain_bmap),20,600,COLOR_WHITE);
			draw_text2(str_printf(NULL,"background_view_active: %d",(int)background_view_active),20,620,COLOR_WHITE);
			draw_text2(str_printf(NULL,"screen_size: (%d, %d)",(int)screen_size.x,(int)screen_size.y),20,640,COLOR_WHITE);
			if(background_unblurred_bmap)  draw_text2(str_printf(NULL,"background_unblurred_bmap: (%d, %d)",(int)background_unblurred_bmap->width,(int)background_unblurred_bmap->height),20,660,COLOR_WHITE);
			if(background_blurred_bmap)  draw_text2(str_printf(NULL,"background_blurred_bmap: (%d, %d)",(int)background_blurred_bmap->width,(int)background_blurred_bmap->height),20,680,COLOR_WHITE);
		}
	#endif
}
