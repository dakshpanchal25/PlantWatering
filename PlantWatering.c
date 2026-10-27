#include <stdio.h> 

#include <conio.h> 

#include <graphics.h> 

#include <dos.h> 

#include <math.h> 

#include <stdlib.h> 

 

enum {C_BLACK,C_BLUE,C_GREEN,C_CYAN,C_RED,C_MAGENTA,C_BROWN,C_LIGHTGRAY, 

      C_DARKGRAY,C_LIGHTBLUE,C_LIGHTGREEN,C_LIGHTCYAN,C_LIGHTRED, 

      C_LIGHTMAGENTA,C_YELLOW,C_WHITE}; 

 

#define FPS 10 

#define SECS 60 

#define SPEED 2 

#define FRAMES (FPS*SECS) 

 

#define DAY_START 360 

#define DAY_END 1320 

#define STEP 30 

#define THRESH 40 

#define INIT_MOIST 46 

#define BOOST 40 

 

int night,theme,oldW,happyNow,pumpState; 

int moisture,minuteNow,cycles,wLog[8]; 

int asmMoisture,asmQuot,asmRem; 

int rate[]={1,3,1}; 

char *pn[]={"MORNING","DAYTIME","EVENING"}; 

void *dropBg,*plantBg; 

 

int sx[]={30,150,215,290,345,390,455,560,610,250,180,320,520,600,120}; 

int sy[]={90,110,80,130,95,170,120,100,160,190,215,225,210,230,180}; 

 

void pumpON() { 

  pumpState=1; 

  asm mov dx,300h 

  asm mov al,01h 

  asm out dx,al 

} 

 

void pumpOFF() { 

  pumpState=0; 

  asm mov dx,300h 

  asm mov al,00h 

  asm out dx,al 

} 

 

void calculateMoistureAsm(int mFrom,int mTo,int elapsed) { 

  asm mov ax,mTo 

  asm sub ax,mFrom 

  asm mov bx,elapsed 

  asm mul bx 

  asm mov bx,FRAMES 

  asm xor dx,dx 

  asm div bx 

  asm mov bx,mFrom 

  asm add ax,bx 

  asm mov asmMoisture,ax 

} 

 

void calculateTimeAsm(int total) { 

  asm mov ax,total 

  asm xor dx,dx 

  asm mov bx,60 

  asm div bx 

  asm mov asmQuot,ax 

  asm mov asmRem,dx 

} 

 

int periodOf(int m) { return m<720?0:(m<1020?1:2); } 

 

int themeOf(int m) { return m<720?0:(m<1020?1:(m<1170?2:3)); } 

 

void box(int x1,int y1,int x2,int y2,int c) { 

  setfillstyle(SOLID_FILL,c); 

  bar(x1,y1,x2,y2); 

} 

 

void ell(int x,int y,int rx,int ry,int c) { 

  setcolor(c); 

  setfillstyle(SOLID_FILL,c); 

  fillellipse(x,y,rx,ry); 

} 

 

void ln(int x1,int y1,int x2,int y2,int c) { 

  setcolor(c); 

  line(x1,y1,x2,y2); 

} 

 

void txt(int x,int y,char *s,int c) { 

  setcolor(c); 

  outtextxy(x,y,s); 

} 

 

void font(int n) { settextstyle(DEFAULT_FONT,HORIZ_DIR,n); } 

 

void thick(int on) { setlinestyle(SOLID_LINE,0,on?THICK_WIDTH:NORM_WIDTH); } 

 

void ctext(int y,char *s,int col,int sc) { 

  int x=(640-textwidth(s))/2; 

  if(sc>=0) 

    txt(x+2,y+2,s,sc); 

  txt(x,y,s,col); 

} 

 

void *grab(int x1,int y1,int x2,int y2) { 

  void *m=malloc(imagesize(x1,y1,x2,y2)); 

  if(m) 

    getimage(x1,y1,x2,y2,m); 

  return m; 

} 

 

void dither(int y1,int y2,int col,int mode) { 

  int x,y; 

  for(y=y1;y<y2;y++) 

    for(x=0;x<640;x++) { 

      if(mode==1 && (x&1) && (y&1)) continue; 

      if(mode==2 && ((x+y)&1)) continue; 

      if(mode==3 && ((x&1) || (y&1))) continue; 

      putpixel(x,y,col); 

    } 

} 

 

