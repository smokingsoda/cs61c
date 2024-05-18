/************************************************************************
**
** NAME:        gameoflife.c
**
** DESCRIPTION: CS61C Fall 2020 Project 1
**
** AUTHOR:      Justin Yokota - Starter Code
**              Junru Wang
**
**
** DATE:        2020-08-23
**
**************************************************************************/

#include <complex.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "imageloader.h"

int dx[8] = {-1, -1, -1, 0, 0, 1, 1, 1};
int dy[8] = {1, 0, -1, 1, -1, 1, 0, -1};

int ring(int n, int m) {
    return (n + m) % m;
}


//Determines what color the cell at the given row/col should be. This function allocates space for a new Color.
//Note that you will need to read the eight neighbors of the cell in question. The grid "wraps", so we treat the top row as adjacent to the bottom row
//and the left column as adjacent to the right column.
Color *evaluateOneCell(Image *image, int row, int col, uint32_t rule)
{
    //YOUR CODE HERE
    int live_countR = 0, live_countG = 0, live_countB = 0;
    int isLiveR = ((*(image->image + image->cols * row + col))->R == 255);
    int isLiveG = ((*(image->image + image->cols * row + col))->G == 255);
    int isLiveB = ((*(image->image + image->cols * row + col))->B == 255);
    Color *colorP = (Color*) malloc(sizeof(Color));
    Color *neighbour;
    for (int i = -1; i < 2; i++) {
        for (int j = -1; j < 2; j++) {
            int newRow = row + i;
            int newCol = col + j;
            if (newRow < 0) {
                newRow = image->rows - 1;
            }
            if (newRow >= image->rows) {
                newRow = 0;
            }
            if (newCol < 0) {
                newCol = image->cols - 1;
            }
            if (newCol >= image->cols) {
                newCol = 0;
            }
            if (newRow == row && newCol == col) {
                continue;
            }
            neighbour = *(image->image + newRow * image->cols + newCol);
            live_countR += (neighbour->R == 255);
            live_countG += (neighbour->G == 255);
            live_countB += (neighbour->B == 255);
        }

    }
    int parR = 9 * isLiveR + live_countR;
    int parG = 9 * isLiveG + live_countG;
    int parB = 9 * isLiveB + live_countB;
    if (1<<parR & rule) {
        colorP->R = 255;
    } else {
        colorP->R = 0;
    }
    if (1<<parG & rule) {
        colorP->G = 255;
    } else {
        colorP->G = 0;
    }
    if (1<<parB & rule) {
        colorP->B = 255;
    } else {
        colorP->B = 0;
    }
    return colorP;
}

//The main body of Life; given an image and a rule, computes one iteration of the Game of Life.
//You should be able to copy most of this from steganography.c
Image *life(Image *image, uint32_t rule)
{
    //YOUR CODE HERE
    Image *new_image = (Image*) malloc(sizeof(Image));
    new_image->rows = image->rows;
    new_image->cols = image->cols;
    new_image->image = (Color**) malloc((new_image->cols * new_image->rows) * sizeof(Color*));
    Color** p = new_image->image;
    for (int i = 0; i < new_image->rows; ++i) {
        for (int j = 0; j < new_image->cols; ++j) {
            *p = evaluateOneCell(image, i, j, rule);
            p++;
        }
    }
    return new_image;
}

/*
Loads a .ppm from a file, computes the next iteration of the game of life, then prints to stdout the new image.

argc stores the number of arguments.
argv stores a list of arguments. Here is the expected input:
argv[0] will store the name of the program (this happens automatically).
argv[1] should contain a filename, containing a .ppm.
argv[2] should contain a hexadecimal number (such as 0x1808). Note that this will be a string.
You may find the function strtol useful for this conversion.
If the input is not correct, a malloc fails, or any other error occurs, you should exit with code -1.
Otherwise, you should return from main with code 0.
Make sure to free all memory before returning!

You may find it useful to copy the code from steganography.c, to start.
*/
int main(int argc, char **argv)
{
    //YOUR CODE HERE
    if (argc != 3) {
        printf("usage: ./gameOfLife filename rule\nfilename is an ASCII PPM file (type P3) with maximum value 255.\nrule is a hex number beginning with 0x; Life is 0x1808.");
        return 1;
    }
    Image *img = readData(argv[1]);
    uint32_t rule = strtol(argv[2], NULL, 16);
    Image *nextImg = life(img, rule);
    writeData(nextImg);
    freeImage(img);
    freeImage(nextImg);
    return 0;
}
