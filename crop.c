#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

typedef struct {
    int w;
    int h;
    unsigned char *data;
    char *header;
    int size_of_header;
    int bytes_per_pixel;
} BMPImage;

BMPImage my_read(char *fileName);
void crop(BMPImage img, int stripSize, char *fileName);


int main(int argc, char *argv[])
{
  
    if (argc != 2) {
        fprintf(stderr, "WRONG INPUT. Should be: %s xxx.bmp\n", argv[0]);
        exit(1);
    }

    BMPImage img = my_read(argv[1]);
    
    char name[60];
    char secp[22], firp[38];
    for(int i=0;i<15;i++){
        firp[i]=argv[1][i];
    }
    char* dir_name={"ResizedAndCropped/"};
    for(int i=0;i<20;i++){
        name[i]=dir_name[i];
    }
    strcpy(secp, "ResizedAndCropped.bmp");
    strcat(firp, secp);
    strcat(name,firp);


    DIR* dir = opendir("ResizedAndCropped");
    if (dir)
    {
        crop(img, 480, name);
        closedir(dir);
    }
    else if (ENOENT == errno)
    {
        mkdir("ResizedAndCropped", 0700);
        crop(img, 480, name);
    }
    

    return 0;
}

BMPImage my_read(char *fileName) {

    FILE *fp = fopen(fileName, "rb");
    BMPImage img;

    char ch[2];
    fread(ch, 1, 2, fp);// magic identifier 0x4d42
    if (!(ch[0] == 'B' && ch[1] == 'M')) exit(2);//if it is not BMP 

    int size;
    fread(&size, 1, 4, fp);// size of file
    fseek(fp, 4, SEEK_CUR);//reserved1 and reserved2

    int offset;
    fread(&offset, 1, 4, fp);//offset to image data in bytes from beginning of file (54 bytes)
    img.size_of_header = offset;

	fseek(fp, 4, SEEK_CUR);//DIB size of header    
    
    fseek(fp, 0, SEEK_SET);
    img.header = (char*) malloc(img.size_of_header * sizeof(char));
    fread(img.header, offset, 1, fp); 
    
    fseek(fp, 18, SEEK_SET);

    int w;
    fread(&w, 1, 4, fp);//width
    w = abs(w);

    int h;
    fread(&h, 1, 4, fp);//height
    h = abs(h);
 
    fseek(fp, 2, SEEK_CUR);//number of color planes

    short bits_per_pixel;
    fread(&bits_per_pixel, 1, 2, fp);//bits per pixel
    fseek(fp, offset, SEEK_SET);

    //putting data 
    img.w = w;
    img.h = h;
    img.bytes_per_pixel = abs(bits_per_pixel/8);
    img.data = (unsigned char*)malloc(w*h*img.bytes_per_pixel * sizeof(unsigned char));
    int padding = (4-(w*img.bytes_per_pixel)%4)%4 ;

    for (int y = 0; y < h; y++) {
        fread(&img.data[y * w * img.bytes_per_pixel], 1, w * img.bytes_per_pixel, fp);//filling data (pixels) line by line
        fseek(fp, padding, SEEK_CUR);
    }

    fclose(fp);    
    return img;
}

void crop(BMPImage img, int stripSize, char *fileName) {

    int padding = (4-(stripSize*img.bytes_per_pixel)%4)%4 ;//new padding
    int w = stripSize;//new width
    int offset = img.w/2 - stripSize/2;
    
    FILE *fp = fopen(fileName, "wr");

    fwrite(img.header, img.size_of_header, 1, fp);

    fseek(fp, 2, SEEK_SET);
    int size = img.size_of_header + img.bytes_per_pixel*w*img.h;//new size
    fwrite(&size, 1, 4, fp);
   
    fseek(fp, 18, SEEK_SET);
    fwrite(&w, 1, 4, fp);
    fseek(fp, img.size_of_header, SEEK_SET);

    int a = 0;
    for (int y = 0; y<img.h; y++) {
        fwrite(&img.data[(y*img.w + offset)*img.bytes_per_pixel], 1, w*img.bytes_per_pixel, fp);
        if (padding) fwrite(&a, 1, padding, fp);
    }
}