void grad(int y,int col) { 

  dither(y,y+12,col,3); 

  dither(y+12,y+24,col,2); 

  dither(y+24,y+36,col,1); 

  box(0,y+36,639,400,col); 

} 

 

void drawSky() { 

  if(theme==3) { 

    box(0,0,639,479,C_BLUE); 

    box(0,62,639,82,C_BLACK); 

    dither(82,90,C_BLACK,1); 

    dither(90,98,C_BLACK,2); 

    dither(98,106,C_BLACK,3); 

    grad(255,C_LIGHTBLUE); 

  } else if(theme==2) { 

    box(0,0,639,479,C_MAGENTA); 

    grad(150,C_LIGHTRED); 

    grad(250,C_YELLOW); 

  } else { 

    box(0,0,639,479,C_LIGHTBLUE); 

    grad(240,C_LIGHTCYAN); 

  } 

} 

 

void drawSun(int cx,int cy,int c1,int c2) { 

  int i,r; 

  double a; 

  thick(1); 

  for(i=0;i<16;i++) { 

    a=i*0.3927; 

    r=(i%2)?44:54; 

    ln(cx+(int)(34*cos(a)),cy+(int)(34*sin(a)), 

       cx+(int)(r*cos(a)),cy+(int)(r*sin(a)),C_YELLOW); 

  } 

  thick(0); 

  ell(cx,cy,27,27,c1); 

  ell(cx,cy,13,13,c2); 

} 

 

void drawMoon() { 

  ell(85,145,40,40,C_LIGHTBLUE); 

  ell(85,145,28,28,C_WHITE); 

  ell(66,148,3,3,C_LIGHTGRAY); ell(70,162,2,2,C_LIGHTGRAY); 

  ell(63,136,2,2,C_LIGHTGRAY); 

  ell(97,135,24,24,C_LIGHTBLUE); 

} 

 

void drawStars() { 

  int i,d; 

  for(i=0;i<15;i++) { 

    putpixel(sx[i],sy[i],C_WHITE); 

    if(i%3==0) 

      for(d=-1;d<=1;d+=2) { 

        putpixel(sx[i]+d,sy[i],C_WHITE); 

        putpixel(sx[i],sy[i]+d,C_WHITE); 

      } 

  } 

} 

 

void cloud(int x,int y,int c,int sh) { 

  int k,d; 

  for(k=0;k<2;k++) { 

    d=k?0:4; 

    ell(x,y+d,28,13,k?c:sh); 

    ell(x+26,y-10+d,26,18,k?c:sh); 

    ell(x+54,y+d,28,13,k?c:sh); 

    box(x-10,y+d,x+64,y+13+d,k?c:sh); 

  } 

} 

 

void drawHills() { 

  int x; 

  for(x=0;x<640;x++) { 

    ln(x,320+(int)(26*sin(x/95.0+2)+8*sin(x/31.0)),x,390,night?C_BLACK:C_CYAN); 

    ln(x,348+(int)(18*sin(x/60.0)+7*sin(x/17.0+1)),x,390,night?C_GREEN:C_LIGHTGREEN); 

  } 

} 

 

void tree(int x) { 

  int s=(theme>=2); 

  box(x-4,355,x+4,385,s?C_BLACK:C_BROWN); 

  ell(x,332,28,34,s?C_BLACK:C_GREEN); 

} 

 

void drawGround() { 

  int x,c=night?C_GREEN:C_LIGHTGREEN; 

  box(0,385,639,479,night?C_BLACK:C_GREEN); 

  for(x=7;x<640;x+=17) { 

    ln(x,392,x-4,383,c); 

    ln(x,392,x,380,c); 

    ln(x,392,x+4,383,c); 

  } 

  if(night) 

    for(x=0;x<8;x++) 

      box(40+x*76,392+(x*13)%28,41+x*76,393+(x*13)%28,C_YELLOW); 

} 

 

void tankLevel(int px) { 

  int top=250+px; 

  box(40,240,120,top,C_BLACK); 

  box(40,top,120,360,C_CYAN); 

  ln(40,top,120,top,C_LIGHTCYAN); 

  ln(47,258,47,330,C_WHITE); 

} 

 

void drawTank() { 

  box(45,365,55,385,C_DARKGRAY); box(105,365,115,385,C_DARKGRAY); 

  box(32,229,128,236,C_DARKGRAY); 

  box(35,235,125,365,C_LIGHTGRAY); 

  font(1); 

  txt(40,216,"WATER TANK",C_WHITE); 

} 

 

