#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>

typedef struct {
    int w;
    int h;
    unsigned char *data;
    char *header;
    int size_of_header;
    int bytes_per_pixel;
} BMPImage;

BMPImage my_read(char *fileName); 
void glue(BMPImage *img, int s, char *name);
int mycmp( char *string );
void bubbleSort(int arr[], int n);

int main(int argc, char const *argv[])
{
    char *name[16];
    
    DIR *d;
    struct dirent *dir;
    d = opendir(".");
    if (d) {
        int qty=0;
        while ((dir = readdir(d)) != NULL) {
            if (!(!strcmp(dir->d_name, ".") || !strcmp(dir->d_name, "..")) && !mycmp(dir->d_name)){
                name[qty]=dir->d_name;//taking the name of photos to glue
                
                qty++;
            }
            // else{
            //     fprintf(stderr,"No files that end with Cropped.bmp\n");
            //     exit(2);
            // }
        }

    int val[qty];//to store the number in the name
    for(int i=0;i<qty;i++){
        char *p = name[i];
        while (*p){ 
            if ( isdigit(*p) || ( (*p=='-'||*p=='+') && isdigit(*(p+1)) )) {
            val[i] = (int) strtol(p, &p, 10);
            }else p++;
        }
    }

    bubbleSort(val, qty);//sorting number to know the right order

    BMPImage BitMap[16];
    for(int i=0;i<qty;i++){
        char *p = name[i];
        while (*p){ 
            if ( isdigit(*p) || ( (*p=='-'||*p=='+') && isdigit(*(p+1)) )) {
            int a = (int) strtol(p, &p, 10);
            for(int j=0;j<qty;j++){
                if(a==val[j]) {
                    BitMap[j] = my_read(name[i]);
                }
            }
            }else p++;
        }
    }
    
    DIR* dir1 = opendir("Glued");
    if (dir1)
    {
        glue(BitMap,qty,"Glued/glue.bmp");
        closedir(dir1);
    }
    else if (ENOENT == errno)
    {
        mkdir("Glued", 0700);
        glue(BitMap,qty,"./Glued/glue.bmp");
    }
    
    closedir(d);
    }
    return 0;
}

int mycmp( char *dirname ){
    dirname = strrchr(dirname, 'C');
    if (dirname != NULL) return(strcmp(dirname, "Cropped.bmp"));
    return( -1 );
}

void swap(int *xp, int *yp) 
{ 
    int temp = *xp; 
    *xp = *yp; 
    *yp = temp; 
} 

void bubbleSort(int arr[], int n) { 
   int i, j; 
   for (i = 0; i < n-1; i++)          
       for (j = 0; j < n-i-1; j++)  
           if (arr[j] > arr[j+1]) 
              swap(&arr[j], &arr[j+1]); 
} 

BMPImage my_read(char *fileName) {

    FILE *fp = fopen(fileName, "rb");
    if(fp==NULL){
        fprintf(stderr,"The file is not opened\n");
        exit(1);
    }
    BMPImage img;

    char ch[2];
    fread(ch, 1, 2, fp);// magic identifier 0x4d42
    if (!(ch[0] == 'B' && ch[1] == 'M')) {
        fprintf(stderr,"Error in format\n");
        exit(2);//if it is not BMP 
    }
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


void glue(BMPImage *img, int s, char *name){
    int padding = (4-((480 + 70*15)*img[0].bytes_per_pixel)%4)%4 ;
    int w = img[0].w+70*(s-1);//new width
    FILE *fp = fopen(name, "wr");

    fwrite(img[0].header, img[0].size_of_header, 1, fp);

    fseek(fp, 2, SEEK_SET);
    int size = img[0].size_of_header + img[0].bytes_per_pixel*w*img[0].h;//new size
    fwrite(&size, 1, 4, fp);
   
    fseek(fp, 18, SEEK_SET);
    fwrite(&w, 1, 4, fp);
    fseek(fp, img[0].size_of_header, SEEK_SET);

    int a = 0;
    
        for (int y = 0; y<img[0].h; y++) {
            for(int i=0; i<s;i++){
                if(i<s-1){
                    fwrite(&img[i].data[y*img[i].w*img[i].bytes_per_pixel], 1, 70*img[i].bytes_per_pixel, fp);
                    if (padding) fwrite(&a, 1, 70*i, fp);
                }
                else if(i==s-1){
                    fwrite(&img[i].data[(y*img[i].w)*img[i].bytes_per_pixel], 1, img[i].w*img[i].bytes_per_pixel, fp);
                    if (padding) fwrite(&a, 1, 70*i, fp);
                }
            }
        }

    fclose(fp);
}
