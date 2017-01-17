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
#include "tools.c"

void putbrick(void);
int rotatehit(void);
int hithoredge(int way);
int hitveredge(void);
void updateplayarea(void);
void newbrick(void);
void checkline(void);
void newlevel(void);
void dead(void);
void updatescore(void);
void border(void);
void title(void);
void highscore(void);

unsigned char palette[256][3];

char brick[7][4][3][2] =
{
    {
        {   {0,-1},{0,1},{1,1}      },
        {   {-1,0},{1,0},{1,-1}     },
        {   {-1,-1},{0,-1},{0,1}    },
        {   {-1,1},{-1,0},{1,0}     }
    },
    {
        {   {0,-1},{0,1},{-1,1}     },
        {   {-1,0},{1,0},{1,1}      },
        {   {1,-1},{0,-1},{0,1}     },
        {   {-1,-1},{-1,0},{1,0}    }
    },
    {
        {   {-1,0},{1,0},{0,1}      },
        {   {0,-1},{1,0},{0,1}      },
        {   {-1,0},{1,0},{0,-1}     },
        {   {0,-1},{-1,0},{0,1}     }
    },
    {
        {   {0,-1},{0,1},{0,2}      },
        {   {-1,0},{1,0},{2,0}      },
        {   {0,-2},{0,-1},{0,1}     },
        {   {-2,0},{-1,0},{1,0}     }
    },
    {
        {   {1,-1},{1,0},{0,1}      },
        {   {-1,-1},{0,-1},{1,0}    },
        {   {-1,0},{-1,1},{0,-1}    },
        {   {-1,0},{0,1},{1,1}      }
    },
    {
        {   {0,-1},{1,0},{1,1}      },
        {   {-1,0},{0,-1},{1,-1}    },
        {   {-1,-1},{-1,0},{0,1}    },
        {   {-1,1},{0,1},{1,0}      }
    },
    {
        {   {0,-1},{1,-1},{1,0}     },
        {   {-1,-1},{0,-1},{-1,0}   },
        {   {-1,0},{-1,1},{0,1}     },
        {   {1,0},{1,1},{0,1}       }
    }
};

struct data
{
    unsigned char down;
    unsigned char left;
    unsigned char right;
    unsigned char crotate;
    unsigned char arotate;
} keydata;

block square[4],background,parea,sarea;
block tempbuffer;
char playarea[20][10];
int x,y,realx,realy,type,direction,level=1,current,delayt;
unsigned long score=0,lines=0;

void main(int argc,char *argv[])
{
    FILE *fp;
    unsigned char key;
    int temp,quit=FALSE;
    long timeticks;
    char name[80];

    if (argc==2)
    {
        temp=(atoi(argv[1]));
        if (temp>1000&&temp<1009)
            level=temp-1000;
    }

    delayt=8-level;
    timeticks=gettimeticks()-delayt;
    tempbuffer=farmalloc(64000U);
    memset(&playarea[0][0],0,200U);

    for (temp=0;temp<4;temp++)
    {
        sprintf(name,"square%d.rlc",temp);
        square[temp]=loadbc(name);
    }
    loadfont("char.fnt",0);
    fonttype=0,fontsp=9;

    fp=fopen("KEY.CFG","rb");
    fread(&keydata,sizeof(keydata),1,fp);
    fclose(fp);
    title();
    loadcol("wltetris.col",palette);
    newlevel();
    updatescore();
    randomize();
    newbrick();

    while (!quit)
    {
        if (kbhit())
        {
            key=getkey(0);
            flushkey();

            if (key==0x01)
                quit=TRUE;

            if (key==keydata.crotate)
            {
                temp=direction;
                direction--;
                if (direction==-1)
                    direction=3;
                if (rotatehit())
                    direction=temp;
            }

            if (key==keydata.arotate)
            {
                temp=direction;
                direction++;
                if (direction==4)
                    direction=0;
                if (rotatehit())
                    direction=temp;
            }

            if (key==keydata.left)
            {
                if (!hithoredge(key))
                    x-=8,realx--;
            }

            if (key==keydata.right)
            {
                if (!hithoredge(key))
                    x+=8,realx++;
            }

            if (key==keydata.down)
            {
                if (!hitveredge())
                    y+=8,realy++;
            }
            putbrick();
        }
        if ((gettimeticks())>timeticks+delayt)
        {
            timeticks=gettimeticks();
            putbrick();
            if (!hitveredge())
                y+=8,realy++;
        }
    }
    setvidmode(0x03);
    for (temp=0;temp<4;temp++)
        freeimage(square[temp]);
    freeimage(background);
    freeimage(parea);
    freeimage(sarea);
    freeimage(tempbuffer);
}