void drawPump() { 

  box(150,280,235,365,C_DARKGRAY); 

  box(145,365,240,385,C_DARKGRAY); 

  box(145,270,240,282,C_LIGHTGRAY); 

  ell(158,291,2,2,C_LIGHTGRAY); ell(227,291,2,2,C_LIGHTGRAY); 

  ell(158,355,2,2,C_LIGHTGRAY); ell(227,355,2,2,C_LIGHTGRAY); 

  font(1); 

  txt(176,355,"PUMP",C_WHITE); 

} 

 

void gauge(int on,int f) { 

  int cx=192,cy=325; 

  double a; 

  ell(cx,cy,21,21,C_BLACK); 

  setcolor(on?C_LIGHTGREEN:C_LIGHTRED); 

  thick(1); 

  circle(cx,cy,24); 

  thick(0); 

  a=on?f*0.5:3.6; 

  ln(cx,cy,cx+(int)(15*cos(a)),cy+(int)(15*sin(a)),on?C_LIGHTCYAN:C_LIGHTGRAY); 

  ell(cx,cy,2,2,C_WHITE); 

} 

 

void pipeFill(int c) { 

  box(125,326,150,334,c); 

  box(188,186,196,264,c); 

  box(188,186,512,194,c); 

  box(504,186,512,204,c); 

} 

 

void drawPipes() { 

  box(125,322,150,338,C_DARKGRAY); 

  box(184,182,200,272,C_DARKGRAY); 

  box(184,182,516,198,C_DARKGRAY); 

  box(500,182,516,206,C_DARKGRAY); 

  box(180,266,204,274,C_LIGHTGRAY); 

  box(495,200,521,208,C_LIGHTGRAY); 

  box(503,208,513,212,C_LIGHTGRAY); 

  pipeFill(C_LIGHTGRAY); 

} 

 

void flow(int f) { 

  int o=(f*5)%40,x,y; 

  pipeFill(C_CYAN); 

  for(x=190+o;x<=500;x+=40) 

    box(x,188,x+12,192,C_LIGHTCYAN); 

  for(y=262-o;y>=200;y-=40) 

    box(190,y-12,194,y,C_LIGHTCYAN); 

} 

 

void drawPot() { 

  int p[8]={415,336,525,336,510,400,430,400}; 

  setcolor(C_BROWN); 

  setfillstyle(SOLID_FILL,C_BROWN); 

  fillpoly(4,p); 

  ln(424,355,516,355,C_YELLOW); 

  ln(425,358,515,358,C_YELLOW); 

  box(408,326,532,338,C_LIGHTRED); 

  ell(470,327,50,5,C_BLACK); 

  font(1); 

  txt(447,406,"PLANT",night?C_WHITE:C_BLACK); 

} 

 

void plant(int happy) { 

  int i,hx=470,hy=238,r=15,p=11; 

  double a; 

  if(plantBg) 

    putimage(420,205,plantBg,COPY_PUT); 

  thick(1); 

  if(happy) 

    ln(470,328,470,246,C_GREEN); 

  else { 

    hx=452; hy=276; r=11; p=8; 

    ln(470,328,470,298,C_GREEN); ln(470,298,456,284,C_GREEN); 

    ln(470,312,455,300,C_GREEN); ln(470,308,484,298,C_GREEN); 

  } 

  thick(0); 

  if(happy) { 

    ell(446,292,24,9,C_LIGHTGREEN); 

    ell(448,314,21,8,C_LIGHTGREEN); 

    ell(473,276,16,8,C_GREEN); 

    ell(474,306,15,7,C_GREEN); 

  } else { 

    ell(452,314,6,14,C_BROWN); 

    ell(486,310,6,13,C_BROWN); 

  } 

  for(i=0;i<6;i++) { 

    a=i*1.0472; 

    ell(hx+(int)(r*cos(a)),hy+(int)(r*sin(a)),p,p,happy?C_LIGHTMAGENTA:C_MAGENTA); 

  } 

  if(happy) { 

    ell(hx,hy,9,9,C_YELLOW); 

    ell(hx,hy,4,4,C_BROWN); 

  } else 

    ell(hx,hy,7,7,C_BROWN); 

} 

 

void setPlant() { 

  int h=(moisture>=THRESH); 

  if(h!=happyNow) { 

    happyNow=h; 

    plant(h); 

  } 

} 

 

