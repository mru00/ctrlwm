/* 
* move/resize resize/move
 * autotile
 * auto trans
 * diff layout per desktop
 * untile
 * delete actual desktop
 * layout from file*/

#include <assert.h>   
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>
#include <X11/Xmu/WinUtil.h>
#include <glib.h>
#include <X11/extensions/XTest.h>
#include <X11/Xmu/Error.h>
#define MAX_PROPERTY_VALUE_LEN 4096
#define HELP "ctrlwm \n" \
"App for automatic positioning and resizing of 1-9 windows\n" \
"\n" \
"Usage: ctrlwm [Action]...\n" \
"  -rc            Read .rc file\n" \
"  -gui           Start gui\n" \
"Actions:\n" \
"  -t  [Options]  Tile windows with active as main/biggest window	 \n" \
"  -v  [Options]  Tile windows in vertical stripes	 \n" \
"  -h  [Options]  Tile windows in horizontal stripes	 \n" \
"  -g  [Options]  Choose windows for tiling (Doubleclick last window to execute	) \n" \
"  -m  [Options]  Maximize active		\n" \
"  -n             Next window (like alt+esc, without bugs/features)\n" \
"  -s  [-o]       Place active depending on mouse window border	\n"\
"                 - similar to aero snap \n" \
"  -snap [-o]     Some sort of experimental snap \n" \
"  -z  [-o]       Place active with placement/size depending on mouse position \n"\
"      [-b]       Add moving windows to another workspace (bottom of screen)\n" \
"      [-u]       Show sectors on screen	\n" \
"  -e             Go on empty workspace.If none available, create one		\n" \
"  -d             Delete last workspace		\n" \
"  -i             Print some info	\n" \
"  -k             Daemon Mode screen corner/border actions\n" \
"  -r             Daemon Mode Auto-raise active window\n" \
"Options:\n" \
"  -p             Tile windows in order opened	\n" \
"  -a             Use alternative layout for tiling \n" \
"  -w             Tile minimized too	\n" \
"  -o             Offset top/bottom/left/right   (-o XX XX XX XX)\n"\
"  -f             Auto offset 1-3  (-f X)\n" \
"  -c             Exclude selected window from tiling	\n"\
"  -mn            Window decoration offset  (-mn XX XX)\n" \
"\n" \
"\n" \
"Hints:\n" \
"If Windows overlap with tiling, you have to try out different values for -mn\n" \
"Default is -mn 12 29\n" \
"Use the software with key shortcuts, mouse strokes or panel starters\n" \
"\n" \
"\n" \
"Author, current maintainer: zlatko kartelo\n" \
"Some code taken from wmctrl \n" \
"Released under the GNU General Public License.\n" \
"Copyright (C) 2009/2010\n"
Window wcount;
char *version="0.6.0.5";
int i;
float win_pm [9][4];
char cornerNW [512];
char cornerSW [512];
char cornerNE [512];
char cornerSE [512];
char borderN [512];
char borderS [512];
char borderW [512];
char borderE [512];
/*window related */
int winondesk=1;
int winl [20]={0}; /*desk window list*/
int activewin;
int tmp_win_1, tmp_win_2;
int winsnc[20][6]={{0}}; /*window old dimensions*/
/*desk related */
int nmbrdesks;
int activedesk;
int dskrw, dskrh;
int dskww, dskwh;
float dskcw, dskch;
int deskZx, deskZy;
/*options*/
int dcw=12; /*decoration width*/
int dch=29; /*decoration height*/
int alternative_layout=0;
int offset_auto=0;
int tilewindowsoorder=0;
int screen_sec=0;
int tile_min_win=0;
int tile_stripes=0;
int tile_vert=0;
int win_max=0;
int win_move_desk=0;
int win_exclude=0;
unsigned int offset_top=0,offset_bottom=0,offset_left=0,offset_right=0;
Window secwin;
unsigned char get_xchar(void);
int WIN_HEIGHT, WIN_WIDTH;
int BUTHEIGHT=40;
int BUTWIDTH,BUTTON1_X,BUTTON1_Y,BUTTON2_X,BUTTON2_Y,BUTTON3_X,BUTTON3_Y,
    BUTTON4_X,BUTTON4_Y;
unsigned char stillused; 
unsigned char toberefreshed;
unsigned char cpslock;
int mousex;
int mousey;
unsigned char clicked;
unsigned char rightclicked;
unsigned char typedchar;
Display *display;
int screen_num;
Window win;
XFontStruct *font_info;
GC black_gc, color_gc, text_gc;
XGCValues gc_values;
unsigned long gc_valuemask = 0;
XColor color_info;
unsigned int color_table[256];
unsigned short color_data[256][3];
char *display_name = NULL;
char *window_name = "ctrlwm";
XTextProperty wname;
char *font_name = "10x20";
XEvent event;
KeySym keysym;

int  Graphics_init ( unsigned int, unsigned int );
void Graphics_shutdown ();
void refresh();
void Menu_gui ();
void trans ();
void Prepare_tile ();
void Tile ();
void Place_window ();	
void Place_window_sugg ();
void Place_sm_window_sugg ();
void Print_screen_sections ();
void Print_screen_menu ();
void Tile_menu();
void Exchange_active ();	
void Exchange_select ();
void Stripe_wins();
void Get_info_desk ();
void Get_info_active ();
void Get_info_win_desk ();
void Sort_list ();
void Sort_list_xchg ();
void Max_remove ();
void Min_remove ();
void Win_raise ();
void Print_info ();
void Print_info_t ();
void Empty_desk ();
void Del_desk ();
static int Client_msg(Display *disp, Window win, char *msg, 
           unsigned long data0, unsigned long data1,unsigned long data2,
		   unsigned long data3, unsigned long data4);
static Window Get_active_window(Display *disp);
static gchar *Get_property (Display *disp, Window win, 
              Atom xa_prop_type, gchar *prop_name, unsigned long *size);
static Window *Get_client_list (Display *disp, unsigned long *size);
static Window Select_Window(Display *disp);
void Mouse_daemon ();
void Next_win ();
void Auto_raise_active ();
void Auto_op_win ();
void Tile_choose_win();
void Count_win (char *label);
void Refresh_count (char *label);
void Read_rc ();
void Get_win_layout ();
void ReadDim ();
void LinSnap ();
void Unsnap ();

int main (int argc, char *argv[])
{XSetErrorHandler(XmuSimpleErrorHandler);
Get_info_desk ();
Get_info_win_desk ();
Get_info_active ();
/*Read_rc ();*/ 
for (i = 1; i < argc; i++)  
  {
	if (strcmp(argv[i], "--help") == 0) {fputs(HELP, stdout);return 1;}
	if (strcmp(argv[i], "-c") == 0) {win_exclude=1;} 	
	if (strcmp(argv[i], "-p") == 0) {tilewindowsoorder=1;}
    if (strcmp(argv[i], "-w") == 0) {tile_min_win=1;}
	if (strcmp(argv[i], "-a") == 0) {alternative_layout=1;}
	if (strcmp(argv[i], "-u") == 0) {screen_sec=1;}
	if (strcmp(argv[i], "-b") == 0) {win_move_desk=1;}
	if (strcmp(argv[i], "-o") == 0) 
       {
       if (i + 4 <= argc - 1) 
          {
          i++;offset_top=atoi(argv[i]);i++;offset_bottom=atoi(argv[i]); 
		  i++;offset_left=atoi(argv[i]);i++;offset_right=atoi(argv[i]); 
          }
        }
	if (strcmp(argv[i], "-f") == 0) 
       {if (i + 1 <= argc - 1){i++;offset_auto = atoi(argv[i]);}}
    if (strcmp(argv[i], "-mn") == 0) 
       {if (i+2<=argc-1){i++;dcw=atoi(argv[i]);i++; dch = atoi(argv[i]);}}
  }
for (i = 1; i < argc; i++)  
  {
	if (strcmp(argv[i], "-rc") == 0) {Read_rc ();}
  }
for (i = 1; i < argc; i++)  
  {
	if (strcmp(argv[i], "-t") == 0) {Prepare_tile ();return 0;}
	if (strcmp(argv[i], "-q") == 0) {Tile_menu ();return 0;}
	if (strcmp(argv[i], "-m") == 0) {win_max=1;Prepare_tile ();return 0;}
	if (strcmp(argv[i], "-n") == 0) {Next_win ();return 0;} 
	if (strcmp(argv[i], "-h") == 0) {tile_stripes=1;Prepare_tile ();return 0;} 	
	if (strcmp(argv[i], "-v") == 0) {tile_vert=1;tile_stripes=1;Prepare_tile ();return 0;} 	
	if (strcmp(argv[i], "-s") == 0) {Place_window ();return 0;} 	
	if (strcmp(argv[i], "-z") == 0) {Place_window_sugg ();return 0;} 	
	if (strcmp(argv[i], "-e") == 0) {Empty_desk();return 0;}
	if (strcmp(argv[i], "-d") == 0) {Del_desk();return 0;}
    if (strcmp(argv[i], "-x") == 0) {Exchange_active ();return 0;}
	if (strcmp(argv[i], "-y") == 0) {Exchange_select ();return 0;}
	if (strcmp(argv[i], "-i") == 0) {Print_info_t ();return 0;}
	if (strcmp(argv[i], "-k") == 0) {Mouse_daemon ();return 0;}
	if (strcmp(argv[i], "-r") == 0) {Auto_raise_active ();return 0;}
	if (strcmp(argv[i], "-g") == 0) {Tile_choose_win ();return 0;}
	if (strcmp(argv[i], "-gui") == 0) {Menu_gui ();return 0;}
    if (strcmp(argv[i], "-snap") == 0) {LinSnap ();return 0;}
  }
fputs(HELP, stdout);
return 1;
}

void Read_rc ()
{
FILE *fp=fopen(".ctrlwm.rc","r");
char var [512],value[512],line[512];
  if (fp) 
     {
	  while (fgets(line,sizeof(line),fp))
             {
			  memset (var,0,sizeof(var));
              memset (value,0,sizeof(value));
              sscanf(line,"%[^'=']=%[^\' '\n]%*[\t]%*[\t]",var,value);
		      if (strcmp (var, "dcw")==0){dcw=atoi (value);}
			  if (strcmp (var, "dch")==0){dch=atoi (value);}
			  if (strcmp (var, "alternative_layout")==0){alternative_layout=atoi (value);}
			  if (strcmp (var, "tilewindowsoorder")==0){tilewindowsoorder=atoi (value);}
			  if (strcmp (var, "tile_min_win")==0){tile_min_win=atoi (value);}
			  if (strcmp (var, "tile_vert")==0){tile_vert=atoi (value);}
			  if (strcmp (var, "tile_stripes")==0){tile_stripes=atoi (value);}
			  if (strcmp (var, "offset_top")==0){offset_top=atoi (value);}
			  if (strcmp (var, "offset_bottom")==0){offset_bottom=atoi (value);}
			  if (strcmp (var, "offset_left")==0){offset_left=atoi (value);}
			  if (strcmp (var, "offset_right")==0){offset_right=atoi (value);}
			  if (strcmp (var, "screen_sec")==0){screen_sec=atoi (value);}
			  if (strcmp (var, "win_move_desk")==0){win_move_desk=atoi (value);}
			  if (strcmp (var, "gui_button_height")==0){BUTHEIGHT=atoi (value);}
			  sscanf(line,"%[^'=']=%[^\'#'\n]%*[\t]%*[\t]",var,value);  
			  if (strcmp (var, "cornerNW")==0){for( i=0;i<100;i++){cornerNW[i]=value[i];}}
			  if (strcmp (var, "cornerSW")==0){for( i=0;i<512;i++){cornerSW[i]=value[i];}}
			  if (strcmp (var, "cornerNE")==0){for( i=0;i<512;i++){cornerNE[i]=value[i];}}
			  if (strcmp (var, "cornerSE")==0){for( i=0;i<512;i++){cornerSE[i]=value[i];}}
			  if (strcmp (var, "borderN")==0){for( i=0;i<512;i++){borderN[i]=value[i];}}
			  if (strcmp (var, "borderS")==0){for( i=0;i<512;i++){borderS[i]=value[i];}}
			  if (strcmp (var, "borderW")==0){for( i=0;i<512;i++){borderW[i]=value[i];}}
			  if (strcmp (var, "borderE")==0){for( i=0;i<512;i++){borderE[i]=value[i];}}
			  }
	  fclose (fp);
	  }
return ;
}

