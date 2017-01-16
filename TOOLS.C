typedef unsigned char far * block;
#define FALSE 0
#define TRUE 1

#include <stdio.h>
#include <io.h>
#include <process.h>
#include <dir.h>
#include <conio.h>
#include <stdlib.h>
#include <dos.h>
#include <fcntl.h>
#include <stdarg.h>
#include <time.h>
#include <alloc.h>
#include <math.h>
#include <mem.h>
#include <string.h>

void drawpixel(int x,int y,unsigned char color);
block loadimage(char *filename);
void loadfont(char *filename,int ft);
void vgaputch(int x,int y,unsigned char color,int ch);
void vgaprintf(int x,int y,unsigned char color,char *fmt,...);
void setfakepage(block hidpage);
void setvidmode(char mode);
void setvgapage(void);
void copypartpage(int l,int t,int r,int b);
void showimage(int x,int y,block buff);
block loadbc(char *filename);
void loadcol(char *filename,unsigned char palette[256][3]);
void clrcol(int start,int number);
void fadein(int start,int count,int speed,unsigned char palette[256][3]);
void fadeout(int start,int count,int speed,unsigned char palette[256][3]);
void drawbox(int l,int t,int r,int b,unsigned char color);
unsigned int getwidth(block buffer);
unsigned int getheight(block buffer);
void freeimage(block buffer);
void fnotfound(char *filename);
unsigned char getkey(int function);
void flushkey(void);
unsigned long gettimeticks(void);
void swap(int *first,int *second);
int getbit(int value,int bit);
unsigned long getfsize(FILE *fp);
block grabimage(int l,int t,int r,int b);
void wait(unsigned long delayticks);
void xrandomout(int x,int y,block buffer);
void cls(unsigned char color);
char *getinput(int x,int y,int color,int slength);

unsigned FAKEPAGE=0xA000;
unsigned char far *TEMPVGAMEM;
unsigned char far *VGAMEM=MK_FP(0xA000,0x00);

char fontdata[1][4096];
int fontsp,fonttype;
int chheight=11;

void drawpixel(int x,int y,unsigned char color)
{
    *(VGAMEM+(y*320+x))=color;
}

block loadimage(char *filename)
{
    FILE *fp;
    block buff;
    unsigned size;

    if (!(fp=fopen(filename,"rb")))
        fnotfound(filename);
    size=getfsize(fp);
    buff=farmalloc(size);
    fseek(fp,0L,SEEK_SET);
    fread(buff,1,size,fp);
    fclose(fp);
    return(buff);
}

void loadfont(char *filename,int ft)
{
    FILE *fp;

    if (!(fp=fopen(filename,"rb")))
        fnotfound(filename);
    fread(&fontdata[ft],1,4096,fp);
    fclose(fp);
}

void vgaputch(int x,int y,unsigned char color,int ch)
{
    int count0,count1,offset;

    TEMPVGAMEM=MK_FP(FAKEPAGE,y*320+x);
	offset=16*ch;

    for (count0=offset;count0<offset+16;count0++,TEMPVGAMEM+=320)
        for (x=0,count1=7;count1>=0;count1--,x++)
            if ((getbit(fontdata[fonttype][count0],count1))==1)
                *(TEMPVGAMEM+x)=color;
}

void vgaprintf(int x,int y,unsigned char color,char *fmt,...)
{
    va_list ap;
    char string[80];
    int count;

    va_start(ap,fmt);
    vsprintf(string,fmt,ap);
    va_end(ap);

    for (count=0;string[count]!='\0';count++)
    {
        vgaputch(x-1,y,0,string[count]);
        vgaputch(x+1,y,0,string[count]);
        vgaputch(x,y-1,0,string[count]);
        vgaputch(x,y+1,0,string[count]);
        vgaputch(x,y,color,string[count]);
        x+=fontsp;
    }
}

void setvidmode(char mode)
{
	asm mov ah,00;
    asm mov al,mode;
    asm int 10h;
}

void setfakepage(block hidpage)
{
    FAKEPAGE=FP_SEG(hidpage);
    VGAMEM=MK_FP(FAKEPAGE,0x00);
}

void setvgapage(void)
{
    FAKEPAGE=0xA000;
    VGAMEM=MK_FP(FAKEPAGE,0x00);
}

void copypartpage(int l,int t,int r,int b)
{
    unsigned char far *VISVGAMEM;
    int width,height;

    if (l>r)
        swap(&r,&l);
    if (t>b)
        swap(&t,&b);

    if (l<0)
        l=0;
    if (r>320)
        r=320;
    if (t<0)
        t=0;
    if (b>200)
        b=200;

    width=r-l+1;
    height=b-t+1;

    if (width>320||height>200)
        return;

    VISVGAMEM=MK_FP(0xA000,t*320+l);
    TEMPVGAMEM=MK_FP(FAKEPAGE,t*320+l);

    for (t=0;t++<height;VISVGAMEM+=320,TEMPVGAMEM+=320)
        _fmemcpy(VISVGAMEM,TEMPVGAMEM,width);
}