void drawDrops(int f) { 

  int i,y; 

  if(dropBg) 

    putimage(499,214,dropBg,COPY_PUT); 

  for(i=0;i<4;i++) { 

    y=220+((f*2+i*30)%110); 

    ell(508,y,3,5,C_CYAN); 

    if(y>=318) { 

      setcolor(C_LIGHTCYAN); 

      ellipse(508,327,0,360,7,2); 

    } 

  } 

} 

 

void drawSensor() { 

  char t[10]; 

  int i,y,ty=336-THRESH*98/100; 

  box(578,350,586,392,C_LIGHTGRAY); 

  box(545,200,620,352,C_DARKGRAY); 

  setcolor(C_LIGHTCYAN); 

  rectangle(545,200,620,352); 

  font(1); 

  txt(567,208,"SOIL",C_WHITE); 

  txt(559,220,"SENSOR",C_WHITE); 

  rectangle(555,235,581,337); 

  for(i=25;i<100;i+=25) { 

    y=336-i*98/100; 

    ln(581,y,583,y,C_LIGHTGRAY); 

  } 

  sprintf(t,"%d%%",THRESH); 

  txt(588,ty-5,"MIN",C_LIGHTRED); 

  txt(588,ty+5,t,C_LIGHTRED); 

} 

 

void meter(int pct) { 

  char t[10]; 

  int h=pct*98/100,ty=336-THRESH*98/100; 

  int c=pct<THRESH?C_LIGHTRED:(pct<THRESH+20?C_YELLOW:C_LIGHTCYAN); 

  box(556,236,580,336,C_BLACK); 

  if(h>0) 

    box(556,336-h,580,336,c); 

  ln(553,ty,583,ty,C_LIGHTRED); 

  box(585,254,617,290,C_DARKGRAY); 

  sprintf(t,"%d%%",pct); 

  txt(588,258,t,c); 

  txt(588,272,pct<THRESH?"LOW":"OK",c); 

} 

 

void drawTitle() { 

  box(0,0,639,61,night?C_BLACK:C_BLUE); 

  ln(0,62,639,62,night?C_CYAN:C_YELLOW); ln(0,63,639,63,night?C_CYAN:C_YELLOW); 

  font(2); 

  ctext(10,"AUTOMATIC PLANT WATERING SYSTEM",C_WHITE,night?C_BLUE:C_BLACK); 

  font(1); 

  ctext(40,"8086 DRIP IRRIGATION   |   SOIL MOISTURE DAY CYCLE",C_LIGHTCYAN,-1); 

} 

 

void drawPanel(int on) { 

  int c=on?C_LIGHTGREEN:C_LIGHTRED; 

  box(405,75,625,165,C_DARKGRAY); 

  setcolor(C_LIGHTCYAN); 

  rectangle(405,75,625,165); rectangle(408,78,622,162); 

  font(1); 

  txt(420,84,"8086 CONTROL UNIT",C_LIGHTCYAN); 

  ln(420,97,610,97,C_LIGHTGRAY); ln(420,142,610,142,C_LIGHTGRAY); 

  txt(420,104,"OUT 300H,AL",C_WHITE); 

  txt(420,117,on?"AL = 01H":"AL = 00H",C_WHITE); 

  txt(420,130,on?"PUMP : ON":"PUMP : OFF",c); 

  ell(598,134,6,6,c); 

  txt(560,148,"SIM 2X",C_YELLOW); 

} 

 

void clockText() { 

  char t[30]; 

  calculateTimeAsm(minuteNow); 

  box(415,146,552,160,C_DARKGRAY); 

  sprintf(t,"TIME  %02d:%02d",asmQuot,asmRem); 

  txt(420,148,t,C_LIGHTCYAN); 

} 

 

void drawStatusBox() { 

  box(15,422,625,476,C_DARKGRAY); 

  setcolor(C_LIGHTGRAY); 

  rectangle(15,422,625,476); 

  setcolor(C_WHITE); 

  rectangle(29,447,611,463); 

  box(30,448,610,462,C_BLACK); 

} 

 