void Tile ()
{
int ow=deskZx+0;
int oh=deskZy+0;
Display *disp;
disp = XOpenDisplay(NULL);
/*Modify values if option*/
dskch=dskch-offset_bottom;oh=deskZy+offset_top;dskch=dskch-offset_top;
dskcw=dskcw-offset_right;ow=deskZx+offset_left;dskcw=dskcw-offset_left;	
  switch (offset_auto)
	{
	case 0:
	break;
	case 1: 
	ow=deskZx+dskcw/4;oh=deskZy+dskch/4;dskcw=dskcw/4*3;dskch=dskch/4*3;break;
	case 2:
	ow=deskZx+dskcw/3;oh=deskZy+dskch/3;dskcw=dskcw/3*2;dskch=dskch/3*2;break;
	case 3:
	ow=deskZx+dskcw/2;oh=deskZy+dskch/2;dskcw=dskcw/2;dskch=dskch/2;break;
	}
/*Place & resize the windows*/
Get_win_layout ();
  for (i =1; i < winondesk+1; i++) 
   {XMoveResizeWindow(disp,winl[i],(dskcw*win_pm [i-1][0])+ow,(dskch*win_pm [i-1][1])+oh,
	                               (win_pm [i-1][2]*dskcw)-dcw,(win_pm [i-1][3]*dskch)-dch);
   }
XCloseDisplay(disp);return;
}

void Mouse_daemon ()
{
Display *dpy;
Window root;
Window ret_root;
Window ret_child;
int root_x=10;
int root_y=10;
int win_x;
int win_y;
unsigned int mask;
dpy = XOpenDisplay(NULL);
root = XDefaultRootWindow(dpy);

  while ((root_x>6) & (root_y>6)& (root_x<(dskcw-6)) & (root_y<(dskch-6)) )
  {
	if(XQueryPointer(dpy, root, &ret_root, &ret_child, &root_x, &root_y,
					 &win_x, &win_y, &mask))
	  {usleep (20000);}
  }
 /*corners*/
 if ((root_x<(6)) & (root_y<(6))) 
    {system (cornerNW);}	
 if ((root_x>(dskrw-6)) & (root_y<(6))) 
    {system (cornerNE);}
 if ((root_x<(6)) & (root_y>(dskrh-6))) 
    {system (cornerSW);}
 if ((root_x>(dskrw-6)) & (root_y>(dskrh-6))) 
    {system (cornerSE);}
 /*borders*/
 if ((root_x>((dskrw/8))) & (root_x<(dskrw-(dskrw/8))) & (root_y<(6))) 
    {system (borderN);}
 if ((root_x>((dskrw/8))) & (root_x<(dskrw-(dskrw/8))) & (root_y>(dskrh-6))) 
    {system (borderS);}
 if ((root_x<(6)) & (root_y>((dskrh/8))) & (root_y<(dskrh-(dskrh/8)))) 
    {system (borderW);}
 if ((root_x>(dskrw-6)) & (root_y>((dskrh/8))) & (root_y<(dskrh-(dskrh/8)))) 
    {system (borderE);}
XCloseDisplay(dpy);
Mouse_daemon ();
}

void Prepare_tile ()
{
 Get_info_desk ();
Get_info_win_desk ();
Get_info_active ();
  if (!tilewindowsoorder){Sort_list();};
  if (win_exclude)
	{
	Display *disp;
	disp = XOpenDisplay(NULL);
	tmp_win_1=Select_Window(disp);
	tmp_win_2=winl[winondesk];
	XCloseDisplay(disp);
	Sort_list_xchg();
	};
  if (win_max){winondesk=1;};
  Max_remove ();
  if (tile_min_win){Win_raise ();Min_remove ();};
  if (win_exclude){winondesk=winondesk-1;Win_raise ();Min_remove ();};
  if (tile_stripes){Stripe_wins();return;};
Tile ();    
return;
}

void Place_window ()
{
int ow=deskZx+0;
int oh=deskZy+0;
Sort_list();
activewin=winl[1];
if (winondesk==0) {return;} 
Display *disp;
disp = XOpenDisplay(NULL);
Client_msg(disp, activewin, "_NET_ACTIVE_WINDOW", 
            0, 0, 0, 0, 0);
Client_msg(disp, activewin, "_NET_WM_STATE", 
           XInternAtom(disp,"_NET_WM_STATE_REMOVE",False) , 
		   XInternAtom(disp,"_NET_WM_STATE_MAXIMIZED_VERT",False),
		   XInternAtom(disp,"_NET_WM_STATE_MAXIMIZED_HORZ",False),0,0);
XCloseDisplay(disp);
	Display *dpy;
	Window root;
	Window ret_root;
	Window ret_child;
	int root_x=10;
	int root_y=10;
	int win_x;
	int win_y;
	unsigned int mask;
 	dpy = XOpenDisplay(NULL);
	root = XDefaultRootWindow(dpy);
/*pointer feedback*/
int status;
Cursor cursor;
cursor = XCreateFontCursor(dpy, XC_crosshair);
status = XGrabPointer(dpy, root, False,ButtonPressMask|ButtonReleaseMask,
                      GrabModeSync,GrabModeAsync, root, cursor, CurrentTime);
  if (status != GrabSuccess) 
	{
     fputs("ERROR: Cannot grab mouse.\n", stderr);
     return ;
    }
XAllowEvents(dpy, SyncPointer, CurrentTime);
/*pointer feedback end -set this when no longer needed- XUngrabPointer(disp, CurrentTime);*/
while ((root_x>3) & (root_y>3)& (root_x<(dskrw-3)) & (root_y<(dskrh-3)) )
  {
  if(XQueryPointer(dpy, root, &ret_root, &ret_child, &root_x, &root_y,
				   &win_x, &win_y, &mask))
	{
	if (mask & (Button1Mask)){return;}
	Display *disp;
	disp = XOpenDisplay(NULL);
	XMoveResizeWindow(disp,activewin,root_x,root_y,300,200);
	XCloseDisplay(disp);
	}
  }
dskch=dskch-offset_bottom;oh=deskZy+offset_top;dskch=dskch-offset_top;
dskcw=dskcw-offset_right;ow=deskZx+offset_left;dskcw=dskcw-offset_left;
 if ((root_x>(dskrw-3)))
    {disp = XOpenDisplay(NULL);XMoveResizeWindow(disp,activewin,(dskcw/2)+ow,0+oh,
	                                            (dskcw/2)-dcw,dskch-dch);}
 if ((root_x<3))
    {disp = XOpenDisplay(NULL);XMoveResizeWindow(disp,activewin,0+ow,0+oh,
	                                            (dskcw/2)-dcw,dskch-dch);}
 if ((root_y<3))
    {disp = XOpenDisplay(NULL);XMoveResizeWindow(disp,activewin,0+ow,0+oh,
	                                            (dskcw)-dcw,(dskch/2)-dch);}
 if ((root_y>(dskrh-3)))
    {disp = XOpenDisplay(NULL);XMoveResizeWindow(disp,activewin,0+ow,(dskch/2)+oh,
	                                            (dskcw)-dcw,(dskch/2)-dch);}
XCloseDisplay(disp); XUngrabPointer(dpy, CurrentTime);return ;
}

