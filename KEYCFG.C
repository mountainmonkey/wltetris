#include <stdio.h>
#include <dos.h>

unsigned char getkey(void);

void main(void)
{
    struct data
    {
        unsigned char down;
        unsigned char left;
        unsigned char right;
        unsigned char crotate;
        unsigned char arotate;
    };

    char *info={
                    "\nษออออออออออออออออออออออออออออป"
                    "\nบ    -= WildLife Tetris =-   บ"
                    "\nบ Keys Configuration Program บ"
                    "\nบ        By Felix Wu         บ"
                    "\nบ        June 5,1993         บ"
                    "\nศออออออออออออออออออออออออออออผ\n"
                };
    struct data keydata;
    FILE *fp;

    fp=fopen("KEY.CFG","wb");
    printf("%s",info);
    printf("DOWN: ");
    keydata.down=getkey();
    printf("þ\nLEFT: ");
    keydata.left=getkey();
    printf("þ\nRIGHT: ");
    keydata.right=getkey();
    printf("þ\nCLOCKWISE ROTATE: ");
    keydata.crotate=getkey();
    printf("þ\nCOUNTER-COLCKWISE ROTATE: ");
    keydata.arotate=getkey();
    printf("þ\nDone.\n");
    fwrite(&keydata,sizeof(keydata),1,fp);
    fclose(fp);
}

unsigned char getkey(void)
{
    char key;

    asm mov ah,10h;
    asm int 16h;
    asm mov key,ah;

    return (key);
}