void statusBar(int mode,int left) { 

  char t[40]; 

  box(20,427,620,442,C_DARKGRAY); 

  font(1); 

  sprintf(t,"PERIOD : %s",pn[periodOf(minuteNow)]); 

  txt(28,431,t,C_YELLOW); 

  if(mode==1) 

    ctext(431,"WATERING IN PROGRESS",C_LIGHTCYAN,-1); 

  else if(mode==2) 

    ctext(431,"WATERING COMPLETED",C_LIGHTGREEN,-1); 

  else if(mode==3) 

    ctext(431,"SOIL LOW - STARTING PUMP",C_LIGHTRED,-1); 

  else if(moisture>=THRESH) 

    ctext(431,"SOIL MOISTURE: ADEQUATE",C_LIGHTGREEN,-1); 

  else 

    ctext(431,"SOIL MOISTURE: LOW",C_LIGHTRED,-1); 

  if(mode==1) 

    sprintf(t,"COUNTDOWN : %02d SEC",left); 

  else 

    sprintf(t,"CYCLES : %d",cycles); 

  txt(612-textwidth(t),431,t,C_WHITE); 

} 

 

void dayBar(int full) { 

  int w=(int)((long)(minuteNow-DAY_START)*580/(DAY_END-DAY_START)); 

  if(full) 

    box(30,448,610,462,C_BLACK); 

  if(w>0) { 

    box(30,448,30+w,462,C_YELLOW); 

    box(30,449,30+w,451,C_WHITE); 

  } 

} 

 

void progress(int f) { 

  int w=580-(int)((long)f*580/FRAMES); 

  if(w<oldW) { 

    box(30+w,448,610,462,C_BLACK); 

    oldW=w; 

  } 

} 

 

void drawScene() { 

  night=(theme==3); 

  free(dropBg); 

  free(plantBg); 

  drawSky(); 

  if(night) { 

    drawStars(); 

    drawMoon(); 

  } else if(theme==2) { 

    drawSun(300,285,C_LIGHTRED,C_YELLOW); 

    cloud(120,110,C_LIGHTMAGENTA,C_LIGHTRED); 

    cloud(290,150,C_LIGHTMAGENTA,C_LIGHTRED); 

  } else { 

    drawSun(theme?320:85,theme?120:150,C_YELLOW,C_WHITE); 

    cloud(theme?100:250,theme?100:110,C_WHITE,C_LIGHTGRAY); 

  } 

  drawHills(); 

  tree(290); 

  tree(355); 

  drawGround(); 

  drawTank(); 

  drawPump(); 

  drawPipes(); 

  drawSensor(); 

  drawPot(); 

  plantBg=grab(420,205,498,325); 

  happyNow=(moisture>=THRESH); 

  plant(happyNow); 

  dropBg=grab(499,214,525,336); 

  drawTitle(); 

  drawPanel(pumpState); 

  clockText(); 

  drawStatusBox(); 

  gauge(0,0); 

  tankLevel(cycles*10); 

  meter(moisture); 

  dayBar(1); 

  statusBar(0,0); 

} 

 

void watering() { 

  int f,m0=moisture,m1=moisture+BOOST; 

  if(m1>100) 

    m1=100; 

  statusBar(3,0); 

  delay(1500); 

  pumpON(); 

  drawPanel(1); 

  clockText(); 

  oldW=580; 

  box(30,448,610,462,C_CYAN); 

  box(30,449,610,451,C_LIGHTCYAN); 

  for(f=0;f<FRAMES;f++) { 

    flow(f); 

    drawDrops(f); 

    gauge(1,f); 

    progress(f); 

    calculateMoistureAsm(m0,m1,f); 

    if(asmMoisture!=moisture) { 

      moisture=asmMoisture; 

      meter(moisture); 

      setPlant(); 

    } 

    if(f%FPS==0) { 

      statusBar(1,SECS-f/FPS); 

      tankLevel(cycles*10+f*10/FRAMES); 

    } 

    delay(1000/FPS/SPEED); 

  } 

  pumpOFF(); 

  pipeFill(C_LIGHTGRAY); 

  if(dropBg) 

    putimage(499,214,dropBg,COPY_PUT); 

  gauge(0,0); 

  drawPanel(0); 

  clockText(); 

  if(cycles<8) 

    wLog[cycles]=minuteNow; 

  cycles++; 

  tankLevel(cycles*10); 

  moisture=m1; 

  meter(moisture); 

  setPlant(); 

  statusBar(2,0); 

  delay(1500); 

  dayBar(1); 

} 

 

void advance() { 

  int i; 

  for(i=0;i<STEP/5;i++) { 

    minuteNow+=5; 

    clockText(); 

    dayBar(0); 

    delay(70); 

  } 

} 

 