void Place_window_sugg ()
{
int ow=deskZx+0;
int oh=deskZy+0;
if (winondesk==0) {return;}
Sort_list();
activewin=winl[1];
Display *disp;
disp = XOpenDisplay(NULL);
Client_msg(disp, activewin, "_NET_ACTIVE_WINDOW",0,0,0,0,0);
Client_msg(disp, activewin, "_NET_WM_STATE", 
           XInternAtom(disp,"_NET_WM_STATE_REMOVE",False) , 
		   XInternAtom(disp,"_NET_WM_STATE_MAXIMIZED_VERT",False),
		   XInternAtom(disp,"_NET_WM_STATE_MAXIMIZED_HORZ",False),0,0);
XCloseDisplay(disp);
	Display *dpy;
	Window root;
	Window ret_root;
	Window ret_child;
	int root_x=10;
	int root_y=10;
	int win_x;
	int win_y;
	unsigned int mask;
 	dpy = XOpenDisplay(NULL);
	root = XDefaultRootWindow(dpy);
 dskwh=dskch;
 dskww=dskcw;
dskch=dskch-offset_bottom;oh=deskZy+offset_top;dskch=dskch-offset_top;
dskcw=dskcw-offset_right;ow=deskZx+offset_left;dskcw=dskcw-offset_left;
/*pointer feedback*/
int status;
Cursor cursor;
cursor = XCreateFontCursor(disp, XC_crosshair);
status = XGrabPointer(disp, root, False,ButtonPressMask|ButtonReleaseMask,
                      GrabModeSync,GrabModeAsync, root, cursor, CurrentTime);
  if (status != GrabSuccess) 
	{
     fputs("ERROR: Cannot grab mouse.\n", stderr);
     return ;
    }
XAllowEvents(disp, SyncPointer, CurrentTime);
/*pointer feedback end -set this when no longer needed- XUngrabPointer(disp, CurrentTime);*/

  if (screen_sec) {Print_screen_sections ();}
while (1)
  {
  
  if(XQueryPointer(dpy, root, &ret_root, &ret_child, &root_x, &root_y,
				   &win_x, &win_y, &mask))
	{
	if (mask & (Button1Mask)){if (screen_sec) {Print_screen_sections ();};return;}
	Display *disp;
	disp = XOpenDisplay(NULL);
	/*CenterVerticalB*/
	if ((root_x>((dskww/16)*7)+ow) & (root_x<((dskww/16)*8)+ow)& (root_y>((dskwh/16)*7)+oh) & (root_y<((dskwh/16)*9)+oh))
	{XMoveResizeWindow(disp,activewin,0+ow,0+oh,(((dskcw/4)*3)-dcw),dskch-dch);}
	if ((root_x>((dskww/16)*8)+ow) & (root_x<((dskww/16)*9)+ow)& (root_y>((dskwh/16)*7)+oh) & (root_y<((dskwh/16)*9)+oh))
	{XMoveResizeWindow(disp,activewin,(dskcw/4)+ow,0+oh,(((dskcw/4)*3)-dcw),dskch-dch);}
	/*CenterHorizontalB*/
	if ((root_x>((dskww/16)*7)+ow) & (root_x<((dskww/16)*9)+ow)& (root_y>((dskwh/16)*6)+oh) & (root_y<((dskwh/16)*7)+oh))
	{XMoveResizeWindow(disp,activewin,0+ow,0+oh,(dskcw-dcw),((dskch/4)*3)-dch);}
	if ((root_x>((dskww/16)*7)+ow) & (root_x<((dskww/16)*9)+ow)& (root_y>((dskwh/16)*9)+oh) & (root_y<((dskwh/16)*10)+oh))
	{XMoveResizeWindow(disp,activewin,0+ow,(dskch/4)+oh,(dskcw-dcw),((dskch/4)*3)-dch);}
	/*CenterVertical*/
	if ((root_x>((dskww/16)*9)+ow) & (root_x<((dskww/8)*5)+ow)& (root_y>((dskwh/8)*3)+oh) & (root_y<((dskwh/8)*5)+oh))
	{XMoveResizeWindow(disp,activewin,((dskcw/2))+ow,0+oh,((dskcw/2)-dcw),dskch);}
	if ((root_x>((dskww/8)*3)+ow) & (root_x<((dskww/16)*7)+ow)& (root_y>((dskwh/8)*3)+oh) & (root_y<((dskwh/8)*5)+oh))
	{XMoveResizeWindow(disp,activewin,0+ow,0+oh,((dskcw/2)-dcw),dskch);}
	/*CenterHorizontal*/
	if ((root_x>((dskww/8)*3)+ow) & (root_x<((dskww/8)*5)+ow)& (root_y>((dskwh/8)*2)+oh) & (root_y<((dskwh/8)*3)+oh))
	{XMoveResizeWindow(disp,activewin,0+ow,0+oh,(dskcw-dcw),(dskch/2)-dch);}
	if ((root_x>((dskww/8)*3)+ow) & (root_x<((dskww/8)*5)+ow)& (root_y>((dskwh/8)*5)+oh) & (root_y<((dskwh/8)*6)+oh))
	{XMoveResizeWindow(disp,activewin,0+ow,(dskch/2)+oh,(dskcw-dcw),(dskch/2)-dch);}
	/*EastWestVertical*/
	if ((root_x>((dskww/8)*2)+ow) & (root_x<((dskww/8)*3)+ow)& (root_y>((dskwh/8)*2)+oh) & (root_y<((dskwh/8)*4)+oh))
	{XMoveResizeWindow(disp,activewin,0+ow,0+oh,((dskcw/2)-dcw),(dskch/2)-dch);}
	if ((root_x>((dskww/8)*2)+ow) & (root_x<((dskww/8)*3)+ow)& (root_y>((dskwh/8)*4)+oh) & (root_y<((dskwh/8)*6)+oh))
	{XMoveResizeWindow(disp,activewin,0+ow,(dskch/2)+oh,((dskcw/2)-dcw),(dskch/2)-dch);}
	if ((root_x>((dskww/8)*5)+ow) & (root_x<((dskww/8)*6)+ow)& (root_y>((dskwh/8)*2)+oh) & (root_y<((dskwh/8)*4)))
	{XMoveResizeWindow(disp,activewin,((dskcw/2)-dcw)+ow,0+oh,((dskcw/2)-dcw),(dskch/2)-dch);}
	if ((root_x>((dskww/8)*5)+ow) & (root_x<((dskww/8)*6)+ow)& (root_y>((dskwh/8)*4)+oh) & (root_y<((dskwh/8)*6)))
	{XMoveResizeWindow(disp,activewin,((dskcw/2)-dcw)+ow,(dskch/2)+oh,((dskcw/2)-dcw),(dskch/2)-dch);}
	/*MiddleCross*/
	if ((root_x>((dskww/8)*3)+ow) & (root_x<((dskww/8)*5)+ow)& (root_y>((dskwh/8)*1)+oh) & (root_y<((dskwh/8)*2)+oh))
	{XMoveResizeWindow(disp,activewin,(((dskcw/8)*1)-dcw)+ow,0+oh,((dskcw/8)*6)-dcw,((dskch/8)*2)-dch);}
	if ((root_x>((dskww/8)*3)+ow) & (root_x<((dskww/8)*5)+ow)& (root_y>((dskwh/8)*6)+oh) & (root_y<((dskwh/8)*7)+oh))
	{XMoveResizeWindow(disp,activewin,(((dskcw/8)*1)-dcw)+ow,((dskch/8)*6)+oh,((dskcw/8)*6)-dcw,((dskch/8)*2)-dch);}
	if ((root_x>((dskww/8)*1)+ow) & (root_x<((dskww/8)*2)+ow)& (root_y>((dskwh/8)*3)+oh) & (root_y<((dskwh/8)*5)+oh))
	{XMoveResizeWindow(disp,activewin,0+ow,((dskch/8)*1)+oh,(((dskcw/8)*2)-dcw),((dskch/8)*6)-dch);}
	if ((root_x>((dskww/8)*6)+ow) & (root_x<((dskww/8)*7)+ow)& (root_y>((dskwh/8)*3)+oh) & (root_y<((dskwh/8)*5)+oh))
	{XMoveResizeWindow(disp,activewin,((dskcw/8)*6)+ow,((dskch/8)*1)+oh,(((dskcw/8)*2)-dcw),((dskch/8)*6)-dch);}
	/*CornersMiddleScreen*/
	if ((root_x>((dskww/8)*1)+ow) & (root_x<((dskww/8)*3)+ow)& (root_y>((dskwh/8)*1)+oh) & (root_y<((dskwh/8)*2)+oh))
	{XMoveResizeWindow(disp,activewin,0+ow,0+oh,(((dskcw/8)*4)-dcw),((dskch/8)*2)-dch);}
	if ((root_x>((dskww/8)*5)+ow) & (root_x<((dskww/8)*7)+ow)& (root_y>((dskwh/8)*1)+oh) & (root_y<((dskwh/8)*2)+oh))
	{XMoveResizeWindow(disp,activewin,(((dskcw/8)*4))+ow,0+oh,(((dskcw/8)*4)-dcw),((dskch/8)*2)-dch);}
	if ((root_x>((dskww/8)*1)+ow) & (root_x<((dskww/8)*3)+ow)& (root_y>((dskwh/8)*6)+oh) & (root_y<((dskwh/8)*7)+oh))
	{XMoveResizeWindow(disp,activewin,0+ow,((dskch/8)*6)+oh,(((dskcw/8)*4)-dcw),((dskch/8)*2)-dch);}
	if ((root_x>((dskww/8)*5)+ow) & (root_x<((dskww/8)*7)+ow)& (root_y>((dskwh/8)*6)+oh) & (root_y<((dskwh/8)*7)+oh))
	{XMoveResizeWindow(disp,activewin,(((dskcw/8)*4))+ow,((dskch/8)*6)+oh,(((dskcw/8)*4)-dcw),((dskch/8)*2)-dch);}
	if ((root_x>((dskww/8)*1)+ow) & (root_x<((dskww/8)*2)+ow)& (root_y>((dskwh/8)*2)+oh) & (root_y<((dskwh/8)*3)+oh))
	{XMoveResizeWindow(disp,activewin,0+ow,0+oh,(((dskcw/8)*2)-dcw),((dskch/8)*4)-dch);}
	if ((root_x>((dskww/8)*6)+ow) & (root_x<((dskww/8)*7)+ow)& (root_y>((dskwh/8)*2)+oh) & (root_y<((dskwh/8)*3)+oh))
	{XMoveResizeWindow(disp,activewin,((dskcw/8)*6)+ow,0+oh,(((dskcw/8)*2)-dcw),((dskch/8)*4)-dch);}
	if ((root_x>((dskww/8)*1)+ow) & (root_x<((dskww/8)*2)+ow)& (root_y>((dskwh/8)*5)+oh) & (root_y<((dskwh/8)*6)+oh))
	{XMoveResizeWindow(disp,activewin,0+ow,((dskch/8)*4)+oh,(((dskcw/8)*2)-dcw),((dskch/8)*4)-dch);}
	if ((root_x>((dskww/8)*6)+ow) & (root_x<((dskww/8)*7)+ow)& (root_y>((dskwh/8)*5)+oh) & (root_y<((dskwh/8)*6)+oh))
	{XMoveResizeWindow(disp,activewin,((dskcw/8)*6)+ow,((dskch/8)*4)+oh,(((dskcw/8)*2)-dcw),((dskch/8)*4)-dch);}
    /*NorthEdge*/
 	if ((root_x>((dskww/8)*0)+ow) & (root_x<((dskww/8)*1)+ow)& (root_y>((dskwh/8)*0)+oh) & (root_y<((dskwh/8)*1)+oh))
	{XMoveResizeWindow(disp,activewin,((dskcw/8)*0)+ow,((dskch/8)*0)+oh,(((dskcw/8)*3)-dcw),((dskch/8)*1)-dch);}
	if ((root_x>((dskww/8)*1)+ow) & (root_x<((dskww/8)*3)+ow)& (root_y>((dskwh/8)*0)+oh) & (root_y<((dskwh/8)*1)+oh))
	{XMoveResizeWindow(disp,activewin,((dskcw/8)*0)+ow,((dskch/8)*0)+oh,(((dskcw/8)*3)-dcw),((dskch/8)*2)-dch);}
	if ((root_x>((dskww/8)*3)+ow) & (root_x<((dskww/8)*5)+ow)& (root_y>((dskwh/8)*0)+oh) & (root_y<((dskwh/8)*1)+oh))
	{XMoveResizeWindow(disp,activewin,(((dskcw/8)*2))+ow,((dskch/8)*0)+oh,(((dskcw/8)*4)-dcw),((dskch/8)*2)-dch);}
	if ((root_x>((dskww/8)*5)+ow) & (root_x<((dskww/8)*7)+ow)& (root_y>((dskwh/8)*0)+oh) & (root_y<((dskwh/8)*1)+oh))
	{XMoveResizeWindow(disp,activewin,((dskcw/8)*5)+ow,((dskch/8)*0)+oh,(((dskcw/8)*3)-dcw),((dskch/8)*2)-dch);}
	if ((root_x>((dskww/8)*7)+ow) & (root_x<((dskww/8)*8)+ow)& (root_y>((dskwh/8)*0)+oh) & (root_y<((dskwh/8)*1)+oh))
	{XMoveResizeWindow(disp,activewin,((dskcw/8)*5)+ow,((dskch/8)*0)+oh,(((dskcw/8)*3)-dcw),((dskch/8)*1)-dch);}
    /*SouthEdge*/
 	if ((root_x>((dskww/8)*0)+ow) & (root_x<((dskww/8)*1)+ow)& (root_y>((dskwh/8)*7)+oh) & (root_y<(((dskwh/8)*8)+oh)))
	{XMoveResizeWindow(disp,activewin,((dskcw/8)*0)+ow,((dskch/8)*7)+oh,(((dskcw/8)*3)-dcw),((dskch/8)*1)-dch);}
	if ((root_x>((dskww/8)*1)+ow) & (root_x<((dskww/8)*3)+ow)& (root_y>((dskwh/8)*7)+oh) & (root_y<((dskwh/8)*7.5)+oh))
	{XMoveResizeWindow(disp,activewin,((dskcw/8)*0)+ow,((dskch/8)*7)+oh,(((dskcw/8)*3)-dcw),((dskch/8)*2)-dch);}
	if ((root_x>((dskww/8)*3)+ow) & (root_x<((dskww/8)*5)+ow)& (root_y>((dskwh/8)*7)+oh) & (root_y<((dskwh/8)*7.5)+oh))
	{XMoveResizeWindow(disp,activewin,(((dskcw/8)*2))+ow,((dskch/8)*7)+oh,(((dskcw/8)*4)-dcw),((dskch/8)*2)-dch);}
	if ((root_x>((dskww/8)*5)+ow) & (root_x<((dskww/8)*7)+ow)& (root_y>((dskwh/8)*7)+oh) & (root_y<((dskwh/8)*7.5)+oh))
	{XMoveResizeWindow(disp,activewin,((dskcw/8)*5)+ow,((dskch/8)*7)+oh,(((dskcw/8)*3)-dcw),((dskch/8)*2)-dch);}
	if ((root_x>((dskww/8)*7)+ow) & (root_x<((dskww/8)*8)+ow)& (root_y>((dskwh/8)*7)+oh) & (root_y<(((dskwh/8)*8)+oh)))
	{XMoveResizeWindow(disp,activewin,((dskcw/8)*5)+ow,((dskch/8)*7)+oh,(((dskcw/8)*3)-dcw),((dskch/8)*1)-dch);}
	/*LeftEdge*/
 	if ((root_x>((dskww/8)*0)+ow) & (root_x<((dskww/8)*1)+ow)& (root_y>((dskwh/8)*1)+oh) & (root_y<((dskwh/8)*2)+oh))
	{XMoveResizeWindow(disp,activewin,((dskcw/8)*0)+ow,((dskch/8)*0)+oh,(((dskcw/8)*1)-dcw),((dskch/8)*2)-dch);}
	if ((root_x>((dskww/8)*0)+ow) & (root_x<((dskww/8)*1)+ow)& (root_y>((dskwh/8)*2)+oh) & (root_y<((dskwh/8)*3)+oh))
	{XMoveResizeWindow(disp,activewin,((dskcw/8)*0)+ow,((dskch/8)*0)+oh,(((dskcw/8)*2)-dcw),((dskch/8)*2)-dch);}
	if ((root_x>((dskww/8)*0)+ow) & (root_x<((dskww/8)*1)+ow)& (root_y>((dskwh/8)*3)+oh) & (root_y<((dskwh/8)*5)+oh))
	{XMoveResizeWindow(disp,activewin,((dskcw/8)*0)+ow,((dskch/8)*2)+oh,(((dskcw/8)*2)-dcw),((dskch/8)*4)-dch);}
	if ((root_x>((dskww/8)*0)+ow) & (root_x<((dskww/8)*1)+ow)& (root_y>((dskwh/8)*5)+oh) & (root_y<((dskwh/8)*6)+oh))
	{XMoveResizeWindow(disp,activewin,((dskcw/8)*0)+ow,((dskch/8)*6)+oh,(((dskcw/8)*2)-dcw),((dskch/8)*2)-dch);}
	if ((root_x>((dskww/8)*0)+ow) & (root_x<((dskww/8)*1)+ow)& (root_y>((dskwh/8)*6)+oh) & (root_y<((dskwh/8)*7)+oh))
	{XMoveResizeWindow(disp,activewin,((dskcw/8)*0)+ow,((dskch/8)*6)+oh,(((dskcw/8)*1)-dcw),((dskch/8)*2)-dch);}
    /*RightEdge*/
 	if ((root_x>((dskww/8)*7)+ow) & (root_x<((dskww/8)*8)+ow)& (root_y>((dskwh/8)*1)+oh) & (root_y<((dskwh/8)*2)+oh))
	{XMoveResizeWindow(disp,activewin,((dskcw/8)*7)+ow,((dskch/8)*0)+oh,(((dskcw/8)*1)-dcw),((dskch/8)*2)-dch);}
	if ((root_x>((dskww/8)*7)+ow) & (root_x<((dskww/8)*8)+ow)& (root_y>((dskwh/8)*2)+oh) & (root_y<((dskwh/8)*3)+oh))
	{XMoveResizeWindow(disp,activewin,((dskcw/8)*6)+ow,((dskch/8)*0)+oh,(((dskcw/8)*2)-dcw),((dskch/8)*2)-dch);}
	if ((root_x>((dskww/8)*7)+ow) & (root_x<((dskww/8)*8)+ow)& (root_y>((dskwh/8)*3)+oh) & (root_y<((dskwh/8)*5)+oh))
	{XMoveResizeWindow(disp,activewin,((dskcw/8)*6)+ow,((dskch/8)*2)+oh,(((dskcw/8)*2)-dcw),((dskch/8)*4)-dch);}
	if ((root_x>((dskww/8)*7)+ow) & (root_x<((dskww/8)*8)+ow)& (root_y>((dskwh/8)*5)+oh) & (root_y<((dskwh/8)*6)+oh))
	{XMoveResizeWindow(disp,activewin,((dskcw/8)*6)+ow,((dskch/8)*6)+oh,(((dskcw/8)*2)-dcw),((dskch/8)*2)-dch);}
	if ((root_x>((dskww/8)*7)+ow) & (root_x<((dskww/8)*8)+ow)& (root_y>((dskwh/8)*6)+oh) & (root_y<((dskwh/8)*7)+oh))
	{XMoveResizeWindow(disp,activewin,((dskcw/8)*7)+ow,((dskch/8)*6)+oh,(((dskcw/8)*1)-dcw),((dskch/8)*2)-dch);}
    if (win_move_desk)
	{
	 for (i=1;i<nmbrdesks+1;i++)
	  {
	   if ((root_x>(((dskww/8)*1)+((((dskww/8)*6)/nmbrdesks)*(i-1)))+ow)& (root_x<(((dskww/8)*1)+((((dskww/8)*6)/nmbrdesks)*i))+ow)
	    & (root_y>((dskwh/16)*15)+oh) & ((root_y<((dskwh/16)*16)+oh)))
	                              {Client_msg(disp, activewin, "_NET_WM_DESKTOP", i-1,
             0, 0, 0, 0);XCloseDisplay(disp);XUngrabPointer(disp, CurrentTime);return;}
	  }
    }
	;XCloseDisplay(disp);};
  }
disp = XOpenDisplay(NULL);
  XMapRaised(disp, winl[1]);
  XCloseDisplay(disp);XUngrabPointer(disp, CurrentTime);return;
}

