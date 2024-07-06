#include <time.h>
#include <stdio.h>
#include <x86intrin.h>
#include "simd.h"
//__m128 _mm_div_ps(__m128 a, __m128 b); 
//Four floating point divisions in single precision (i.e. float

//__m128i _mm_max_epi8(__m128i a, __m128i b);
//Sixteen max operations over signed 8-bit integers (i.e. char)

//__m128i _mm_srai_epi16(__m128i a, int imm8);
//Arithmetic shift right of eight signed 16-bit integers (i.e. short)

long long int sum(int vals[NUM_ELEMS]) {
	clock_t start = clock();

	long long int sum = 0;
	for(unsigned int w = 0; w < OUTER_ITERATIONS; w++) {
		for(unsigned int i = 0; i < NUM_ELEMS; i++) {
			if(vals[i] >= 128) {
				sum += vals[i];
			}
		}
	}
	clock_t end = clock();
	printf("Time taken: %Lf s\n", (long double)(end - start) / CLOCKS_PER_SEC);
	return sum;
}

long long int sum_unrolled(int vals[NUM_ELEMS]) {
	clock_t start = clock();
	long long int sum = 0;

	for(unsigned int w = 0; w < OUTER_ITERATIONS; w++) {
		for(unsigned int i = 0; i < NUM_ELEMS / 4 * 4; i += 4) {
			if(vals[i] >= 128) sum += vals[i];
			if(vals[i + 1] >= 128) sum += vals[i + 1];
			if(vals[i + 2] >= 128) sum += vals[i + 2];
			if(vals[i + 3] >= 128) sum += vals[i + 3];
		}

		//This is what we call the TAIL CASE
		//For when NUM_ELEMS isn't a multiple of 4
		//NONTRIVIAL FACT: NUM_ELEMS / 4 * 4 is the largest multiple of 4 less than NUM_ELEMS
		for(unsigned int i = NUM_ELEMS / 4 * 4; i < NUM_ELEMS; i++) {
			if (vals[i] >= 128) {
				sum += vals[i];
			}
		}
	}
	clock_t end = clock();
	printf("Time taken: %Lf s\n", (long double)(end - start) / CLOCKS_PER_SEC);
	return sum;
}

long long int sum_simd(int vals[NUM_ELEMS]) {
	clock_t start = clock();
	__m128i _127 = _mm_set1_epi32(127);		// This is a vector with 127s in it... Why might you need this?
	long long int result = 0;				   // This is where you should put your final result!
	/* DO NOT DO NOT DO NOT DO NOT WRITE ANYTHING ABOVE THIS LINE. */
	
	for(unsigned int w = 0; w < OUTER_ITERATIONS; w++) {
		/* YOUR CODE GOES HERE */
		/* You'll need a tail case. */
		__m128i inner_sum = _mm_setzero_si128();
		__m128i elements;
		__m128i and_flag;
		int inner_sum_array[4] = {0, 0, 0, 0};
		for (unsigned int i = 0; i < NUM_ELEMS / 4 * 4; i += 4) {
			elements = _mm_loadu_si128((__m128i*)(vals + i));
			and_flag = _mm_cmpgt_epi32(elements, _127);
			elements = _mm_and_si128(and_flag, elements);
			inner_sum = _mm_add_epi32(inner_sum, elements);
		}
		_mm_storeu_si128((__m128i*)inner_sum_array, inner_sum);
		result = result + inner_sum_array[0] + inner_sum_array[1] 
		+ inner_sum_array[2] + inner_sum_array[3];
		for (unsigned int i = NUM_ELEMS / 4 * 4; i < NUM_ELEMS; i++) {
			if (vals[i] >= 128) {
				result = result + vals[i];
			}
		}
	}
	clock_t end = clock();
	printf("Time taken: %Lf s\n", (long double)(end - start) / CLOCKS_PER_SEC);
	return result;
}

long long int sum_simd_unrolled(int vals[NUM_ELEMS]) {
	clock_t start = clock();
	__m128i _127 = _mm_set1_epi32(127);
	long long int result = 0;
	for(unsigned int w = 0; w < OUTER_ITERATIONS; w++) {
		/* COPY AND PASTE YOUR sum_simd() HERE */
		/* MODIFY IT BY UNROLLING IT */
		__m128i inner_sum = _mm_setzero_si128();
		__m128i elements0;
		__m128i elements1;
		__m128i elements2;
		__m128i elements3;
		__m128i and_flag0;
		__m128i and_flag1;
		__m128i and_flag2;
		__m128i and_flag3;
		int inner_sum_array[4] = {0, 0, 0, 0};
		for (unsigned int i = 0; i < NUM_ELEMS; i += 16) {
			elements0 = _mm_loadu_si128((__m128i*)(vals + i));
			elements1 = _mm_loadu_si128((__m128i*)(vals + i + 4));
			elements2 = _mm_loadu_si128((__m128i*)(vals + i + 8));
			elements3 = _mm_loadu_si128((__m128i*)(vals + i + 12));
			and_flag0 = _mm_cmpgt_epi32(elements0, _127);
			and_flag1 = _mm_cmpgt_epi32(elements1, _127);
			and_flag2 = _mm_cmpgt_epi32(elements2, _127);
			and_flag3 = _mm_cmpgt_epi32(elements3, _127);
			elements0 = _mm_and_si128(and_flag0, elements0);
			elements1 = _mm_and_si128(and_flag1, elements1);
			elements2 = _mm_and_si128(and_flag2, elements2);
			elements3 = _mm_and_si128(and_flag3, elements3);
			inner_sum = _mm_add_epi32(inner_sum, elements0);
			inner_sum = _mm_add_epi32(inner_sum, elements1);
			inner_sum = _mm_add_epi32(inner_sum, elements2);
			inner_sum = _mm_add_epi32(inner_sum, elements3);
		}
		_mm_storeu_si128((__m128i*)inner_sum_array, inner_sum);
		result = result + inner_sum_array[0] + inner_sum_array[1] + inner_sum_array[2] + inner_sum_array[3];
		for (unsigned int i = NUM_ELEMS / 16 * 16; i < NUM_ELEMS; i++) {
			if (vals[i] >= 128) {
				result = result + vals[i];
			}
		}
		/* You'll need 1 or maybe 2 tail cases here. */

	}
	clock_t end = clock();
	printf("Time taken: %Lf s\n", (long double)(end - start) / CLOCKS_PER_SEC);
	return result;
}