void updatescore(void)
{
    showimage(3,15,sarea);

    vgaprintf(3,15,71,"SCORE:%ld",score);
    vgaprintf(3,27,130,"LINES:%ld",lines);
    vgaprintf(3,39,84,"LEVEL:%d",level);
    copypartpage(3,15,113,50);
}

void putbrick(void)
{
    int count,i,j;

    showimage(120,20,parea);
    for (i=0;i<20;i++)
        for (j=0;j<10;j++)
            if (playarea[i][j]!=0)
                showimage(j*8+120,i*8+20,square[playarea[i][j]-1]);//pixel(i,j+50,9);

    showimage(x,y,square[current-1]);
    for (count=0;count<3;count++)
        showimage(x+brick[type][direction][count][0]*8,y+brick[type][direction][count][1]*8,square[current-1]);

    copypartpage(120,20,199,179);
}

int hitveredge(void)
{
    int count,tempx,tempy,status=FALSE;

    if (playarea[realy+1][realx]!=0)
        status=TRUE;
    else
        for (count=0;count<3;count++)
        {
            tempx=brick[type][direction][count][0]+realx;
            tempy=brick[type][direction][count][1]+realy+1;
            if (tempy>19||(playarea[tempy][tempx]!=0&&tempy>=1))
            {
                status=TRUE;
                break;
            }
        }
    if (status==TRUE)
        updateplayarea(),checkline(),newbrick(),score+=(long)level*5,updatescore();
    return (status);
}

int rotatehit(void)
{
    int count,tempx,tempy,location,status=FALSE;

    for (count=0;count<3;count++)
    {
        tempx=brick[type][direction][count][0]+realx;
        tempy=brick[type][direction][count][1]+realy;
        location=playarea[tempy][tempx];
        if (tempx<0||tempx>9||location!=0)
        {
            status=TRUE;
            break;
        }
    }
    return (status);
}

int hithoredge(int way)
{
    int count,tempx,tempy,status=FALSE;

    for (count=0;count<3;count++)
    {
        tempx=brick[type][direction][count][0]+realx;
        tempy=brick[type][direction][count][1]+realy;
        if (way==keydata.left)
            if ((tempx-1)<0||playarea[tempy][tempx-1]!=0)
                status=TRUE;
        if (way==keydata.right)
            if ((tempx+1)>9||playarea[tempy][tempx+1]!=0)
                status=TRUE;
    }
    return (status);
}

void updateplayarea(void)
{
    int count,tempx,tempy;

    if (realy<=0)
        dead();
    else
    {
        playarea[realy][realx]=current;
        for (count=0;count<3;count++)
        {
            tempx=brick[type][direction][count][0]+realx;
            tempy=brick[type][direction][count][1]+realy;
            if (tempx>=0&&tempx<10&&tempy>=0&&tempy<20)
                playarea[tempy][tempx]=current;
        }
    }
}

void newbrick(void)
{
    x=144,y=20,realx=3,realy=0;
    type=random(7);
    current=random(4)+1;
    if (kbhit())
        flushkey();
}

void checkline(void)
{
    int i,j;

    for (i=realy-2;i<20;i++)
    {
        for (j=0;j<10;j++)
            if (playarea[i][j]==0)
                break;
        if (j==10)
        {
            memmove(&playarea[1][0],&playarea[0][0],10*i);
            score+=(long)(level*5+5*(i-realy+2)),lines++,updatescore();
            if ((lines==level*15+level)&&level<8)
                level++,newlevel();
        }
    }
}