void Print_screen_sections ()
{
int dpw=dskww/8;
int dph=dskwh/8;
int ow=deskZx+0;
int oh=deskZy+0;
double transparency = 0.4;	/* between 1.0 and 0.0 */
unsigned int opacity = (unsigned int) (0xffffffff * transparency);
Display *display = XOpenDisplay(NULL);
/*___________________________________________________________________________*/
int blackColor = BlackPixel(display, DefaultScreen(display));
int whiteColor = WhitePixel(display, DefaultScreen(display));

screen_num = DefaultScreen ( display );
XSetWindowAttributes xwinattrs;
xwinattrs.override_redirect = TRUE;
win = XCreateWindow(display, RootWindow(display,screen_num),0,0,dskcw,dskch,0,0,0,0,CWOverrideRedirect,&xwinattrs);
XSelectInput(display, win, StructureNotifyMask);
GC gc = XCreateGC(display, win, 0, NULL);
XSetForeground(display, gc, whiteColor);
XSetBackground(display, gc, blackColor);
XMapWindow(display,win );
XStringListToTextProperty ( &window_name, 1, &wname );
XSetWMProperties ( display, win, &wname, NULL, NULL, 0, NULL, NULL, NULL );
Atom atom_window_opacity = XInternAtom(display, "_NET_WM_WINDOW_OPACITY", TRUE);
XChangeProperty(display,win,atom_window_opacity,XA_CARDINAL,32,PropModeReplace,(unsigned char *) &opacity,1);
XFlush(display);
/*___________________________________________________________________________*/

char green[] = "#00FF00";
Colormap colormap;
XColor green_col;
colormap = DefaultColormap(display, 0);
XParseColor(display, colormap, green, &green_col);
XAllocColor(display, colormap, &green_col);
XSetForeground(display, gc, blackColor);
XFillRectangle(display, win, gc, 0, 0, dskww,dskwh);
XSetForeground(display, gc, green_col.pixel);
/*CenterVerticalB*/
XDrawRectangle(display, win, gc, ((dpw)*3.5)+ow, ((dph)*3.5)+oh, ((dpw)*0.5), ((dph)*1));
XDrawRectangle(display, win, gc, ((dpw)*4)+ow, ((dph)*3.5)+oh, ((dpw)*0.5), ((dph)*1));
/*CenterHorizontalB*/
XDrawRectangle(display, win, gc, ((dpw)*3.5)+ow, ((dph)*3)+oh, ((dpw)*1), ((dph)*0.5));
XDrawRectangle(display, win, gc, ((dpw)*3.5)+ow, ((dph)*4.5)+oh, ((dpw)*1), ((dph)*0.5));
/*CenterVertical*/
XDrawRectangle(display, win, gc, ((dpw)*3)+ow, ((dph)*3)+oh, ((dpw)*0.5), ((dph)*2));
XDrawRectangle(display, win, gc, ((dpw)*4.5)+ow, ((dph)*3)+oh, ((dpw)*0.5), ((dph)*2));
/*CenterHorizontal*/
XDrawRectangle(display, win, gc, ((dpw)*3)+ow, ((dph)*2)+oh, ((dpw)*2), ((dph)*1));
XDrawRectangle(display, win, gc, ((dpw)*3)+ow, ((dph)*5)+oh, ((dpw)*2), ((dph)*1));
/*EastWestVertical*/
XDrawRectangle(display, win, gc, ((dpw)*2)+ow, ((dph)*2)+oh, ((dpw)*1), ((dph)*2));
XDrawRectangle(display, win, gc, ((dpw)*2)+ow, ((dph)*4)+oh, ((dpw)*1), ((dph)*2));
XDrawRectangle(display, win, gc, ((dpw)*5)+ow, ((dph)*2)+oh, ((dpw)*1), ((dph)*2));
XDrawRectangle(display, win, gc, ((dpw)*5)+ow, ((dph)*4)+oh, ((dpw)*1), ((dph)*2));
/*MiddleCross*/
XDrawRectangle(display, win, gc, ((dpw)*3)+ow, ((dph)*1)+oh, ((dpw)*2), ((dph)*1));
XDrawRectangle(display, win, gc, ((dpw)*3)+ow, ((dph)*6)+oh, ((dpw)*2), ((dph)*1));
XDrawRectangle(display, win, gc, ((dpw)*1)+ow, ((dph)*3)+oh, ((dpw)*1), ((dph)*2));
XDrawRectangle(display, win, gc, ((dpw)*6)+ow, ((dph)*3)+oh, ((dpw)*1), ((dph)*2));
/*CornersMiddleScreen*/
XDrawRectangle(display, win, gc, ((dpw)*1)+ow, ((dph)*1)+oh, ((dpw)*2), ((dph)*1));
XDrawRectangle(display, win, gc, ((dpw)*5)+ow, ((dph)*1)+oh, ((dpw)*2), ((dph)*1));
XDrawRectangle(display, win, gc, ((dpw)*1)+ow, ((dph)*6)+oh, ((dpw)*2), ((dph)*1));
XDrawRectangle(display, win, gc, ((dpw)*5)+ow, ((dph)*6)+oh, ((dpw)*2), ((dph)*1));
XDrawRectangle(display, win, gc, ((dpw)*1)+ow, ((dph)*2)+oh, ((dpw)*1), ((dph)*1));
XDrawRectangle(display, win, gc, ((dpw)*6)+ow, ((dph)*2)+oh, ((dpw)*1), ((dph)*1));
XDrawRectangle(display, win, gc, ((dpw)*1)+ow, ((dph)*5)+oh, ((dpw)*1), ((dph)*1));
XDrawRectangle(display, win, gc, ((dpw)*6)+ow, ((dph)*5)+oh, ((dpw)*1), ((dph)*1));
/*NorthEdge*/
XDrawRectangle(display, win, gc, ((dpw)*0)+ow, ((dph)*0)+oh, ((dpw)*1), ((dph)*1));
XDrawRectangle(display, win, gc, ((dpw)*1)+ow, ((dph)*0)+oh, ((dpw)*2), ((dph)*1));
XDrawRectangle(display, win, gc, ((dpw)*3)+ow, ((dph)*0)+oh, ((dpw)*2), ((dph)*1));
XDrawRectangle(display, win, gc, ((dpw)*5)+ow, ((dph)*0)+oh, ((dpw)*2), ((dph)*1));
XDrawRectangle(display, win, gc, ((dpw)*7)+ow, ((dph)*0)+oh, ((dpw)*1), ((dph)*1));
/*SouthEdge*/
XDrawRectangle(display, win, gc, ((dpw)*0)+ow, ((dph)*7)+oh, ((dpw)*1), (((dph)*1)-1));
XDrawRectangle(display, win, gc, ((dpw)*1)+ow, ((dph)*7)+oh, ((dpw)*2), ((dph)*0.5));
XDrawRectangle(display, win, gc, ((dpw)*3)+ow, ((dph)*7)+oh, ((dpw)*2), ((dph)*0.5));
XDrawRectangle(display, win, gc, ((dpw)*5)+ow, ((dph)*7)+oh, ((dpw)*2), ((dph)*0.5));
XDrawRectangle(display, win, gc, ((dpw)*7)+ow, ((dph)*7)+oh, ((dpw)*1), (((dph)*1)-4));
/*LeftEdge*/
XDrawRectangle(display, win, gc, ((dpw)*0)+ow, ((dph)*1)+oh, ((dpw)*1), ((dph)*1));
XDrawRectangle(display, win, gc, ((dpw)*0)+ow, ((dph)*2)+oh, ((dpw)*1), ((dph)*1));
XDrawRectangle(display, win, gc, ((dpw)*0)+ow, ((dph)*3)+oh, ((dpw)*1), ((dph)*2));
XDrawRectangle(display, win, gc, ((dpw)*0)+ow, ((dph)*5)+oh, ((dpw)*1), ((dph)*1));
XDrawRectangle(display, win, gc, ((dpw)*0)+ow, ((dph)*6)+oh, ((dpw)*1), ((dph)*1));
/*RightEdge*/
XDrawRectangle(display, win, gc, ((dpw)*7)+ow, ((dph)*1)+oh, ((dpw)*1), ((dph)*1));
XDrawRectangle(display, win, gc, ((dpw)*7)+ow, ((dph)*2)+oh, ((dpw)*1), ((dph)*1));
XDrawRectangle(display, win, gc, ((dpw)*7)+ow, ((dph)*3)+oh, ((dpw)*1), ((dph)*2));
XDrawRectangle(display, win, gc, ((dpw)*7)+ow, ((dph)*5)+oh, ((dpw)*1), ((dph)*1));
XDrawRectangle(display, win, gc, ((dpw)*7)+ow, ((dph)*6)+oh, ((dpw)*1), ((dph)*1));
if (win_move_desk)
  {
   for (i=1;i<nmbrdesks;i++)
	   {
	    XDrawRectangle(display, win, gc, (((dpw)*1)+((((dpw)*6)/nmbrdesks)*i))+ow,((dph)*7.5)+oh,
	                                 (((dpw)*6)/nmbrdesks),((dph)*0.5));									
	   }
  }
XFlush(display);


return ;
}

