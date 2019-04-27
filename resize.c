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
BMPImage resize(BMPImage img, float *shifting, int shifting_num);
void save(BMPImage img, char *fileName);

int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "WRONG INPUT. Should be: %s xxx.bmp psf\n", argv[0]);
        exit(1);
    }
    
    BMPImage img = my_read(argv[1]);
    int i=0;
    char * lin = NULL;
    size_t len = 0;
    int shift_num = 0;
    char line[4];

    FILE *fp = fopen(argv[2], "r");
    while ((getline(&lin, &len, fp)) != -1) {
        shift_num++;
    }
    fclose ( fp );
    
    FILE *f = fopen(argv[2], "r");
    float shift[shift_num];
    while ( fgets ( line, sizeof line, fp ) != NULL ) /* read a line */
    {
        fputs ( line, stdout ); /* write the line */
        shift[i]=(float)atof(line);
        i++;
    }
    fclose ( f );
    char* dir_name={"Resized/"};
    char name[37];
    char secp[11], firp[26];

    for(int i=0;i<11;i++){
        name[i]=dir_name[i];
    }

    for(int i=0;i<15;i++){
        firp[i]=argv[1][i];
    }
    strcpy(secp, "Resized.bmp");
    strcat(firp, secp);
    strcat(name,firp);

    BMPImage new_img = resize(img, shift, shift_num);

    DIR* dir = opendir("Resized");
    if (dir)
    {
        save(new_img, name);
        closedir(dir);
    }
    else if (ENOENT == errno)
    {
        mkdir("Resized", 0700);
        save(new_img, name);
    }
    
    return 0;
}


BMPImage my_read(char *fileName) {

    FILE *fp = fopen(fileName, "rb");
    if(fp==NULL){
        fprintf(stderr,"The file %s is not opened\n",fileName);
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

void save(BMPImage img, char *fileName) {
    int padding = (4-(img.w*img.bytes_per_pixel)%4)%4 ;
    FILE *fp = fopen(fileName, "wr");

    fwrite(img.header, img.size_of_header, 1, fp);

    fseek(fp, 2, SEEK_SET);
    int size = img.size_of_header+ img.bytes_per_pixel* img.w*img.h;
    fwrite(&size, 1, 4, fp);
   
    fseek(fp, 18, SEEK_SET);
    fwrite(&img.w, 1, 4, fp);
    fseek(fp, img.size_of_header, SEEK_SET);

    int a = 0;
    for (int y = 0; y<img.h; y++) {
        fwrite(&img.data[y*img.w*img.bytes_per_pixel], 1, img.w*img.bytes_per_pixel, fp);
        if (padding) fwrite(&a, 1, padding, fp);
    }
}

BMPImage resize(BMPImage img, float shifting[], int shifting_num) {

    int max_shifting=shifting[0];
    for (int i=1; i < shifting_num; i++) if(max_shifting < shifting[i]) max_shifting = shifting[i]; //finding maximum shifting on the image
    for (int i = 0; i < shifting_num; i++) shifting[i] = max_shifting / shifting[i]; //finding psf


    float max_psf = shifting[0];
    for (int i=1; i < shifting_num; i++) if(max_psf < shifting[i]) max_psf = shifting[i]; //finding maximum psf

    BMPImage new_img;

    new_img.size_of_header = img.size_of_header;//as header size is the same for bmp images
    new_img.header = (char*) malloc(new_img.size_of_header * sizeof(char));//allocating header for new image
    
    for (int i = 0; i < new_img.size_of_header; i++) new_img.header[i] = img.header[i];// making them equal for now

    new_img.w = img.w*max_psf; //to make sure that our image will have enough width to full fill whole shifting
    new_img.h = img.h; //height is still the same
    new_img.bytes_per_pixel = img.bytes_per_pixel;//as the bpp (rgb) are the same

    new_img.data = (unsigned char*)calloc(new_img.w*new_img.h*new_img.bytes_per_pixel, sizeof(unsigned char));
    //allocation for data (pixels)

    int step = new_img.h / shifting_num; // height of each piece of image (223=2230/10)
    for (int pos_h = 0; pos_h < shifting_num; pos_h++){ // it will go from one piece to another  
        int x_start = (new_img.w - img.w*shifting[pos_h])/2;// starting point
        
        for (int y = pos_h * step; y < (pos_h + 1) * step && y < img.h; y++) { // while filling the current step
            int img_x = 0;
            int new_img_x = x_start;

            float cur_psf = shifting[pos_h]; // starting from the max psf (1.75 and etc.)
            float pixel_to_fill = 1.0;//as the initially pixels is filling one by one
            while (img_x < img.w && new_img_x < new_img.w){ //while we do not went out of image
                if (cur_psf >= pixel_to_fill) {
                    for (int i = 0; i < new_img.bytes_per_pixel; i++) { // rgb
                        new_img.data[(y*new_img.w + new_img_x)*new_img.bytes_per_pixel + i] += pixel_to_fill*img.data[(y*img.w + img_x)*img.bytes_per_pixel + i];
                    }
                    cur_psf-=pixel_to_fill;
                    pixel_to_fill = 1.0;
                    new_img_x++;
                }else{
                    for (int i = 0; i < new_img.bytes_per_pixel; i++) { // rgb
                        new_img.data[(y*new_img.w + new_img_x)*new_img.bytes_per_pixel + i] += cur_psf*img.data[(y*img.w + img_x)*img.bytes_per_pixel + i];
                    }
                    pixel_to_fill = 1-cur_psf;
                    cur_psf = shifting[pos_h];
                    img_x++;
                }
            }
        }
    }
    return new_img;
}

