#include "matrix.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

// Include SSE intrinsics
#if defined(_MSC_VER)
#include <intrin.h>
#elif defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__))
#include <immintrin.h>
#include <x86intrin.h>
#endif

/* Below are some intel intrinsics that might be useful
 * void _mm256_storeu_pd (double * mem_addr, __m256d a)
 * __m256d _mm256_set1_pd (double a)
 * __m256d _mm256_set_pd (double e3, double e2, double e1, double e0)
 * __m256d _mm256_loadu_pd (double const * mem_addr)
 * __m256d _mm256_add_pd (__m256d a, __m256d b)
 * __m256d _mm256_sub_pd (__m256d a, __m256d b)
 * __m256d _mm256_fmadd_pd (__m256d a, __m256d b, __m256d c)
 * __m256d _mm256_mul_pd (__m256d a, __m256d b)
 * __m256d _mm256_cmp_pd (__m256d a, __m256d b, const int imm8)
 * __m256d _mm256_and_pd (__m256d a, __m256d b)
 * __m256d _mm256_max_pd (__m256d a, __m256d b)
*/

/*
 * Generates a random double between `low` and `high`.
 */
double rand_double(double low, double high) {
    double range = (high - low);
    double div = RAND_MAX / range;
    return low + (rand() / div);
}

/*
 * Generates a random matrix with `seed`.
 */
void rand_matrix(matrix *result, unsigned int seed, double low, double high) {
    srand(seed);
    for (int i = 0; i < result->rows; i++) {
        for (int j = 0; j < result->cols; j++) {
            set(result, i, j, rand_double(low, high));
        }
    }
}

/*
 * Allocate space for a matrix struct pointed to by the double pointer mat with
 * `rows` rows and `cols` columns. You should also allocate memory for the data array
 * and initialize all entries to be zeros. Remember to set all fields of the matrix struct.
 * `parent` should be set to NULL to indicate that this matrix is not a slice.
 * You should return -1 if either `rows` or `cols` or both have invalid values, or if any
 * call to allocate memory in this function fails. If you don't set python error messages here upon
 * failure, then remember to set it in numc.c.
 * Return 0 upon success and non-zero upon failure.
 */
int allocate_matrix(matrix **mat, int rows, int cols) {
    /* TODO: YOUR CODE HERE */
    if (rows <= 0 || cols <= 0) {
        return -1;
    }
    *mat = (matrix *) malloc(sizeof(matrix)); //allocate memory for (*mat) pointer
    if (*mat == NULL) {
        return -2;
    }
    (*mat)->data = (double **) malloc(sizeof(double *) * rows); //allocate memory for date double pointer
    if ((*mat)->data == NULL) {
        return -2;
    }
    for (int i = 0; i < rows; ++i) {
        *((*mat)->data + i) = (double *) malloc(sizeof(double) * cols);
        for (int j = 0; j < cols; ++j) {
            *((*((*mat)->data + i)) + j) = 0;
        }
    }
    (*mat)->rows = rows;
    (*mat)->cols = cols;
    if (rows == 1 || cols == 1) {
        (*mat)->is_1d = 1;
    } else {
        (*mat)->is_1d = 0;
    }
    (*mat)->parent = NULL;
    (*mat)->ref_cnt = 1;

    return 0;
}

/*
 * Allocate space for a matrix struct pointed to by `mat` with `rows` rows and `cols` columns.
 * This is equivalent to setting the new matrix to be
 * from[row_offset:row_offset + rows, col_offset:col_offset + cols]
 * If you don't set python error messages here upon failure, then remember to set it in numc.c.
 * Return 0 upon success and non-zero upon failure.
 */