void Print_screen_menu ()
{
Display *dpy = XOpenDisplay(NULL);
Window win=XDefaultRootWindow(dpy);
XMapWindow(dpy,win );
char green[] = "#00FF00";
Colormap colormap;
XColor green_col;
colormap = DefaultColormap(dpy, 0);
XParseColor(dpy, colormap, green, &green_col);
XAllocColor(dpy, colormap, &green_col);
GC gc = XCreateGC(dpy,win, 0, NULL);
XSetForeground(dpy, gc, green_col.pixel);
XSetSubwindowMode(dpy, gc, IncludeInferiors);
XSetFunction(dpy, gc, GXinvert);
XDrawLine(dpy, win, gc,(dskww/2),0,(dskww/2),dskwh);
XDrawLine(dpy, win, gc,0,(dskwh/2),dskww,(dskwh/2));
XFlush(dpy);
XCloseDisplay(dpy);
return ;
}

void Tile_menu()
{
Max_remove ();
Display *dpy;
Window root;
Window ret_root;
Window ret_child;
int root_x=10;
int root_y=10;
int win_x;
int win_y;
unsigned int mask;
dpy = XOpenDisplay(NULL);
root = XDefaultRootWindow(dpy);
 /*pointer feedback*/
int status;
Cursor cursor;
cursor = XCreateFontCursor(dpy, XC_crosshair);
status = XGrabPointer(dpy, root, False,ButtonPressMask|ButtonReleaseMask,
                      GrabModeSync,GrabModeAsync, root, cursor, CurrentTime);
  if (status != GrabSuccess) 
	{
     fputs("ERROR: Cannot grab mouse.\n", stderr);
     return ;
    }
XAllowEvents(dpy, SyncPointer, CurrentTime);
/*pointer feedback end -set this when no longer needed- XUngrabPointer(disp, CurrentTime);*/
   while (1)
	{
	  if(XQueryPointer(dpy, root, &ret_root, &ret_child, &root_x, &root_y,
		     		   &win_x, &win_y, &mask))
		{
		  if (mask & (Button1Mask)){return;}
		  if ((root_x>(0)) & (root_x<(dskww/2))& (root_y>(0)) & (root_y<(dskwh/2)))
		  {alternative_layout=0;tile_stripes=0;Prepare_tile();}
	      if ((root_x>(dskww/2)) & (root_x<(dskww))& (root_y>(0)) & (root_y<(dskwh/2)))
		  {alternative_layout=1;tile_stripes=0;Prepare_tile();}
		  if ((root_x>(0)) & (root_x<(dskww/2))& (root_y>(dskwh/2)) & (root_y<(dskwh)))
		  {tile_vert=0;tile_stripes=1;Prepare_tile();}
		  if ((root_x>(dskww/2)) & (root_x<(dskww))& (root_y>(dskwh/2)) & (root_y<(dskwh)))
		  {tile_vert=1;tile_stripes=1;Prepare_tile();}
		}
	}
XUngrabPointer(dpy, CurrentTime);
}

void Exchange_select ()
{
Display *disp;
disp = XOpenDisplay(NULL);
tmp_win_1=Select_Window(disp);
tmp_win_2=Select_Window(disp);
XCloseDisplay(disp);
Sort_list_xchg();
Tile ();    
}

void Exchange_active ()
{
tmp_win_1=activewin;
Display *disp;
disp = XOpenDisplay(NULL);
tmp_win_2=Select_Window(disp);
XCloseDisplay(disp);
Sort_list_xchg();
Tile ();    
}

void Stripe_wins ()
{
int ow=deskZx+0;
int oh=deskZy+0;
dskch=dskch-offset_bottom;oh=deskZy+offset_top;dskch=dskch-offset_top;
dskcw=dskcw-offset_right;ow=deskZx+offset_left;dskcw=dskcw-offset_left;
Display *disp;
disp = XOpenDisplay(NULL);
  for (i = 1; i < winondesk+1;i++ )
	{
    if (tile_vert)
	  {XMoveResizeWindow(disp,winl[i],(((i-1)*(dskcw/winondesk))+ow),0+oh,((dskcw/winondesk)-dcw),dskch-dch);}
	else
	  {XMoveResizeWindow(disp,winl[i],0+ow,(((i-1)*(dskch/winondesk))+oh),dskcw-dcw,((dskch/winondesk)-dch));}
    }
 XCloseDisplay(disp); 
 return;
}

void Get_info_desk ()
{
Display *disp;
disp = XOpenDisplay(NULL);
Window root = DefaultRootWindow(disp);
unsigned long *cur_desktop = NULL;
cur_desktop = (unsigned long *)Get_property(disp, root,
            XA_CARDINAL,"_NET_CURRENT_DESKTOP", NULL);
activedesk= (int) *cur_desktop; 
cur_desktop = (unsigned long *)Get_property(disp, root,
            XA_CARDINAL,"_NET_NUMBER_OF_DESKTOPS", NULL);
nmbrdesks= (int) *cur_desktop; 
cur_desktop = (unsigned long *)Get_property(disp, root,
            XA_CARDINAL,"_NET_DESKTOP_GEOMETRY",NULL);
dskrw= (int) cur_desktop[0];
dskrh= (int) cur_desktop[1];
cur_desktop = (unsigned long *)Get_property(disp, root,
            XA_CARDINAL,"_NET_WORKAREA",NULL);
deskZx= (int) cur_desktop[0];
deskZy= (int) cur_desktop[1];
dskww= (int) cur_desktop[2];
dskwh= (int) cur_desktop[3];
dskcw=dskww;
dskch=dskwh;
XCloseDisplay(disp);
return;
}

void Get_info_active ()
{
Display *disp;
disp = XOpenDisplay(NULL);
activewin= Get_active_window(disp);
XCloseDisplay(disp);
return;
}

void Get_info_win_desk ()
{
winondesk=1;
Display *disp; 
Window *client_list;
unsigned long client_list_size;
int i;
unsigned long *desktop;
disp = XOpenDisplay(NULL);
if ((client_list=Get_client_list(disp,&client_list_size))==NULL) {return;}          
for (i = 0; i < client_list_size / sizeof(Window); i++) 
	{
    if ((desktop=(unsigned long *)Get_property(disp, client_list[i],
                                               XA_CARDINAL, "_NET_WM_DESKTOP", NULL)) == NULL) 
	  {
       desktop = (unsigned long *)Get_property(disp, client_list[i],
                                               XA_CARDINAL, "_WIN_WORKSPACE", NULL);
      }
    if (desktop!=0)
    {if ((signed long)*desktop==activedesk) {winl[winondesk]=client_list[i];winondesk=winondesk+1;}}   
	}
winondesk=winondesk-1;
XCloseDisplay(disp);
return;
}

void Sort_list()
{
int wintemp=0;
for (i =1; i < winondesk+1; i++)
	{
	if (winl[i]==activewin)
	  {wintemp=winl[1]; winl[1]=activewin;winl[i]=wintemp;}
	}
}

void Sort_list_xchg()
{
int b;
for (i =1; i < winondesk+1;i++)
	{
	 if (winl[i]==tmp_win_1){b=i;};
	}
for (i =1; i < winondesk+1;i++)
	{
	 if (winl[i]==tmp_win_2){winl[b]=tmp_win_2 ;winl[i]=tmp_win_1; break;};
	}
}


void Print_info ()
{
printf("Info:\n");
printf ("Version: %s\n",version);
printf("Windows on desktop:\n");
for (i = 1; i < winondesk+1;i++ ){printf("Win id: %i\n",winl[i]);}   
printf("Number of Windows on desk: %i   \n",winondesk) ;
printf("Active window id: %i\n",activewin);
printf("Active desktop: %i   ",activedesk);printf("Desks %i   \n",nmbrdesks);
printf("Real width %i  ",dskrw);printf("Real height %i \n ",dskrh);
printf("Work width %i  ",dskww);printf("Work_height %i \n ",dskwh);
printf("Desk Zero x: %i  ",deskZx); printf("Desk Zero y: %i  \n ",deskZy) ;
printf("Runtime Var:\n");
printf("Custm width %f   ",dskcw); printf("Custom height %f\n",dskch) ;
printf("i %i   ",i);printf("tmp_win_1 %i    ",tmp_win_1) ;printf("tmp_win_2 %i\n",tmp_win_2) ;

}

void Print_info_t ()
{
Sort_list ();
Print_info ();
}

static int Client_msg(Display *disp, Window win, char *msg, unsigned long data0,
  unsigned long data1,unsigned long data2, unsigned long data3,
  unsigned long data4)
{
XEvent event;
long mask = SubstructureRedirectMask | SubstructureNotifyMask;
event.xclient.type = ClientMessage;
event.xclient.serial = 0;
event.xclient.send_event = True;
event.xclient.message_type = XInternAtom(disp, msg, False);
event.xclient.window = win;
event.xclient.format = 32;
event.xclient.data.l[0] = data0;
event.xclient.data.l[1] = data1;
event.xclient.data.l[2] = data2;
event.xclient.data.l[3] = data3;
event.xclient.data.l[4] = data4;
  if (XSendEvent(disp, DefaultRootWindow(disp), False, mask, &event)) 
	{return EXIT_SUCCESS; }
  else 
	{fprintf(stderr, "Cannot send %s event.\n", msg); return EXIT_FAILURE; }
}

static Window Get_active_window(Display *disp) 
{
char *prop;
unsigned long size;
Window ret = (Window)0;
prop = Get_property(disp, DefaultRootWindow(disp), XA_WINDOW, 
                    "_NET_ACTIVE_WINDOW", &size);
if (prop){ret = *((Window*)prop); g_free(prop);}
return(ret);    
}