void showimage(int x,int y,block buff)
{
	int width=getwidth(buff);
	int height=getheight(buff);
    int dwidth=width,dheight=height;

    if (x>320||y>200)
        return;

    buff+=4;

    if (x<0)
        dwidth=width+x,buff+=0-x,x=0;
    else
        if ((x+width)>=320)
            dwidth=320-x;

    if (y<0)
        dheight=height+y,buff+=(0-y)*dwidth,y=0;
    else
        if ((y+height)>=200)
            dheight=200-y;

    if (dwidth<=0||dheight<=0)
        return;

    TEMPVGAMEM=MK_FP(FAKEPAGE,y*320+x);
    for (y=0;y<dheight;y++,buff+=width,TEMPVGAMEM+=320)
        _fmemcpy(TEMPVGAMEM,buff,dwidth);
}

block loadbc(char *filename)
{
    FILE *fp;
    int x,maxx,maxy;
    block buffer;
    block bc;
    long offset,total,count0,count1;
    unsigned size;

    if (!(fp=fopen(filename,"rb")))
        fnotfound(filename);
    size=getfsize(fp);           // Get the size of the file.
    bc=farmalloc(size);         // Locating memory for load the file.
    fseek(fp,0L,SEEK_SET);
    fread(bc,1,size,fp);        // Reads the file into the memory.
    fclose(fp);
    maxx=getwidth(bc);
    maxy=getheight(bc);

    // Locating memory to store the converted block file.
    buffer=farmalloc((unsigned)((maxx*maxy)+4));
    total=((long)maxx*(long)maxy+4);      // Calcualte total pixels
    for (x=0;x<4;x++)          // Copy width and height into the
        buffer[x]=bc[x];      // block from bc.

    count0=4;                  // Skip first four bytes.
    offset=4;
    while (offset<total)       // Decompressing data.
    {                          // run-length encoding explained in the
                               // report.
        if (bc[count0]>0xBF)
        {
            for (count1=0;count1<((long)bc[count0]-(long)0xBF);count1++)
                buffer[offset+count1]=bc[count0+1];
            count0+=1;
            offset=offset+count1-1;
        }
        else
            buffer[offset]=bc[count0];

        offset++;
        count0++;
    }
    farfree(bc);
    // After bc has been decompressed and sotre into buffer then return the
    // block.
    return (buffer);
}

void loadcol(char *filename,unsigned char palette[256][3])
{
	FILE *fp;
    int count;

    if (!(fp=fopen(filename,"rb")))
        fnotfound(filename);

    for (count=0;count<256;count++)
        fread(&palette[count],1,3,fp);

    fclose(fp);
}

void clrcol(int start,int number)
{
    unsigned int i,j;

    if (start>256||start<0||((start+number)>256))
		return;

    for (i=start;i<=(start+number);i++)
    {
		asm cli;
        outportb(0x3C8,i);
        for (j=0;j<3;j++)
            outportb(0x3C9,0);
		asm sti;
    }
}

void fadein(int start,int count,int speed,unsigned char palette[256][3])
{
    int i,j,cycle;
    unsigned char newpal[256][3];

    for (i=0;i<256;i++)
        for (j=0;j<3;j++)
            newpal[i][j]=0;

    for (cycle=0;cycle<64;cycle++)
    {
        for (i=start;i<=(start+count);i++)
        {
            asm cli;
            outportb(0x3C8,i);
            for (j=0;j<3;j++)
            {
                if (newpal[i][j]<palette[i][j])
                    newpal[i][j]++;
                outportb(0x3C9,newpal[i][j]);
            }
            asm sti;
        }
        delay(speed);
    }
}

void fadeout(int start,int count,int speed,unsigned char palette[256][3])
{
    int i,j,cycle;
    unsigned char newpal[256][3];

    for (i=0;i<256;i++)
        for (j=0;j<3;j++)
            newpal[i][j]=palette[i][j];

    for (cycle=0;cycle<64;cycle++)
    {
        for (i=start;i<=(start+count);i++)
        {
            asm cli;
            outportb(0x3C8,i);
            for (j=0;j<3;j++)
            {
                if (newpal[i][j]!=0)
                    newpal[i][j]--;
                outportb(0x3C9,newpal[i][j]);
            }
            asm sti;
        }
        delay(speed);
    }
}

void drawbox(int l,int t,int r,int b,unsigned char color)
{
    unsigned int width=r-l;

    TEMPVGAMEM=MK_FP(FAKEPAGE,t*320+l);
    _fmemset(TEMPVGAMEM,color,width+1),TEMPVGAMEM+=320,t+=1;
    while (t<b)
        *TEMPVGAMEM=color,*(TEMPVGAMEM+width)=color,t++,TEMPVGAMEM+=320;
    _fmemset(TEMPVGAMEM,color,width+1);
}