int allocate_matrix_ref(matrix **mat, matrix *from, int row_offset, int col_offset,
                        int rows, int cols) {
    /* TODO: YOUR CODE HERE */
    if (row_offset < 0 || col_offset < 0 || rows < 0 || cols < 0 || row_offset + rows > from->rows || col_offset + cols > from->cols) {
        return -1;
    }
    *mat = (matrix *) malloc(sizeof(matrix));
    if (rows == 1 || cols == 1) {
        (*mat)->is_1d = -1;
    } else {
        (*mat)->is_1d = 0;
    }
    (*mat)->rows = rows;
    (*mat)->cols = cols;
    (*mat)->data = (double **) malloc(sizeof(double *) * (*mat)->rows);
    if ((*mat)->data == NULL) {
        return -2;
   }
   for (int i = 0; i < rows; i++) {
        *((*mat)->data + i) = (*(from->data + row_offset + i)) + col_offset;
   }
    from->ref_cnt += 1;
    (*mat)->ref_cnt = from->ref_cnt;
    (*mat)->parent = from;
    return 0;
}

/*
 * This function will be called automatically by Python when a numc matrix loses all of its
 * reference pointers.
 * You need to make sure that you only free `mat->data` if no other existing matrices are also
 * referring this data array.
 * See the spec for more information.
 */
void deallocate_matrix(matrix *mat) {
    /* TODO: YOUR CODE HERE */
    if (mat == NULL) {
        return;
    }
    if (mat->ref_cnt == 1) {
        for (int i = 0; i < mat->rows; ++i) {
            free(*(mat->data + i));
        }
        free(mat->data);
    }
    free(mat);
}

/*
 * Return the double value of the matrix at the given row and column.
 * You may assume `row` and `col` are valid.
 */
double get(matrix *mat, int row, int col) {
    /* TODO: YOUR CODE HERE */
    return *(*((mat->data) + row) + col);
}

/*
 * Set the value at the given row and column to val. You may assume `row` and
 * `col` are valid
 */
void set(matrix *mat, int row, int col, double val) {
    /* TODO: YOUR CODE HERE */
    if (row < 0 || col < 0 || row >= mat->rows || row >= mat->cols) {
        PyErr_SetString(PyExc_IndexError, "Set Value Out of Range");
        return;
    }
    *(*((mat->data) + row) + col) = val;
}

/*
 * Set all entries in mat to val
 */
void fill_matrix(matrix *mat, double val) {
    /* TODO: YOUR CODE HERE */
    for (int i = 0; i < mat->rows; ++i) {
        for (int j = 0; j < mat->cols; ++j) {
            *(*((mat->data) + i) + j) = val;
        }
    }
}

/*
 * Store the result of adding mat1 and mat2 to `result`.
 * Return 0 upon success and a nonzero value upon failure.
 */
int add_matrix(matrix *result, matrix *mat1, matrix *mat2) {
    /* TODO: YOUR CODE HERE */
    if (mat1->rows != mat2->rows || mat1->cols != mat2->cols || result->rows != mat1->rows || result->cols != mat1->cols) {
        return -1;
    }
    int rows = result->rows;
    int cols = result->cols;
    int boundary = cols / 16 * 16;
    __m256d result_element0, result_element1, result_element2, result_element3;
    __m256d mat1_element0, mat1_element1, mat1_element2, mat1_element3;
    __m256d mat2_element0, mat2_element1, mat2_element2, mat2_element3; //256 bit can contain 4 double
    #pragma omp parallel for collapse(2)
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < boundary; j+=16) {
                mat1_element0 = _mm256_loadu_pd(&(mat1->data[i][j]));
                mat1_element1 = _mm256_loadu_pd(&(mat1->data[i][j + 4]));
                mat1_element2 = _mm256_loadu_pd(&(mat1->data[i][j + 8]));
                mat1_element3 = _mm256_loadu_pd(&(mat1->data[i][j + 12]));

                mat2_element0 = _mm256_loadu_pd(&(mat2->data[i][j]));
                mat2_element1 = _mm256_loadu_pd(&(mat2->data[i][j + 4]));
                mat2_element2 = _mm256_loadu_pd(&(mat2->data[i][j + 8]));
                mat2_element3 = _mm256_loadu_pd(&(mat2->data[i][j + 12]));

                result_element0 = _mm256_add_pd(mat1_element0, mat2_element0);
                result_element1 = _mm256_add_pd(mat1_element1, mat2_element1);
                result_element2 = _mm256_add_pd(mat1_element2, mat2_element2);
                result_element3 = _mm256_add_pd(mat1_element3, mat2_element3);

                _mm256_storeu_pd(&(result->data[i][j]), result_element0);
                _mm256_storeu_pd(&(result->data[i][j + 4]), result_element1);
                _mm256_storeu_pd(&(result->data[i][j + 8]), result_element2);
                _mm256_storeu_pd(&(result->data[i][j + 12]), result_element3);
                //*(*(result->data + i) + j) = *(*(mat1->data + i) + j) + *(*(mat2->data + i) + j);
            }
        }
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < rows; i++) {
        for (int j = boundary; j < cols; j++) {
            result->data[i][j] = mat1->data[i][j] + mat2->data[i][j];
        }
    }
    return 0;

}