static gchar *Get_property (Display *disp, Window win, /*{{{*/
        Atom xa_prop_type, gchar *prop_name, unsigned long *size) 
{
Atom xa_prop_name;
Atom xa_ret_type;
int ret_format;
unsigned long ret_nitems;
unsigned long ret_bytes_after;
unsigned long tmp_size;
unsigned char *ret_prop;
gchar *ret;
xa_prop_name = XInternAtom(disp, prop_name, False);
  if (XGetWindowProperty(disp, win, xa_prop_name, 0, MAX_PROPERTY_VALUE_LEN / 4, False,
                         xa_prop_type, &xa_ret_type, &ret_format,     
            &ret_nitems, &ret_bytes_after, &ret_prop) != Success) 
	{return NULL;}
  if (xa_ret_type != xa_prop_type) 
    {XFree(ret_prop); return NULL;}
tmp_size = (ret_format / 8) * ret_nitems;
ret = g_malloc(tmp_size + 1);
memcpy(ret, ret_prop, tmp_size);
ret[tmp_size] = '\0';
  if (size) 
    { *size = tmp_size; }
XFree(ret_prop);
return ret;
}

static Window *Get_client_list (Display *disp, unsigned long *size) 
{
Window *client_list;
  if ((client_list=(Window *)Get_property(disp, DefaultRootWindow(disp), 
                                          XA_WINDOW, "_NET_CLIENT_LIST", size)) == NULL) 
	{
      if ((client_list = (Window *)Get_property(disp, DefaultRootWindow(disp), 
                                                XA_CARDINAL, "_WIN_CLIENT_LIST", size)) == NULL) 
		{
         fputs("Cannot get client list properties. \n"
               "(_NET_CLIENT_LIST or _WIN_CLIENT_LIST)"
               "\n", stderr);
         return NULL;
        }
    }
return client_list;
}

void Count_win (char *label)
{
Display *disp = XOpenDisplay(NULL);
assert(disp);
int blackColor = BlackPixel(disp, DefaultScreen(disp));
int whiteColor = WhitePixel(disp, DefaultScreen(disp));
wcount = XCreateSimpleWindow(disp, DefaultRootWindow(disp), 0, 0, 
		                     50, 50, 0, blackColor, blackColor);
XSelectInput(disp, wcount, StructureNotifyMask);
XMapWindow(disp, wcount);
GC gc = XCreateGC(disp, wcount, 0, NULL);
XSetForeground(disp, gc, whiteColor);
XMoveResizeWindow(disp,wcount,(dskww/2)-25,(dskwh/2)-25,50,50);
for(;;) 
 {
  XEvent e;
  XNextEvent(disp, &e);
  if (e.type == MapNotify){ break;}
 }
XDrawString ( disp,wcount,gc,23,29,label,strlen (label));
XFlush(disp);
return;
}	  

void Refresh_count (char *label)
{
Display *disp = XOpenDisplay(NULL);
GC gc = XCreateGC(disp, wcount, 0, NULL);
int whiteColor = WhitePixel(disp, DefaultScreen(disp));
XSetForeground(disp, gc, whiteColor);
XClearWindow (disp,wcount);
XDrawString ( disp,wcount,gc,23,29,label,strlen (label));
XFlush(disp);
return;
}


void Tile_choose_win()
{
char nrwindows[0];
char mystring[0];
winondesk=1;winl[0]=12;winl[1]=793;
Display *disp;
strcpy(nrwindows,"0");
Count_win (nrwindows);
while (1)
 {
  disp = XOpenDisplay(NULL);
   if (winl[winondesk-1]!=winl[winondesk-2])
   {
    winl[winondesk]=Select_Window(disp);
    mystring[0] = 0x30 + (winondesk);
	winondesk=winondesk+1;
    Refresh_count (nrwindows);
   }
   else {break;};
 }
winondesk=winondesk-2;Tile();
}

void Auto_op_win ()
{
Display *disp;
while (1)
 {
  disp = XOpenDisplay(NULL);
  Get_info_desk ();
  Get_info_win_desk ();
  Get_info_active ();
  double transparency = 0.5;	/* between 1.0 and 0.0 */
  unsigned int opacity = (unsigned int) (0xffffffff * transparency);
  Atom atom_window_opacity = XInternAtom(disp, "_NET_WM_WINDOW_OPACITY", FALSE);
  for (i =1; i < winondesk+1; i++)
      {XChangeProperty(disp,winl[i],atom_window_opacity,XA_CARDINAL,32,PropModeReplace,(unsigned char *) &opacity,1);}
  XFlush(disp);
  XCloseDisplay(disp);
  usleep (120000);
 }
return;
}

void Auto_raise_active ()
{
Display *disp;
	while (1)
	{
	disp = XOpenDisplay(NULL);
	activewin= Get_active_window(disp);
	Client_msg(disp, activewin, "_NET_ACTIVE_WINDOW", 0, 0, 0, 0, 0);
	/*XMapRaised(disp, activewin);*/
	XCloseDisplay(disp);
	usleep (120000);
    }
return;
}

static Window Select_Window(Display *disp) {
int status;
Cursor cursor;
XEvent event;
Window target_win = None, root = DefaultRootWindow(disp);
int buttons = 0;
int dummyi;
unsigned int dummy;
/* Make the target cursor */
cursor = XCreateFontCursor(disp, XC_crosshair);
/* Grab the pointer using target cursor, letting it room all over */
status = XGrabPointer(disp, root, False,ButtonPressMask|ButtonReleaseMask,
                      GrabModeSync,GrabModeAsync, root, cursor, CurrentTime);
  if (status != GrabSuccess) 
	{
     fputs("ERROR: Cannot grab mouse.\n", stderr);
     return 0;
    }
  while ((target_win == None) || (buttons != 0)) 
	{
     /* allow one more event */
     XAllowEvents(disp, SyncPointer, CurrentTime);
     XWindowEvent(disp, root, ButtonPressMask|ButtonReleaseMask, &event);
       switch (event.type) 
	     {
          case ButtonPress:
            if (target_win == None) 
			  {
               target_win = event.xbutton.subwindow; /* window selected */
                 if (target_win == None) target_win = root;
              }
           buttons++;
           break;
          case ButtonRelease:
            if (buttons > 0) 
            buttons--;
            break;
          }
    } 
XUngrabPointer(disp, CurrentTime);      /* Done with pointer */
  if (XGetGeometry (disp, target_win, &root, &dummyi, &dummyi,
                    &dummy, &dummy, &dummy, &dummy) && target_win != root) 
	{
    target_win = XmuClientWindow (disp, target_win);
    }  
return(target_win);
}

void Del_desk ()
{
Display *disp;
disp = XOpenDisplay(NULL);
Client_msg(disp, DefaultRootWindow(disp), "_NET_NUMBER_OF_DESKTOPS", 
           nmbrdesks-1, 0, 0, 0, 0); 
XCloseDisplay(disp);return;
}

void Empty_desk ()
{
Display *disp;
disp = XOpenDisplay(NULL);
  for (i = 0; i < nmbrdesks;i++ )
	{
	activedesk=i;
	Get_info_win_desk ();
	  if (winondesk==0)
		{
		Client_msg(disp, DefaultRootWindow(disp), "_NET_CURRENT_DESKTOP", 
                   i, 0, 0, 0, 0);
		XCloseDisplay(disp);
		return ;
		}
	}
Client_msg(disp, DefaultRootWindow(disp), "_NET_NUMBER_OF_DESKTOPS", 
           i+1, 0, 0, 0, 0); 
Client_msg(disp, DefaultRootWindow(disp), "_NET_CURRENT_DESKTOP", 
      	   i, 0, 0, 0, 0);
XCloseDisplay(disp);return;
}

void Next_win ()
{
for (i =1; i < winondesk+1;)
	{
	if (winl[i]==activewin)
	  {
	   Display *disp;
       disp = XOpenDisplay(NULL);
       if (winl[i+1]!=0)
	      {Client_msg(disp, winl[i+1], "_NET_ACTIVE_WINDOW", 
                   0, 0, 0, 0, 0);i=winondesk+2;}
	   else {Client_msg(disp, winl[1], "_NET_ACTIVE_WINDOW", 
                   0, 0, 0, 0, 0);i=winondesk+2;}
       XCloseDisplay(disp);
	   }
    i=i+1;
	}
}
	
void Max_remove ()
{
Display *disp;
disp = XOpenDisplay(NULL);
for (i =1; i < winondesk+1; i++)
	{
	Client_msg(disp, winl[i], "_NET_WM_STATE", 
	           XInternAtom(disp,"_NET_WM_STATE_TOGGLE",False), 
	           XInternAtom(disp,"_NET_WM_STATE_MAXIMIZED_VERT", False),
	           XInternAtom (disp,"_NET_WM_STATE_MAXIMIZED_HORZ", False),0,0);
	}
XCloseDisplay(disp);
return;
}

void Min_remove ()
{
Display *disp;
disp = XOpenDisplay(NULL);
for (i =1; i < winondesk+1; i++)
	{Client_msg(disp, winl[i], "_NET_ACTIVE_WINDOW",0, 0, 0, 0, 0);}
XCloseDisplay(disp);return;
}

void Win_raise ()
{
Display *disp;
disp = XOpenDisplay(NULL);
for (i =1; i < winondesk+1; i++) {XMapRaised(disp, winl[i]);}
XCloseDisplay(disp);
return;
}

void Get_win_layout ()
{
int j;
int winondesk_tmp=winondesk;
float win_pm_tmp [20][9][4]={{{0}},
                            {{0,0,1,1}},
                            {{0,0,0.5,1},{0.5,0,0.5,1}}, 
                            {{0,0,0.5,1},{0.5,0,0.5,0.5},{0.5,0.5,0.5,0.5}},
                            {{0,0,0.5,0.5},{0,0.5,0.5,0.5},{0.5,0,0.5,0.5},{0.5,0.5,0.5,0.5}}, 
                            {{0,0,0.333,1},{0.333,0,0.333,0.5},{0.666,0,0.333,0.5},{0.333,0.5,0.333,0.5},{0.666,0.5,0.333,0.5}},
                            {{0,0,0.333,0.5},{0.333,0,0.333,0.5},{0.666,0,0.333,0.5},{0.333,0.5,0.333,0.5},{0.666,0.5,0.333,0.5},{0,0.5,0.333,0.5}},
                            {{0,0,0.333,0.333},{0.333,0,0.333,0.333},{0.666,0,0.333,0.333},{0,0.333,0.333,0.333},{0.333,0.333,0.333,0.333},{0.666,0.333,0.333,0.333},{0,0.666,1,0.333}}, 
                            {{0,0,0.5,0.333},{0.5,0,0.5,0.333},{0,0.333,0.333,0.333},{0.333,0.333,0.333,0.333},{0.666,0.333,0.333,0.333},{0,0.666,0.333,0.333},{0.333,0.666,0.333,0.333},{0.666,0.666,0.333,0.333}}, 
                            {{0,0,0.333,0.333},{0.333,0,0.333,0.333},{0.666,0,0.333,0.333},{0,0.333,0.333,0.333},{0.333,0.333,0.333,0.333},{0.666,0.333,0.333,0.333},{0,0.666,0.333,0.333},{0.333,0.666,0.333,0.333},{0.666,0.666,0.333,0.333}},
                            {{0}},
						    {{0,0,1,1}}, 
                            {{0,0,0.75,1},{0.75,0,0.25,1}}, 
                            {{0,0,0.75,1},{0.75,0,0.25,0.5},{0.75,0.5,0.25,0.5}}, 
                            {{0,0,0.5,1},{0.5,0,0.5,0.333},{0.5,0.333,0.5,0.333},{0.5,0.666,0.5,0.333}}, 
                            {{0,0,0.5,0.5},{0.5,0,0.5,0.5},{0.333,0.5,0.333,0.5},{0.666,0.5,0.333,0.5},{0,0.5,0.333,0.5}},
                            {{0,0,0.333,0.5},{0.333,0,0.333,0.5},{0.666,0,0.333,0.5},{0.333,0.5,0.333,0.5},{0.666,0.5,0.333,0.5},{0,0.5,0.333,0.5}}, 
                            {{0,0,0.333,0.333},{0.333,0,0.333,0.333},{0.666,0,0.333,0.333},{0,0.333,0.333,0.333},{0.333,0.333,0.333,0.333},{0.666,0.333,0.333,0.333},{0,0.666,1,0.333}}, 
                            {{0,0,0.5,0.333},{0.5,0,0.5,0.333},{0,0.333,0.333,0.333},{0.333,0.333,0.333,0.333},{0.666,0.333,0.333,0.333},{0,0.666,0.333,0.333},{0.333,0.666,0.333,0.333},{0.666,0.666,0.333,0.333}}, 
                            {{0,0,0.333,0.333},{0.333,0,0.333,0.333},{0.666,0,0.333,0.333},{0,0.333,0.333,0.333},{0.333,0.333,0.333,0.333},{0.666,0.333,0.333,0.333},{0,0.666,0.333,0.333},{0.333,0.666,0.333,0.333},{0.666,0.666,0.333,0.333}}};
if (alternative_layout) {winondesk_tmp=winondesk+10;};
for( i=0;i<9;i++) 
   {for( j=0;j<4;j++) 
       {win_pm[i][j]=win_pm_tmp[winondesk_tmp][i][j];}
   }
return;
}