unsigned int getwidth(block buffer)
{
    return (buffer[0]+buffer[1]*256);
}

unsigned int getheight(block buffer)
{
    return (buffer[2]+buffer[3]*256);
}

void freeimage(block buffer)
{
    if (buffer!=NULL)
        farfree(buffer);
}

void fnotfound(char *filename)
{
    setvidmode(0x3);
    printf("Can't find %s\n",filename);
    exit(1);
}

unsigned char getkey(int function)
{
    char key;

    asm mov ah,10h;
    asm int 16h;

    if (function==0)            // get scan code
        asm mov key,ah;
    if (function==1)            // get ASCII code
        asm mov key,al;

    return (key);
}

void flushkey(void)
{
	asm mov ah,0Ch;
    asm int 21h;
}

unsigned long gettimeticks(void)
{
    long far *timeticks=MK_FP(0x40,0x6C);
    return (*timeticks);
}

void swap(int *first,int *second)
{
    int temp;

    temp=*first;
    *first=*second;
    *second=temp;
}

int getbit(int value,int bit)
{
    if (((value>>=bit)%2)==0)
	return(0);
    else
        return(1);
}

unsigned long getfsize(FILE *fp)
{
    unsigned long filesize;

    fseek(fp,0L,SEEK_END);            // Go to the end of the file
    filesize=ftell(fp);               // and find its position, which
    return (filesize);                // is the file size.
}

block grabimage(int l,int t,int r,int b)
{
    block buff;
    int width,height;
    unsigned int count;

    if (l>r)
        swap(&r,&l);
    if (t>b)
        swap(&t,&b);

    if (l<0)
        l=0;
    if (r>320)
        r=320;
    if (t<0)
        t=0;
    if (b>200)
        b=200;

    width=r-l+1;
    height=b-t+1;

    if (width>321||height>201)
        return(NULL);

    buff=farmalloc((unsigned)(width*height+4));

    if (width>256)
        count=256;
    else
        count=0;

    buff[0]=width-count;
    buff[1]=count>>8;
    buff[2]=height;
    buff[3]=0;

    TEMPVGAMEM=MK_FP(FAKEPAGE,t*320+l);
    for (t=0,count=4;t<height;t++,count+=width,TEMPVGAMEM+=320)
        _fmemcpy(buff+count,TEMPVGAMEM,width);

    return (buff);
}

void wait(unsigned long delayticks)
{
    long far *timer=MK_FP(0x40,0x6C);

    delayticks+=*timer;
    while ((*timer)<delayticks);
}

void xrandomout(int x,int y,block buffer)
{
    unsigned int width=getwidth(buffer);
    unsigned int height=getheight(buffer);
    unsigned int total=width*height,tempx=0,tempy=0;
    unsigned int count,ran,temptotal;
    unsigned int *temp;
    unsigned int *new;
    unsigned char pixel;

    if (total>32000)
        return;

    temp=malloc(total*2);
    new=malloc(total*2);

    for (count=0;count<total;count++)
        new[count]=count;

    temptotal=total;
    randomize();
    for (count=0;count<total;count++)
    {
        ran=random(temptotal);
        temp[count]=new[ran];
        new[ran]=new[temptotal-1];
        temptotal--;
    }
    free(new);

    for (count=0;count<total;count++)
    {
        tempx=temp[count]%width;
        tempy=(temp[count]-tempx)/width;
        pixel=buffer[temp[count]+4];
        if (pixel!=255)
            drawpixel(tempx+x,tempy+y,pixel);
        if (count%200==0)
            wait(1);
    }
    free(temp);
}

void cls(unsigned char color)
{
    _fmemset(VGAMEM,color,64000U);
}

char *getinput(int x,int y,int color,int slength)
{
	char *temp,key;
	int count,bottom=y+chheight;
    block BackGround[25];

    for (count=0;count<25;count++)
        BackGround[count]=grabimage(x-1+count*fontsp,y,x+count*fontsp+fontsp,bottom+3);
	count=0;
	temp=calloc(slength,1);
	while (1)
	{
		if (kbhit())
		{
            showimage(x-1,y,BackGround[count]);
			key=getkey(1);
			if (key==13&&count>0)
				break;
			if (count>0&&key==8)
                x-=fontsp,count--,showimage(x-1,y,BackGround[count]);
			if ((key>=32&&key<=126)&&count<slength)
			{
				temp[count]=key;
                vgaprintf(x,y,color,"%c",temp[count]);
				count++,x+=fontsp;
			}
		}
	}
	temp[count]='\0';

    for (count=0;count<25;count++)
        freeimage(BackGround[count]);
	return(temp);
}
