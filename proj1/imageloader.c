/************************************************************************
**
** NAME:        imageloader.c
**
** DESCRIPTION: CS61C Fall 2020 Project 1
**
** AUTHOR:      Dan Garcia  -  University of California at Berkeley
**              Copyright (C) Dan Garcia, 2020. All rights reserved.
**              Justin Yokota - Starter Code
**				Yuyang Tian
**
**
** DATE:        2023-10-22
**
**************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include "imageloader.h"

//Opens a .ppm P3 image file, and constructs an Image object.
//You may find the function fscanf useful.
//Make sure that you close the file with fclose before returning.
Image *readData(char *filename) {

    //YOUR CODE HERE
    FILE *imagefile = fopen(filename,"r");
    if(imagefile == NULL){
        printf("fail to open %s\n", filename);
        return NULL;
    }
    char format[3];
    int maxcolor;
    fscanf(imagefile, "%s", format);
    if(format[0] != 'P' || format[1] != '3'){
        printf("%s wrong format\n", filename);
        return NULL;
    }
    Image *img = (Image*) malloc(sizeof(Image));
    fscanf(imagefile, "%d", &(img->cols));
    fscanf(imagefile, "%d", &(img->rows));
    fscanf(imagefile, "%d", &maxcolor);
    if(img->rows < 0 || img->cols < 0 || maxcolor != 255){
        printf("%s wrong format\n", filename);
        return NULL;
    }
    int total = img->rows * img->cols;
    img->image = (Color **) malloc(total * sizeof(Color*));
    for(int i = 0; i < total; i++) {
        *(img->image + i) = (Color*) malloc(sizeof(Color));
        fscanf(imagefile, "%hhu %hhu %hhu",
                &((*(img->image + i))->R),
                &((*(img->image + i))->G),
                &((*(img->image + i))->B));
    }
    fclose(imagefile);
    return img;
}



//Given an image, prints to stdout (e.g. with printf) a .ppm P3 file with the image's data.
void writeData(Image *image)
{
	//YOUR CODE HERE
    printf("P3\n%d %d\n255\n", image->cols, image->rows);
    Color** p = image->image;
    for (int i = 0; i < image->rows; i++) {
        for (int j = 0; j < image->cols-1; j++) {
            //*p is read the value from p, *p is a Color pointer
            printf("%3hhu %3hhu %3hhu   ", (*p)->R, (*p)->G, (*p)->B);
            p++;
        }
    //must treat the last pixel separately.
        printf("%3hhu %3hhu %3hhu\n", (*p)->R, (*p)->G, (*p)->B);
        p++;
    }
}

//Frees an image
void freeImage(Image *image)
{
    //YOUR CODE HERE
    int total = image->cols * image->rows;
    for(int i = 0; i < total; i++){
        free(*(image->image + i));
    }
    free(image->image);
    free(image);

}