void newlevel(void)
{
    char fname[13];

    delayt=8-level;
    sprintf(fname,"level%d.rlc",level);
    fadeout(0,255,20,palette);
    background=loadbc(fname);
    setvgapage();
    showimage(0,0,background);
    freeimage(background);
    freeimage(parea);
    freeimage(sarea);
    parea=grabimage(120,20,199,179);
    sarea=grabimage(3,15,113,50);
    border();
    fadein(0,255,20,palette);
    setfakepage(tempbuffer);
}

void dead(void)
{
    int count;

    char *info={
"\nÉÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ»"
"\nº -=WildLife Tetris=- º"
"\nº     Version 1.0     º"
"\nº     By Felix Wu     º"
"\nº     June 5,1993     º"
"\nÈÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ¼"
"\nFREE WARE"};

    setvgapage();

    for (count=0;count<4;count++)
        freeimage(square[count]);
    freeimage(background);
    freeimage(parea);
    freeimage(sarea);
    freeimage(tempbuffer);
    highscore();
    setvidmode(0x3);
    printf("%s",info);
    exit(1);
}

void highscore(void)
{
    struct high
    {
        char name[20];
        unsigned long score;
    };

    struct high top10[10],temp[10];
    FILE *fp;
    int count,order,num,you=10,colour;
    char *name;
    block bk;

    if ((fp=fopen("HIGH","rb"))==NULL)
        for (count=0;count<10;count++)
        {
            strcpy(top10[count].name,"\0");
            top10[count].score=0;
        }
    else
        fread(&top10,sizeof(top10[0]),10,fp);

    for (count=0;count<10;count++)
        if (score>top10[count].score)
            break;

    if (count!=10)
    {
        vgaprintf(70,85,68,"Please Enter Your Name:");
        fontsp++;
        name=getinput(70,100,15,20);
        for (count=0;count<9;count++)
        {
            strcpy(temp[count].name,top10[count].name);
            temp[count].score=top10[count].score;
        }

        strcpy(top10[9].name,name);
        strcpy(temp[9].name,top10[9].name);
        temp[9].score=top10[9].score=score;

        for (count=0;count<10;count++)
        {
            for (num=0,order=0;num<10;num++)
                if (num!=count)
                    if (temp[num].score>=temp[count].score)
                        order++;
            if (count==9)
                you=order;
            strcpy(top10[order].name,temp[count].name);
            top10[order].score=temp[count].score;
        }

        fclose(fp);
        fp=fopen("HIGH","wb");
        fwrite(&top10,sizeof(top10[0]),10,fp);
    }
    fclose(fp);

    fadeout(0,255,20,palette);
    loadcol("title.col",palette);
    bk=loadimage("title.1");
    clrcol(0,255);
    showimage(0,0,bk);
    freeimage(bk);
    fontsp=9;
    for (count=0;count<10;count++)
    {
        if (count==you)
            colour=15;
        else
            colour=118;
        vgaprintf(20,count*12+30,colour,"%2d.%20s %8ld",count+1,top10[count].name,top10[count].score);
    }
    fadein(0,255,20,palette);
    getch();
    flushkey();
    fadeout(0,255,20,palette);
}

void border(void)
{
    int colour=26;
    int t=119,l=19,r=200,b=180;

    while (colour>20)
        drawbox(t--,l--,r++,b++,colour-=2);
    while (colour<26)
        drawbox(t--,l--,r++,b++,colour+=2);
}

void title(void)
{
    block titlebk,title,name;

    titlebk=loadimage("title.1");
    title=loadbc("title.2");
    name=loadbc("title.3");
    loadcol("title.col",palette);
    setvidmode(0x13);
    clrcol(0,255);
    showimage(0,0,titlebk);
    freeimage(titlebk);
    fadein(0,255,20,palette);
    xrandomout(104,30,title);
    freeimage(title);
    xrandomout(92,160,name);
    freeimage(name);
    getch();
    fadeout(0,255,20,palette);
    cls(0);
}