void refresh(void)
{
XFillRectangle ( display, win, black_gc, BUTTON1_X, BUTTON1_Y,BUTWIDTH, BUTHEIGHT );
XFillRectangle ( display, win, black_gc, BUTTON2_X, BUTTON2_Y,BUTWIDTH, BUTHEIGHT );
XFillRectangle ( display, win, black_gc, BUTTON3_X, BUTTON3_Y,BUTWIDTH, BUTHEIGHT );
XFillRectangle ( display, win, black_gc, BUTTON4_X, BUTTON4_Y,BUTWIDTH, BUTHEIGHT );

XFillRectangle ( display, win, color_gc, BUTTON1_X+2, BUTTON1_Y+2,BUTWIDTH-4, BUTHEIGHT-4 );
XFillRectangle ( display, win, color_gc, BUTTON2_X+2, BUTTON2_Y+2,BUTWIDTH-4, BUTHEIGHT-4 );
XFillRectangle ( display, win, color_gc, BUTTON3_X+2, BUTTON3_Y+2,BUTWIDTH-4, BUTHEIGHT-4 );
XFillRectangle ( display, win, color_gc, BUTTON4_X+2, BUTTON4_Y+2,BUTWIDTH-4, BUTHEIGHT-4 );
Get_win_layout ();
for (i =1; i < winondesk+1; i++) 
   {
	XDrawRectangle(display,win,black_gc,(BUTTON1_X+(BUTWIDTH*win_pm [i-1][0])),(BUTTON1_Y+( BUTHEIGHT*win_pm [i-1][1])),
                                                    (win_pm [i-1][2]*BUTWIDTH),(win_pm [i-1][3]*BUTHEIGHT));
    XDrawRectangle(display,win,black_gc,BUTTON3_X+(((i-1)*(BUTWIDTH/winondesk))),BUTTON3_Y,
                                                          ((BUTWIDTH/winondesk)),BUTHEIGHT );
    XDrawRectangle(display,win,black_gc,BUTTON4_X,BUTTON4_Y+(((i-1)*(BUTHEIGHT/winondesk))),
                                    BUTWIDTH,((BUTHEIGHT/winondesk)));
    }
alternative_layout=1;
Get_win_layout ();
for (i =1; i < winondesk+1; i++) 
    {XDrawRectangle(display,win,black_gc,(BUTTON2_X+(BUTWIDTH*win_pm [i-1][0])),(BUTTON2_Y+( BUTHEIGHT*win_pm [i-1][1])),
                                                     (win_pm [i-1][2]*BUTWIDTH),(win_pm [i-1][3]*BUTHEIGHT));}
alternative_layout=0;
}

unsigned char get_xchar(void)
{
unsigned char key;
key = 0;
while ((key==0)&&(clicked==FALSE)&&(rightclicked==FALSE))
  {
  usleep(10000);
  XNextEvent ( display, &event );
  if (event.type==KeyPress)
	 {
	 keysym = XLookupKeysym ( &(event.xkey), 0 );
	 if (keysym <= 255)
		{
		key = (unsigned char) keysym;
		if (cpslock==1)
	       {
		    if (key==',') {key='?';}
			if (key==';') {key='.';}
			if (key==':') {key='/';}
		    }
		cpslock = 0;
		}
	 else
		{
		switch ( keysym )
			{
			case  XK_KP_1:
			case XK_KP_End:
			key = 27;
			break;
			case  XK_KP_2:
			case XK_KP_Down:
			key = '2';
			break;
			case  XK_KP_3:
			case XK_KP_Page_Down:
			key = '3';
			break;
			case  XK_KP_4:
			case XK_KP_Left:
			key = '4';
			break;
            case XK_Escape:
			key = 27;
			break;
			default:
			key = '?';
			break;
			}
		}
	 }
  else
	 {
	  if (event.type==Expose)
		 {
		 refresh();
		 while (XCheckTypedEvent(display, Expose, &event));
		 }
	  else
		 {
		 if (event.type==ButtonPress)
			{
			if (event.xbutton.window==win)
		       {
				if(event.xbutton.button==Button1)
				  {
				  clicked = TRUE;
		          mousex = event.xbutton.x;
				  mousey = event.xbutton.y;
				  }
				else
				  {rightclicked = TRUE;}
				}
			}
		 else
			{if (event.type==ButtonRelease){refresh();}}
		  }
	}
  }
return key;
}

int Graphics_init ( unsigned int win_width, unsigned int win_height )
{
BUTWIDTH=(BUTHEIGHT*(((float) (dskcw/dskch))));
WIN_HEIGHT=BUTHEIGHT+1;
WIN_WIDTH=(BUTWIDTH*4)+16;
BUTTON1_X=0;
BUTTON1_Y=0;
BUTTON2_X=(BUTWIDTH)+5;
BUTTON2_Y=0;
BUTTON3_X=(BUTWIDTH*2)+10;
BUTTON3_Y=0;
BUTTON4_X=(BUTWIDTH*3)+15;
BUTTON4_Y=0;
display = XOpenDisplay ( display_name );
if ( !display ){ return FALSE;}
Window root;
Window ret_root;
Window ret_child;
int root_x=10;
int root_y=10;
int win_x;
int win_y;
unsigned int mask;
root = XDefaultRootWindow(display);
if(XQueryPointer(display,root,&ret_root,&ret_child,&root_x,&root_y,&win_x,&win_y, &mask));
int i, j;
screen_num = DefaultScreen ( display );
XSetWindowAttributes xwinattrs;
xwinattrs.override_redirect = TRUE;
if (dskww<WIN_WIDTH+root_x){root_x=dskww-WIN_WIDTH;};
if (dskwh<WIN_HEIGHT+root_y){root_y=dskwh-WIN_HEIGHT;};
win = XCreateWindow(display, RootWindow(display,screen_num), root_x, root_y,WIN_WIDTH, WIN_HEIGHT,0,
                   CopyFromParent, InputOutput, CopyFromParent  ,CWOverrideRedirect, &xwinattrs);
XSelectInput(display, win, ExposureMask | KeyPressMask | ButtonPressMask | KeyReleaseMask | ButtonReleaseMask | StructureNotifyMask);
XStringListToTextProperty ( &window_name, 1, &wname );
XSetWMProperties ( display, win, &wname, NULL, NULL, 0, NULL, NULL, NULL );
black_gc = XCreateGC ( display, win, gc_valuemask, &gc_values );
color_gc = XCreateGC ( display, win, gc_valuemask, &gc_values );
/* load default color scheme */
/* red */
for ( i=0, j=0; i<64; i++, j++ )
	{
	color_data[i][0] = 1024 * j;
	color_data[i][1] = color_data[i][2] = 0;
	}
   /* green */
for ( i=64, j=0; i<128; i++, j++ )
    {
    color_data[i][1] = 1024 * j;
    color_data[i][0] = color_data[i][2] = 0;
    }
   /* blue */
for ( i=128, j=0; i<192; i++, j++ )
    {
    color_data[i][2] = 1024 * j;
    color_data[i][0] = color_data[i][1] = 0;
    }
   /* white */
for ( i=192, j=0; i<256; i++, j++ )
    {color_data[i][0] = color_data[i][1] = color_data[i][2] = j * 1024;}
   /* yellow */
    color_data[192][0] = 63 * 1024;
    color_data[192][1] = 63 * 1024;
    color_data[192][2] = 32 * 1024;
for ( i=0; i<256; i++ )
    {
    color_info.red   = color_data[i][0];
    color_info.green = color_data[i][1];
    color_info.blue  = color_data[i][2];
    XAllocColor ( display, DefaultColormap ( display, screen_num ), &color_info );
    color_table[i] = color_info.pixel;
    }
XSetForeground ( display, black_gc, color_table[0] );
XSetForeground ( display, color_gc, color_table[224] );
XMapWindow ( display, win );
return TRUE;
}

void Graphics_shutdown ()
{
   XFreeGC ( display, black_gc );
   XFreeGC ( display, color_gc );
   XCloseDisplay ( display );
}

void Menu_gui ()
{
if (!Graphics_init (WIN_WIDTH , WIN_HEIGHT)){printf("X11 Error!\n");exit(0);}
stillused = TRUE;  /* Setting this variable to FALSE will terminate the application */
while (stillused == TRUE)
 {
  toberefreshed = FALSE;
  typedchar = get_xchar();
  if (typedchar == 27){stillused = FALSE;}
  if (clicked==TRUE)
     {
     if ((mousex>BUTTON1_X)&&(mousex<(BUTTON1_X+BUTWIDTH))&&(mousey>BUTTON1_Y)&&(mousey<(BUTTON1_Y+BUTHEIGHT)))
        {Graphics_shutdown();Prepare_tile ();stillused = FALSE;}
     if ((mousex>BUTTON2_X)&&(mousex<(BUTTON2_X+BUTWIDTH))&&(mousey>BUTTON2_Y)&&(mousey<(BUTTON2_Y+BUTHEIGHT)))
        {alternative_layout=1;Graphics_shutdown();Prepare_tile ();stillused = FALSE;}
     if ((mousex>BUTTON3_X)&&(mousex<(BUTTON3_X+BUTWIDTH))&&(mousey>BUTTON3_Y)&&(mousey<(BUTTON3_Y+BUTHEIGHT)))
        {tile_vert=1;tile_stripes=1;Prepare_tile ();stillused = FALSE;}
     if ((mousex>BUTTON4_X)&&(mousex<(BUTTON4_X+BUTWIDTH))&&(mousey>BUTTON4_Y)&&(mousey<(BUTTON4_Y+BUTHEIGHT)))
        {tile_stripes=1;Prepare_tile ();stillused = FALSE;}
	else {Graphics_shutdown();}
     clicked = FALSE;
     }
  if(toberefreshed == TRUE){refresh();}
  rightclicked = FALSE;
  }
Graphics_shutdown();
}

void trans ()
{
Sort_list();
Display *disp;
disp = XOpenDisplay(NULL);
double transparency = 0.5;	/* between 1.0 and 0.0 */
unsigned int opacity = (unsigned int) (0xffffffff * transparency);
Atom atom_window_opacity = XInternAtom(disp, "_NET_WM_WINDOW_OPACITY", FALSE);
/*for (i =1; i < winondesk+1; i++)
	{
     XChangeProperty(disp,winl[i],atom_window_opacity,XA_CARDINAL,32,PropModeReplace,(unsigned char *) &opacity,1);
	}
transparency = 0.2;*/
XChangeProperty(disp,activewin,atom_window_opacity,XA_CARDINAL,32,PropModeReplace,(unsigned char *) &opacity,1);
XFlush(disp);
XCloseDisplay(disp);
return;
}

