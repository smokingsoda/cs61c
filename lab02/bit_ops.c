#include <stdio.h>
#include "bit_ops.h"

// Return the nth bit of x.
// Assume 0 <= n <= 31
unsigned get_bit(unsigned x,
                 unsigned n) {
    // YOUR CODE HERE
    // Returning -1 is a placeholder (it makes
    // no sense, because get_bit only returns 
    // 0 or 1)
    x = x >> n;
	unsigned result = x & 1;
	return result;
    return -1;
}
// Set the nth bit of the value of x to v.
// Assume 0 <= n <= 31, and v is 0 or 1
void set_bit(unsigned * x,
             unsigned n,
             unsigned v) {
    // YOUR CODE HERE
	unsigned helper1 = ~(1 << n);
	*x = *x & helper1;
	unsigned helper2 = v << n;
	*x = *x | helper2;
}
// Flip the nth bit of the value of x.
// Assume 0 <= n <= 31
void flip_bit(unsigned * x,
              unsigned n) {
    // YOUR CODE HERE
	// unsigned tem = 1;
	// tem = tem << n;
	// *x = *x ^ tem;
	unsigned helper = 1 << n;
	*x = *x ^ helper;
}