void dayCycle() { 

  minuteNow=DAY_START; 

  moisture=INIT_MOIST; 

  cycles=0; 

  theme=themeOf(minuteNow); 

  drawScene(); 

  delay(1200); 

  while(minuteNow<DAY_END) { 

    if(themeOf(minuteNow)!=theme) { 

      theme=themeOf(minuteNow); 

      drawScene(); 

    } 

    moisture-=rate[periodOf(minuteNow)]; 

    if(moisture<0) 

      moisture=0; 

    meter(moisture); 

    setPlant(); 

    if(moisture>=THRESH) { 

      pumpOFF(); 

      statusBar(0,0); 

    } else 

      watering(); 

    advance(); 

  } 

  pumpOFF(); 

} 

 

void intro() { 

  int i,p[6]={320,45,303,85,337,85}; 

  box(0,0,639,479,C_BLUE); 

  setcolor(C_LIGHTCYAN); 

  rectangle(20,20,619,459); rectangle(24,24,615,455); 

  setcolor(C_CYAN); 

  setfillstyle(SOLID_FILL,C_CYAN); 

  fillpoly(3,p); 

  ell(320,92,17,17,C_CYAN); 

  ell(312,90,3,6,C_WHITE); 

  font(4); 

  ctext(150,"PLANT WATERING",C_WHITE,C_BLACK); 

  ctext(192,"SYSTEM",C_YELLOW,C_BLACK); 

  font(1); 

  ctext(255,"8086 MICROPROCESSOR  |  DRIP IRRIGATION  |  FULL DAY CYCLE",C_LIGHTCYAN,-1); 

  ctext(275,"SOIL MOISTURE MONITORING  |  AUTOMATIC PUMP CONTROL",C_LIGHTGRAY,-1); 

  setcolor(C_WHITE); 

  rectangle(170,320,470,338); 

  for(i=0;i<=294;i+=3) { 

    box(173,323,173+i,335,C_LIGHTGREEN); 

    delay(20); 

  } 

  ctext(350,"SYSTEM READY",C_LIGHTGREEN,-1); 

  delay(1000); 

} 

 

void tick(int x,int y) { 

  ell(x,y,11,11,C_GREEN); 

  setcolor(C_LIGHTGREEN); 

  circle(x,y,11); 

  thick(1); 

  ln(x-5,y,x-1,y+4,C_WHITE); ln(x-1,y+4,x+6,y-5,C_WHITE); 

  thick(0); 

} 

 

void summary() { 

  char t[50]; 

  int i; 

  box(0,0,639,479,C_BLUE); 

  box(110,110,530,370,C_DARKGRAY); 

  setcolor(C_LIGHTCYAN); 

  rectangle(110,110,530,370); rectangle(114,114,526,366); 

  font(2); 

  ctext(132,"DAY CYCLE COMPLETE",C_LIGHTGREEN,C_BLACK); 

  ln(130,165,510,165,C_LIGHTGRAY); 

  font(1); 

  tick(150,190); 

  sprintf(t,"FINAL SOIL MOISTURE   : %d%%",moisture); 

  txt(175,186,t,C_WHITE); 

  tick(150,218); 

  sprintf(t,"TOTAL WATERING CYCLES : %d",cycles); 

  txt(175,214,t,C_WHITE); 

  tick(150,246); 

  sprintf(t,"FINAL PUMP STATUS     : %s",pumpState?"ON":"OFF"); 

  txt(175,242,t,C_WHITE); 

  ln(130,268,510,268,C_LIGHTGRAY); 

  for(i=0;i<cycles && i<4;i++) { 

    calculateTimeAsm(wLog[i]); 

    sprintf(t,"WATERING %d STARTED AT %02d:%02d",i+1,asmQuot,asmRem); 

    txt(175,278+i*14,t,C_LIGHTCYAN); 

  } 

  ctext(342,"PLANT WATERING REQUIREMENT SATISFIED",C_YELLOW,-1); 

  ctext(430,"PRESS ANY KEY TO EXIT",C_LIGHTGRAY,-1); 

} 

 

void main() { 

  int gd=DETECT,gm; 

  initgraph(&gd,&gm,"C:\\TURBOC3\\BGI"); 

  if(graphresult()!=grOk) { 

    printf("Graphics initialization failed."); 

    getch(); 

    return; 

  } 

  intro(); 

  pumpOFF(); 

  dayCycle(); 

  summary(); 

  getch(); 

  closegraph(); 

} 