/*
 * Store the result of subtracting mat2 from mat1 to `result`.
 * Return 0 upon success and a nonzero value upon failure.
 */
int sub_matrix(matrix *result, matrix *mat1, matrix *mat2) {
    /* TODO: YOUR CODE HERE */
    if (mat1->rows != mat2->rows || mat1->cols != mat2->cols || result->rows != mat1->rows || result->cols != mat1->cols) {
        return -1;
    }
    int rows = result->rows;
    int cols = result->cols;
    int boundary = cols / 16 * 16;
    __m256d result_element0, result_element1, result_element2, result_element3;
    __m256d mat1_element0, mat1_element1, mat1_element2, mat1_element3;
    __m256d mat2_element0, mat2_element1, mat2_element2, mat2_element3; //256 bit can contain 4 double
    #pragma omp parallel for collapse(2)
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < boundary; j+=16) {
                mat1_element0 = _mm256_loadu_pd(&(mat1->data[i][j]));
                mat1_element1 = _mm256_loadu_pd(&(mat1->data[i][j + 4]));
                mat1_element2 = _mm256_loadu_pd(&(mat1->data[i][j + 8]));
                mat1_element3 = _mm256_loadu_pd(&(mat1->data[i][j + 12]));

                mat2_element0 = _mm256_loadu_pd(&(mat2->data[i][j]));
                mat2_element1 = _mm256_loadu_pd(&(mat2->data[i][j + 4]));
                mat2_element2 = _mm256_loadu_pd(&(mat2->data[i][j + 8]));
                mat2_element3 = _mm256_loadu_pd(&(mat2->data[i][j + 12]));

                result_element0 = _mm256_sub_pd(mat1_element0, mat2_element0);
                result_element1 = _mm256_sub_pd(mat1_element1, mat2_element1);
                result_element2 = _mm256_sub_pd(mat1_element2, mat2_element2);
                result_element3 = _mm256_sub_pd(mat1_element3, mat2_element3);

                _mm256_storeu_pd(&(result->data[i][j]), result_element0);
                _mm256_storeu_pd(&(result->data[i][j + 4]), result_element1);
                _mm256_storeu_pd(&(result->data[i][j + 8]), result_element2);
                _mm256_storeu_pd(&(result->data[i][j + 12]), result_element3);
                //*(*(result->data + i) + j) = *(*(mat1->data + i) + j) - *(*(mat2->data + i) + j);
            }
        }
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < rows; i++) {
        for (int j = boundary; j < cols; j++) {
            result->data[i][j] = mat1->data[i][j] - mat2->data[i][j];
        }
    }
    return 0;
}

/*
 * Store the result of multiplying mat1 and mat2 to `result`.
 * Return 0 upon success and a nonzero value upon failure.
 * Remember that matrix multiplication is not the same as multiplying individual elements.
 */
