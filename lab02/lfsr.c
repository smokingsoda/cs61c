#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "lfsr.h"

void lfsr_calculate(uint16_t *reg) {
    /* YOUR CODE HERE */
    uint16_t n0 = (*reg >> 0) & 1;
    uint16_t n2 = (*reg >> 2) & 1;
    uint16_t n3 = (*reg >> 3) & 1;
    uint16_t n5 = (*reg >> 5) & 1;
    uint16_t next = n0 ^ n2 ^ n3 ^ n5;
    *reg = *reg >> 1;
    next = next << 15;
    *reg = *reg | next;
}

