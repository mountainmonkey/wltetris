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
                    "\n浜様様様様様様様様様様様様様融"
                    "\n�    -= WildLife Tetris =-   �"
                    "\n� Keys Configuration Program �"
                    "\n�        By Felix Wu         �"
                    "\n�        June 5,1993         �"
                    "\n藩様様様様様様様様様様様様様夕\n"
                };
    struct data keydata;
    FILE *fp;

    fp=fopen("KEY.CFG","wb");
    printf("%s",info);
    printf("DOWN: ");
    keydata.down=getkey();
    printf("�\nLEFT: ");
    keydata.left=getkey();
    printf("�\nRIGHT: ");
    keydata.right=getkey();
    printf("�\nCLOCKWISE ROTATE: ");
    keydata.crotate=getkey();
    printf("�\nCOUNTER-COLCKWISE ROTATE: ");
    keydata.arotate=getkey();
    printf("�\nDone.\n");
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