int mul_matrix(matrix *result, matrix *mat1, matrix *mat2) {
    /* TODO: YOUR CODE HERE */
    if (mat1->cols != mat2->rows || result->rows != mat1->rows || result->cols != mat2->cols) {
        return -1;
    }
    int new_row = mat1->rows;
    int new_col = mat2->cols;
    int middle = mat1->cols;
    __m256d result_element;
    __m256d mat1_element;
    __m256d mat2_element;
    int col_boundary = new_col / 4 * 4;
    for (int k = 0; k < middle; k++) {
        for (int i = 0; i < new_row; k++) {
            for (int j = 0; j < col_boundary; j += 4) {
                if (k == 0) {
                    result_element = _mm256_setzero_pd();
                } else {
                    result_element = _mm256_loadu_pd(&(result->data[i][j]));
                }
                mat1_element = _mm256_set1_pd(mat1->data[i][k]);
                mat2_element = _mm256_loadu_pd(&(mat2->data[k][j]));
                result_element = _mm256_fmadd_pd(mat1_element, mat2_element, result_element);
                _mm256_storeu_pd(&(result->data[i][j]), result_element);
                //(*(*(result->data + i) + j)) = (*(*(result->data + i) + j) + ((*(*(mat1->data + i) + k)) * (*(*(mat2->data + k) + j))));
            }
        }
    }

    for (int i = 0; i < new_row; ++i) {
        for (int j = col_boundary; j < new_col; ++j) {
            for (int k = 0; k < middle; k++) {
                (*(*(result->data + i) + j)) = (*(*(result->data + i) + j) + ((*(*(mat1->data + i) + k)) * (*(*(mat2->data + k) + j))));
            }
        }
    }

    return 0;
}

/*
 * Store the result of raising mat to the (pow)th power to `result`.
 * Return 0 upon success and a nonzero value upon failure.
 * Remember that pow is defined with matrix multiplication, not element-wise multiplication.
 */
int pow_matrix(matrix *result, matrix *mat, int pow) {
    /* TODO: YOUR CODE HERE */
    if (result->rows != mat->rows || result->cols != mat->cols || mat->rows != mat->cols) {
        return -1;
    }
    matrix *mid1 = NULL;
    matrix *mid2 = NULL;
    int flag;
    flag = allocate_matrix(&mid1, mat->rows, mat->rows);
    if (flag != 0) {
        return -2;
    }
    flag = allocate_matrix(&mid2, mat->rows, mat->rows);
    if (flag != 0) {
        return -2;
    }
    for (int i = 1; i <= pow; ++i) {
        if (i == 1) {
            for (int m = 0; m < mat->rows; ++m) {
                for (int n = 0; n < mat->cols; ++n) {
                    mid1->data[m][n] = mat->data[m][n];
                }
            }
        } else if (i % 2 == 1) {
            mul_matrix(mid1, mid2, mat);
        } else if (i % 2 == 0) {
            mul_matrix(mid2, mid1, mat);
        }
    }
    if (pow % 2 == 1) {
        for (int m = 0; m < mid1->rows; ++m) {
            for (int n = 0; n < mid1->cols; ++n) {
                result->data[m][n] = mid1->data[m][n];
            }
        }
    } else {
        for (int m = 0; m < mid2->rows; ++m) {
            for (int n = 0; n < mid2->cols; ++n) {
                result->data[m][n] = mid2->data[m][n];
            }
        }
    }
    deallocate_matrix(mid1);
    deallocate_matrix(mid2);
    return 0;
}

/*
 * Store the result of element-wise negating mat's entries to `result`.
 * Return 0 upon success and a nonzero value upon failure.
 */
int neg_matrix(matrix *result, matrix *mat) {
    /* TODO: YOUR CODE HERE */
    if (result->rows != mat->rows || result->cols != mat->cols) {
        return -1;
    }
    int rows = result->rows;
    int cols = result->cols;
    int boundary = cols / 16 * 16;
    __m256d result_element0, result_element1, result_element2, result_element3;
    __m256d mat_element0, mat_element1, mat_element2, mat_element3;
    __m256d _neg = _mm256_set1_pd(-0.0);
    #pragma omp parallel for collapse(2)
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < boundary; j+=16) {
                mat_element0 = _mm256_loadu_pd(&(mat->data[i][j]));
                mat_element1 = _mm256_loadu_pd(&(mat->data[i][j + 4]));
                mat_element2 = _mm256_loadu_pd(&(mat->data[i][j + 8]));
                mat_element3 = _mm256_loadu_pd(&(mat->data[i][j + 12]));

                result_element0 = _mm256_xor_pd(mat_element0, _neg);
                result_element1 = _mm256_xor_pd(mat_element1, _neg);
                result_element2 = _mm256_xor_pd(mat_element2, _neg);
                result_element3 = _mm256_xor_pd(mat_element3, _neg);

                _mm256_storeu_pd(&(result->data[i][j]), result_element0);
                _mm256_storeu_pd(&(result->data[i][j + 4]), result_element1);
                _mm256_storeu_pd(&(result->data[i][j + 8]), result_element2);
                _mm256_storeu_pd(&(result->data[i][j + 12]), result_element3);
                //*(*(result->data + i) + j) = *(*(mat1->data + i) + j) - *(*(mat2->data + i) + j);
            }
        }
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < rows; i++) {
        for (int j = boundary; j < cols; j++) {
            result->data[i][j] = - mat->data[i][j];
        }
    }
    return 0;
}