void ReadDim ()
{
Display *disp;
disp = XOpenDisplay(NULL);
int x, y, junkx, junky;
unsigned int wwidth, wheight, bw, depth;
Window junkroot;

        if (!XGetGeometry (disp,activewin,&junkroot,&junkx,&junky,&wwidth,&wheight,&bw,&depth))
           { ;}
		else {
              XTranslateCoordinates (disp,activewin,junkroot,junkx,junky,&x,&y,&junkroot);
              winsnc[i][0]=activewin;
	          winsnc[i][1]=x;
              winsnc[i][2]=y;
	          winsnc[i][3]=wwidth;
              winsnc[i][4]=wheight;
	          }
	    
XCloseDisplay (disp);
}

void LinSnap ()
{
Get_info_desk ();
Get_info_win_desk ();
Display *disp;
disp = XOpenDisplay(NULL);
int winsn[20][6]={{0},};
int winsnb[20][6]={{0}}; 
for (i =1; i < 20; i++)
	   {winsn[i][5]=0;}
int x, y, junkx, junky,wait;
unsigned int wwidth, wheight, bw, depth;
Window junkroot;
for (i =1; i < winondesk+1; i++)
	   {
        if (!XGetGeometry (disp,winl[i],&junkroot,&junkx,&junky,&wwidth,&wheight,&bw,&depth))
           { ;}
		else {
              XTranslateCoordinates (disp,winl[i],junkroot,junkx,junky,&x,&y,&junkroot);
              winsnb[i][0]=winl[i];
	          winsnb[i][1]=x;
              winsnb[i][2]=y;
	          winsnb[i][3]=wwidth;
              winsnb[i][4]=wheight;
	          }
	    }       
	    XCloseDisplay (disp);
while (1)
  {usleep (50000);wait=0;
	disp = XOpenDisplay(NULL);
	Get_info_desk ();
	Get_info_win_desk ();
   for (i =1; i < winondesk+1; i++)
	   {
		if (!XGetGeometry (disp,winl[i],&junkroot,&junkx,&junky,&wwidth,&wheight,&bw,&depth)){ ;}
		else {XTranslateCoordinates (disp,winl[i],junkroot,junkx,junky,&x,&y,&junkroot);
        winsn[i][0]=winl[i];
	    winsn[i][1]=x;//printf ("winsnx %i        ",winsn[i][1]);
	    winsn[i][2]=y;//printf ("winsny %i\n",winsn[i][2]);
	    winsn[i][3]=wwidth;
        winsn[i][4]=wheight;}
	    
	    if ((winsn[i][2]<50) && (winsnb[i][2]>y)) 
		{
		 int ev, er, ma, mi;
         if(!XTestQueryExtension(disp, &ev, &er, &ma, &mi))
         {fprintf(stderr, "XTest extension not supported on server.\n");exit(1);}
         XTestFakeButtonEvent(disp, 1, True, CurrentTime);
         XTestFakeButtonEvent(disp, 1, False, CurrentTime);
         /*,***************************************************
         if (!XGetGeometry (disp,activewin,&junkroot,&junkx,&junky,&wwidth,&wheight,&bw,&depth))
           { ;}
		else {
              XTranslateCoordinates (disp,activewin,junkroot,junkx,junky,&x,&y,&junkroot);
              winsnc[i][0]=activewin;
	          winsnc[i][1]=x;
              winsnc[i][2]=y;
	          winsnc[i][3]=wwidth;
              winsnc[i][4]=wheight;
	          };
         ***************************************************/
         XMoveResizeWindow(disp,winsn[i][0],0,0,dskcw-dcw,dskch-dch);
         winsn[i][5]=1;wait=1; XFlush(disp);      
        }
		if ((winsn[i][1]<5) && (winsnb[i][1]>x)) 
		{
		 int ev, er, ma, mi;
         if(!XTestQueryExtension(disp, &ev, &er, &ma, &mi))
         {fprintf(stderr, "XTest extension not supported on server.\n");exit(1);}
         XTestFakeButtonEvent(disp, 1, True, CurrentTime);
         XTestFakeButtonEvent(disp, 1, False, CurrentTime);
         /*,***************************************************
         if (!XGetGeometry (disp,activewin,&junkroot,&junkx,&junky,&wwidth,&wheight,&bw,&depth))
           { ;}
		else {
              XTranslateCoordinates (disp,activewin,junkroot,junkx,junky,&x,&y,&junkroot);
              winsnc[i][0]=activewin;
	          winsnc[i][1]=x;
              winsnc[i][2]=y;
	          winsnc[i][3]=wwidth;
              winsnc[i][4]=wheight;
	          };
	     ************************************************************/
         XMoveResizeWindow(disp,winsn[i][0],0,0,(dskcw/2)-dcw,dskch-dch);
         winsn[i][5]=1;wait=1; XFlush(disp);
        }
        if ((winsn[i][1]+winsn[i][3]>dskww-5) && (winsnb[i][1]<x)) 
		{
		 int ev, er, ma, mi;
         if(!XTestQueryExtension(disp, &ev, &er, &ma, &mi))
         {fprintf(stderr, "XTest extension not supported on server.\n");exit(1);}
         XTestFakeButtonEvent(disp, 1, True, CurrentTime);
         XTestFakeButtonEvent(disp, 1, False, CurrentTime);
         /*,**************************************************
         if (!XGetGeometry (disp,activewin,&junkroot,&junkx,&junky,&wwidth,&wheight,&bw,&depth))
           { ;}
		else {
              XTranslateCoordinates (disp,activewin,junkroot,junkx,junky,&x,&y,&junkroot);
              winsnc[i][0]=activewin;
	          winsnc[i][1]=x;
              winsnc[i][2]=y;
	          winsnc[i][3]=wwidth;
              winsnc[i][4]=wheight;
	          };
         ***************************************************/
         XMoveResizeWindow(disp,winsn[i][0],(dskcw/2),0,(dskcw/2)-dcw,dskch-dch);
         winsn[i][5]=1;wait=1;
         XFlush(disp);
         }
        
        
   /*     if ((winsn[i][1]>15) && (winsnb[i][1]<x)&& (winsn[i][5]>0) && wait==0 )
		   {
		    winsn[i][5]=0;
		    for (i =1; i < winondesk+1; i++)
	            {
		        if (activewin==winsnc [i][0])
		           {
		            XTestFakeButtonEvent(disp, 1, True, CurrentTime);
         XTestFakeButtonEvent(disp, 1, False, CurrentTime);
                    XMoveResizeWindow(disp,winsnc[i][0],winsnc[i][1],winsnc[i][2],winsnc[i][3],winsnc[i][4]);
                    XFlush(disp);
                    }
                 };
            }
		if ((winsn[i][1]+winsn[i][3]<dskww-15) && (winsnb[i][1]>x)  && (winsn[i][5]>0) && wait==0)
		   {
		    winsn[i][5]=0;
		    for (i =1; i < winondesk+1; i++)
	            {
		        if (activewin==winsnc [i][0])
		           {
		            XTestFakeButtonEvent(disp, 1, True, CurrentTime);
         XTestFakeButtonEvent(disp, 1, False, CurrentTime);
                    XMoveResizeWindow(disp,winsnc[i][0],winsnc[i][1],winsnc[i][2],winsnc[i][3],winsnc[i][4]);
                    XFlush(disp);
                    }
                 };
            }
        if ((winsn[i][2]>70) && (winsnb[i][2]<y) && (winsn[i][5]>0) && wait==0) 
		   {
		    winsn[i][5]=0;
		    for (i =1; i < winondesk+1; i++)
	            {
		        if (activewin==winsnc [i][0])
		           {
		            XTestFakeButtonEvent(disp, 1, True, CurrentTime);
         XTestFakeButtonEvent(disp, 1, False, CurrentTime);
                    XMoveResizeWindow(disp,winsnc[i][0],winsnc[i][1],winsnc[i][2],winsnc[i][3],winsnc[i][4]);
                    XFlush(disp);
                    }
                 };
            }*/
        }
 for (i =1; i < winondesk+1; i++)
	   {
        if (!XGetGeometry (disp,winl[i],&junkroot,&junkx,&junky,&wwidth,&wheight,&bw,&depth))
           { ;}
		else {
              XTranslateCoordinates (disp,winl[i],junkroot,junkx,junky,&x,&y,&junkroot);
              winsnb[i][0]=winl[i];
	          winsnb[i][1]=x;
              winsnb[i][2]=y;
	          winsnb[i][3]=wwidth;
              winsnb[i][4]=wheight;
	          }
	    }       

  XCloseDisplay (disp);}
}

void Unsnap ()
{

		
}

void Place_sm_window_sugg ()
{
int ow=deskZx+0;
int oh=deskZy+0;
if (winondesk==0) {return;}
Sort_list();
activewin=winl[1];
Display *disp;
disp = XOpenDisplay(NULL);
Client_msg(disp, activewin, "_NET_ACTIVE_WINDOW",0,0,0,0,0);
Client_msg(disp, activewin, "_NET_WM_STATE", 
           XInternAtom(disp,"_NET_WM_STATE_REMOVE",False) , 
		   XInternAtom(disp,"_NET_WM_STATE_MAXIMIZED_VERT",False),
		   XInternAtom(disp,"_NET_WM_STATE_MAXIMIZED_HORZ",False),0,0);
XCloseDisplay(disp);
	Display *dpy;
	Window root;
	Window ret_root;
	Window ret_child;
	int root_x=10;
	int root_y=10;
	int win_x;
	int win_y;
	unsigned int mask;
 	dpy = XOpenDisplay(NULL);
	root = XDefaultRootWindow(dpy);
 dskwh=dskch;
 dskww=dskcw;
dskch=dskch-offset_bottom;oh=deskZy+offset_top;dskch=dskch-offset_top;
dskcw=dskcw-offset_right;ow=deskZx+offset_left;dskcw=dskcw-offset_left;
/*pointer feedback*/
int status;
Cursor cursor;
cursor = XCreateFontCursor(disp, XC_crosshair);
status = XGrabPointer(disp, root, False,ButtonPressMask|ButtonReleaseMask,
                      GrabModeSync,GrabModeAsync, root, cursor, CurrentTime);
  if (status != GrabSuccess) 
	{
     fputs("ERROR: Cannot grab mouse.\n", stderr);
     return ;
    }
XAllowEvents(disp, SyncPointer, CurrentTime);
/*pointer feedback end -set this when no longer needed- XUngrabPointer(disp, CurrentTime);*/

  if (screen_sec) {Print_screen_sections ();}
while (1)
  {
  
  if(XQueryPointer(dpy, root, &ret_root, &ret_child, &root_x, &root_y,
				   &win_x, &win_y, &mask))
	{
	if (mask & (Button1Mask)){if (screen_sec) {Print_screen_sections ();};return;}
	Display *disp;
	disp = XOpenDisplay(NULL);
	/*CenterVerticalB*/
	if ((root_y<root_x+ow) & (root_x<((dskww/2)+oh)))
	{XMoveResizeWindow(disp,activewin,0+ow,0+oh,(((dskcw/4)*3)-dcw),dskch);}
	if ((root_x>((dskww/16)*8)+ow) & (root_x<((dskww/16)*9)+ow)& (root_y>((dskwh/16)*7)+oh) & (root_y<((dskwh/16)*9)+oh))
	{XMoveResizeWindow(disp,activewin,(dskcw/4)+ow,0+oh,(((dskcw/4)*3)-dcw),dskch);}
	
	;XCloseDisplay(disp);};
  }
disp = XOpenDisplay(NULL);
  XMapRaised(disp, winl[1]);
  XCloseDisplay(disp);XUngrabPointer(disp, CurrentTime);return;
}