/*
 * Store the result of taking the absolute value element-wise to `result`.
 * Return 0 upon success and a nonzero value upon failure.
 */
int abs_matrix(matrix *result, matrix *mat) {
    /* TODO: YOUR CODE HERE */
    if (result->rows != mat->rows || result->cols != mat->cols) {
        return -1;
    }
    int rows = result->rows;
    int cols = result->cols;
    int boundary = cols / 16 * 16;
    __m256d result_element0, result_element1, result_element2, result_element3;
    __m256d mat_element0, mat_element1, mat_element2, mat_element3;
    __m256d positive_flag0, positive_flag1, positive_flag2, positive_flag3;
    __m256d mask0, mask1, mask2, mask3;
    __m256d _neg = _mm256_set1_pd(-0.0);
    __m256d _zero = _mm256_set1_pd(0.0);
    #pragma omp parallel for collapse(2)
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < boundary; j+=16) {
                
                mat_element0 = _mm256_loadu_pd(&(mat->data[i][j]));
                mat_element1 = _mm256_loadu_pd(&(mat->data[i][j + 4]));
                mat_element2 = _mm256_loadu_pd(&(mat->data[i][j + 8]));
                mat_element3 = _mm256_loadu_pd(&(mat->data[i][j + 12]));

                //if is less than, then return ones, else return zeros
                mask0 = _mm256_cmp_pd(mat_element0, _zero, _CMP_LT_OQ);
                mask1 = _mm256_cmp_pd(mat_element1, _zero, _CMP_LT_OQ);
                mask2 = _mm256_cmp_pd(mat_element2, _zero, _CMP_LT_OQ);
                mask3 = _mm256_cmp_pd(mat_element3, _zero, _CMP_LT_OQ);

                //if mask is 1, then second, else first
                positive_flag0 = _mm256_blendv_pd(_zero, _neg, mask0);
                positive_flag1 = _mm256_blendv_pd(_zero, _neg, mask1);
                positive_flag2 = _mm256_blendv_pd(_zero, _neg, mask2);
                positive_flag3 = _mm256_blendv_pd(_zero, _neg, mask3);

                result_element0 = _mm256_xor_pd(mat_element0, positive_flag0);
                result_element1 = _mm256_xor_pd(mat_element1, positive_flag1);
                result_element2 = _mm256_xor_pd(mat_element2, positive_flag2);
                result_element3 = _mm256_xor_pd(mat_element3, positive_flag3);

                _mm256_storeu_pd(&(result->data[i][j]), result_element0);
                _mm256_storeu_pd(&(result->data[i][j + 4]), result_element1);
                _mm256_storeu_pd(&(result->data[i][j + 8]), result_element2);
                _mm256_storeu_pd(&(result->data[i][j + 12]), result_element3);
                //*(*(result->data + i) + j) = *(*(mat1->data + i) + j) - *(*(mat2->data + i) + j);
            }
        }
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < rows; i++) {
        for (int j = boundary; j < cols; j++) {
            if (mat->data[i][j] < 0) {
                result->data[i][j] = - mat->data[i][j];
            } else {
                result->data[i][j] = mat->data[i][j];
            }

        }
    }
    return 0;
}

