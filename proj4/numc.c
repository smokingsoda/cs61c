#include "numc.h"
//#include "/opt/homebrew/Cellar/python@3.12/3.12.4/Frameworks/Python.framework/Versions/3.12/include/python3.12/structmember.h"
#include <structmember.h>


PyTypeObject Matrix61cType;

/* Helper functions for initalization of matrices and vectors */

/*
 * Return a tuple given rows and cols
 */
PyObject *get_shape(int rows, int cols) {
  if (rows == 1 || cols == 1) {
    return PyTuple_Pack(1, PyLong_FromLong(rows * cols));
  } else {
    return PyTuple_Pack(2, PyLong_FromLong(rows), PyLong_FromLong(cols));
  }
}
/*
 * Matrix(rows, cols, low, high). Fill a matrix random double values
 */
int init_rand(PyObject *self, int rows, int cols, unsigned int seed, double low,
              double high) {
    matrix *new_mat;
    int alloc_failed = allocate_matrix(&new_mat, rows, cols);
    if (alloc_failed) return alloc_failed;
    rand_matrix(new_mat, seed, low, high);
    ((Matrix61c *)self)->mat = new_mat;
    ((Matrix61c *)self)->shape = get_shape(new_mat->rows, new_mat->cols);
    return 0;
}

/*
 * Matrix(rows, cols, val). Fill a matrix of dimension rows * cols with val
 */
int init_fill(PyObject *self, int rows, int cols, double val) {
    matrix *new_mat;
    int alloc_failed = allocate_matrix(&new_mat, rows, cols);
    if (alloc_failed)
        return alloc_failed;
    else {
        fill_matrix(new_mat, val);
        ((Matrix61c *)self)->mat = new_mat;
        ((Matrix61c *)self)->shape = get_shape(new_mat->rows, new_mat->cols);
    }
    return 0;
}

/*
 * Matrix(rows, cols, 1d_list). Fill a matrix with dimension rows * cols with 1d_list values
 */
int init_1d(PyObject *self, int rows, int cols, PyObject *lst) {
    if (rows * cols != PyList_Size(lst)) {
        PyErr_SetString(PyExc_ValueError, "Incorrect number of elements in list");
        return -1;
    }
    matrix *new_mat;
    int alloc_failed = allocate_matrix(&new_mat, rows, cols);
    if (alloc_failed) return alloc_failed;
    int count = 0;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            set(new_mat, i, j, PyFloat_AsDouble(PyList_GetItem(lst, count)));
            count++;
        }
    }
    ((Matrix61c *)self)->mat = new_mat;
    ((Matrix61c *)self)->shape = get_shape(new_mat->rows, new_mat->cols);
    return 0;
}

/*
 * Matrix(2d_list). Fill a matrix with dimension len(2d_list) * len(2d_list[0])
 */
int init_2d(PyObject *self, PyObject *lst) {
    int rows = PyList_Size(lst);
    if (rows == 0) {
        PyErr_SetString(PyExc_ValueError,
                        "Cannot initialize numc.Matrix with an empty list");
        return -1;
    }
    int cols;
    if (!PyList_Check(PyList_GetItem(lst, 0))) {
        PyErr_SetString(PyExc_ValueError, "List values not valid");
        return -1;
    } else {
        cols = PyList_Size(PyList_GetItem(lst, 0));
    }
    for (int i = 0; i < rows; i++) {
        if (!PyList_Check(PyList_GetItem(lst, i)) ||
                PyList_Size(PyList_GetItem(lst, i)) != cols) {
            PyErr_SetString(PyExc_ValueError, "List values not valid");
            return -1;
        }
    }
    matrix *new_mat;
    int alloc_failed = allocate_matrix(&new_mat, rows, cols);
    if (alloc_failed) return alloc_failed;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            set(new_mat, i, j,
                PyFloat_AsDouble(PyList_GetItem(PyList_GetItem(lst, i), j)));
        }
    }
    ((Matrix61c *)self)->mat = new_mat;
    ((Matrix61c *)self)->shape = get_shape(new_mat->rows, new_mat->cols);
    return 0;
}

/*
 * This deallocation function is called when reference count is 0
 */
void Matrix61c_dealloc(Matrix61c *self) {
    deallocate_matrix(self->mat);
    Py_TYPE(self)->tp_free(self);
}

/* For immutable types all initializations should take place in tp_new */
PyObject *Matrix61c_new(PyTypeObject *type, PyObject *args,
                        PyObject *kwds) {
    /* size of allocated memory is tp_basicsize + nitems*tp_itemsize*/
    Matrix61c *self = (Matrix61c *)type->tp_alloc(type, 0);
    return (PyObject *)self;
}

/*
 * This matrix61c type is mutable, so needs init function. Return 0 on success otherwise -1
 */
int Matrix61c_init(PyObject *self, PyObject *args, PyObject *kwds) {
    /* Generate random matrices */
    if (kwds != NULL) {
        PyObject *rand = PyDict_GetItemString(kwds, "rand");
        if (!rand) {
            PyErr_SetString(PyExc_TypeError, "Invalid arguments");
            return -1;
        }
        if (!PyBool_Check(rand)) {
            PyErr_SetString(PyExc_TypeError, "Invalid arguments");
            return -1;
        }
        if (rand != Py_True) {
            PyErr_SetString(PyExc_TypeError, "Invalid arguments");
            return -1;
        }

        PyObject *low = PyDict_GetItemString(kwds, "low");
        PyObject *high = PyDict_GetItemString(kwds, "high");
        PyObject *seed = PyDict_GetItemString(kwds, "seed");
        double double_low = 0;
        double double_high = 1;
        unsigned int unsigned_seed = 0;

        if (low) {
            if (PyFloat_Check(low)) {
                double_low = PyFloat_AsDouble(low);
            } else if (PyLong_Check(low)) {
                double_low = PyLong_AsLong(low);
            }
        }

        if (high) {
            if (PyFloat_Check(high)) {
                double_high = PyFloat_AsDouble(high);
            } else if (PyLong_Check(high)) {
                double_high = PyLong_AsLong(high);
            }
        }

        if (double_low >= double_high) {
            PyErr_SetString(PyExc_TypeError, "Invalid arguments");
            return -1;
        }

        // Set seed if argument exists
        if (seed) {
            if (PyLong_Check(seed)) {
                unsigned_seed = PyLong_AsUnsignedLong(seed);
            }
        }

        PyObject *rows = NULL;
        PyObject *cols = NULL;
        if (PyArg_UnpackTuple(args, "args", 2, 2, &rows, &cols)) {
            if (rows && cols && PyLong_Check(rows) && PyLong_Check(cols)) {
                return init_rand(self, PyLong_AsLong(rows), PyLong_AsLong(cols), unsigned_seed, double_low,
                                 double_high);
            }
        } else {
            PyErr_SetString(PyExc_TypeError, "Invalid arguments");
            return -1;
        }
    }
    PyObject *arg1 = NULL;
    PyObject *arg2 = NULL;
    PyObject *arg3 = NULL;
    if (PyArg_UnpackTuple(args, "args", 1, 3, &arg1, &arg2, &arg3)) {
        /* arguments are (rows, cols, val) */
        if (arg1 && arg2 && arg3 && PyLong_Check(arg1) && PyLong_Check(arg2) && (PyLong_Check(arg3)
                || PyFloat_Check(arg3))) {
            if (PyLong_Check(arg3)) {
                return init_fill(self, PyLong_AsLong(arg1), PyLong_AsLong(arg2), PyLong_AsLong(arg3));
            } else
                return init_fill(self, PyLong_AsLong(arg1), PyLong_AsLong(arg2), PyFloat_AsDouble(arg3));
        } else if (arg1 && arg2 && arg3 && PyLong_Check(arg1) && PyLong_Check(arg2) && PyList_Check(arg3)) {
            /* Matrix(rows, cols, 1D list) */
            return init_1d(self, PyLong_AsLong(arg1), PyLong_AsLong(arg2), arg3);
        } else if (arg1 && PyList_Check(arg1) && arg2 == NULL && arg3 == NULL) {
            /* Matrix(rows, cols, 1D list) */
            return init_2d(self, arg1);
        } else if (arg1 && arg2 && PyLong_Check(arg1) && PyLong_Check(arg2) && arg3 == NULL) {
            /* Matrix(rows, cols, 1D list) */
            return init_fill(self, PyLong_AsLong(arg1), PyLong_AsLong(arg2), 0);
        } else {
            PyErr_SetString(PyExc_TypeError, "Invalid arguments");
            return -1;
        }
    } else {
        PyErr_SetString(PyExc_TypeError, "Invalid arguments");
        return -1;
    }
}

/*
 * List of lists representations for matrices
 */
PyObject *Matrix61c_to_list(Matrix61c *self) {
    int rows = self->mat->rows;
    int cols = self->mat->cols;
    PyObject *py_lst = NULL;
    if (self->mat->is_1d) {  // If 1D matrix, print as a single list
        py_lst = PyList_New(rows * cols);
        int count = 0;
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                PyList_SetItem(py_lst, count, PyFloat_FromDouble(get(self->mat, i, j)));
                count++;
            }
        }
    } else {  // if 2D, print as nested list
        py_lst = PyList_New(rows);
        for (int i = 0; i < rows; i++) {
            PyList_SetItem(py_lst, i, PyList_New(cols));
            PyObject *curr_row = PyList_GetItem(py_lst, i);
            for (int j = 0; j < cols; j++) {
                PyList_SetItem(curr_row, j, PyFloat_FromDouble(get(self->mat, i, j)));
            }
        }
    }
    return py_lst;
}

PyObject *Matrix61c_class_to_list(Matrix61c *self, PyObject *args) {
    PyObject *mat = NULL;
    if (PyArg_UnpackTuple(args, "args", 1, 1, &mat)) {
        if (!PyObject_TypeCheck(mat, &Matrix61cType)) {
            PyErr_SetString(PyExc_TypeError, "Argument must of type numc.Matrix!");
            return NULL;
        }
        Matrix61c* mat61c = (Matrix61c*)mat;
        return Matrix61c_to_list(mat61c);
    } else {
        PyErr_SetString(PyExc_TypeError, "Invalid arguments");
        return NULL;
    }
}

/*
 * Add class methods
 */
PyMethodDef Matrix61c_class_methods[] = {
    {"to_list", (PyCFunction)Matrix61c_class_to_list, METH_VARARGS, "Returns a list representation of numc.Matrix"},
    {NULL, NULL, 0, NULL}
};

/*
 * Matrix61c string representation. For printing purposes.
 */
PyObject *Matrix61c_repr(PyObject *self) {
    PyObject *py_lst = Matrix61c_to_list((Matrix61c *)self);
    return PyObject_Repr(py_lst);
}

/* NUMBER METHODS */

/*
 * Add the second numc.Matrix (Matrix61c) object to the first one. The first operand is
 * self, and the second operand can be obtained by casting `args`.
 */
PyObject *Matrix61c_add(Matrix61c* self, PyObject* args) {
    /* TODO: YOUR CODE HERE */
    if (PyObject_TypeCheck(args, &Matrix61cType) == 0) {
        PyErr_SetString(PyExc_TypeError, "Expected a Matrix61c object");
        return NULL;
    }
    Matrix61c *other = (Matrix61c *) args;
    Matrix61c *rv = (Matrix61c *) Matrix61c_new(&Matrix61cType, NULL, NULL);
    matrix *new_mat;
    int flag;
    flag = allocate_matrix(&new_mat, self->mat->rows, self->mat->cols);
    if (flag == -2) {
        PyErr_SetString(PyExc_RuntimeError, "Malloc fails");
        return NULL;
    } else if (flag == -1){
        PyErr_SetString(PyExc_ValueError, "Expected the equal dimensions");
        return NULL;
    }
    rv->mat = new_mat;
    rv->shape = PyTuple_New(2);
    PyObject *item1 = PyLong_FromLong(self->mat->rows);
    PyTuple_SetItem(rv->shape, 0, item1);
    PyObject *item2 = PyLong_FromLong(self->mat->cols);
    PyTuple_SetItem(rv->shape, 1, item2);
    flag = add_matrix(rv->mat, self->mat, other->mat);
    if (flag == -1) {
        PyErr_SetString(PyExc_ValueError, "Expected the equal dimensions");
        return NULL;
    }
    return (PyObject *) rv;
}

/*
 * Substract the second numc.Matrix (Matrix61c) object from the first one. The first operand is
 * self, and the second operand can be obtained by casting `args`.
 */
PyObject *Matrix61c_sub(Matrix61c* self, PyObject* args) {
    /* TODO: YOUR CODE HERE */
    if (PyObject_TypeCheck(args, &Matrix61cType) == 0) {
        PyErr_SetString(PyExc_TypeError, "Expected a Matrix61c object");
        return NULL;
    }
    Matrix61c *other = (Matrix61c *) args;
    Matrix61c *rv = (Matrix61c *) Matrix61c_new(&Matrix61cType, NULL, NULL);
    matrix *new_mat;
    int flag;
    flag = allocate_matrix(&new_mat, self->mat->rows, self->mat->cols);
    if (flag == -2) {
        PyErr_SetString(PyExc_RuntimeError, "Malloc fails");
        return NULL;
    } else if (flag == -1){
        PyErr_SetString(PyExc_ValueError, "Expected the equal dimensions");
        return NULL;
    }
    rv->mat = new_mat;
    rv->shape = PyTuple_New(2);
    PyObject *item1 = PyLong_FromLong(self->mat->rows);
    PyTuple_SetItem(rv->shape, 0, item1);
    PyObject *item2 = PyLong_FromLong(self->mat->cols);
    PyTuple_SetItem(rv->shape, 1, item2);
    flag = sub_matrix(rv->mat, self->mat, other->mat);
    if (flag == -1) {
        PyErr_SetString(PyExc_ValueError, "Expected the equal dimensions");
        return NULL;
    }
    return (PyObject *) rv;
}

/*
 * NOT element-wise multiplication. The first operand is self, and the second operand
 * can be obtained by casting `args`.
 */
PyObject *Matrix61c_multiply(Matrix61c* self, PyObject *args) {
    /* TODO: YOUR CODE HERE */
    if (PyObject_TypeCheck(args, &Matrix61cType) == 0) {
        PyErr_SetString(PyExc_TypeError, "Expected a Matrix61c object");
        return NULL;
    }
    Matrix61c *other = (Matrix61c *) args;
    Matrix61c *rv = (Matrix61c *) Matrix61c_new(&Matrix61cType, NULL, NULL);
    matrix *new_mat;
    int flag;
    flag = allocate_matrix(&new_mat, self->mat->rows, other->mat->cols);
    if (flag == -2) {
        PyErr_SetString(PyExc_RuntimeError, "Malloc fails");
        return NULL;
    } else if (flag == -1){
        PyErr_SetString(PyExc_ValueError, "Expected the equal dimensions");
        return NULL;
    }
    rv->mat = new_mat;
    rv->shape = PyTuple_New(2);
    PyObject *item1 = PyLong_FromLong(self->mat->rows);
    PyTuple_SetItem(rv->shape, 0, item1);
    PyObject *item2 = PyLong_FromLong(self->mat->cols);
    PyTuple_SetItem(rv->shape, 1, item2);
    flag = mul_matrix(rv->mat, self->mat, other->mat);
    if (flag == -1) {
        PyErr_SetString(PyExc_ValueError, "Expected the equal dimensions");
        return NULL;
    }
    return (PyObject *) rv;
}

/*
 * Negates the given numc.Matrix.
 */
PyObject *Matrix61c_neg(Matrix61c* self) {
    /* TODO: YOUR CODE HERE */
    Matrix61c *rv = (Matrix61c *) Matrix61c_new(&Matrix61cType, NULL, NULL);
    matrix *new_mat;
    int flag;
    flag = allocate_matrix(&new_mat, self->mat->rows, self->mat->cols);
    if (flag == -2) {
        PyErr_SetString(PyExc_RuntimeError, "Malloc fails");
        return NULL;
    } else if (flag == -1){
        PyErr_SetString(PyExc_ValueError, "Expected the equal dimensions");
        return NULL;
    }
    rv->mat = new_mat;
    rv->shape = PyTuple_New(2);
    PyObject *item1 = PyLong_FromLong(self->mat->rows);
    PyTuple_SetItem(rv->shape, 0, item1);
    PyObject *item2 = PyLong_FromLong(self->mat->cols);
    PyTuple_SetItem(rv->shape, 1, item2);
    flag = neg_matrix(rv->mat, self->mat);
    if (flag == -1) {
        PyErr_SetString(PyExc_ValueError, "Expected the equal dimensions");
        return NULL;
    }
    return (PyObject *) rv;
}

/*
 * Take the element-wise absolute value of this numc.Matrix.
 */
PyObject *Matrix61c_abs(Matrix61c *self) {
    /* TODO: YOUR CODE HERE */
    Matrix61c* rv = (Matrix61c*)Matrix61c_new(&Matrix61cType, NULL, NULL);
    matrix* tmp_mat = NULL;
    int allocate_failed = allocate_matrix(&tmp_mat, self->mat->rows, self->mat->cols);
    if (allocate_failed) return NULL;

    abs_matrix(tmp_mat, self->mat);

    rv->mat = tmp_mat;
    rv->shape = get_shape(tmp_mat->rows, tmp_mat->cols);
    return (PyObject*)rv;
}

/*
 * Raise numc.Matrix (Matrix61c) to the `pow`th power. You can ignore the argument `optional`.
 */
PyObject *Matrix61c_pow(Matrix61c *self, PyObject *pow, PyObject *optional) {
    /* TODO: YOUR CODE HERE */
    if (PyObject_TypeCheck(pow, &PyLong_Type) == 0) {
        PyErr_SetString(PyExc_TypeError, "Expected a Long object");
        return NULL;
    }
    long pow_num = PyLong_AsLong(pow);
    Matrix61c *rv = (Matrix61c *) Matrix61c_new(&Matrix61cType, NULL, NULL);
    matrix *new_mat;
    int flag;
    flag = allocate_matrix(&new_mat, self->mat->rows, self->mat->cols);
    if (flag == -2) {
        PyErr_SetString(PyExc_RuntimeError, "Malloc fails");
        return NULL;
    } else if (flag == -1){
        PyErr_SetString(PyExc_ValueError, "Expected the equal dimensions");
        return NULL;
    }
    rv->mat = new_mat;
    rv->shape = PyTuple_New(2);
    PyObject *item1 = PyLong_FromLong(self->mat->rows);
    PyTuple_SetItem(rv->shape, 0, item1);
    PyObject *item2 = PyLong_FromLong(self->mat->cols);
    PyTuple_SetItem(rv->shape, 1, item2);
    flag = pow_matrix(rv->mat, self->mat, pow_num);
    if (flag == -1) {
        PyErr_SetString(PyExc_ValueError, "Expected the equal dimensions");
        return NULL;
    } else if (flag == -2) {
        PyErr_SetString(PyExc_RuntimeError, "Malloc fails");
    }
    return (PyObject *) rv;
    
}

/*
 * Create a PyNumberMethods struct for overloading operators with all the number methods you have
 * define. You might find this link helpful: https://docs.python.org/3.6/c-api/typeobj.html
 */
PyNumberMethods Matrix61c_as_number = {
    /* TODO: YOUR CODE HERE */
    .nb_add = (binaryfunc)Matrix61c_add,           // binaryfunc nb_add;
    .nb_subtract = (binaryfunc)Matrix61c_sub,      // binaryfunc nb_subtract;
    .nb_multiply = (binaryfunc)Matrix61c_multiply, // binaryfunc nb_multiply;
    .nb_power = (ternaryfunc)Matrix61c_pow,        // ternaryfunc nb_power;
    .nb_negative = (unaryfunc)Matrix61c_neg,       // unaryfunc nb_negative;
    .nb_absolute = (unaryfunc)Matrix61c_abs,       // unaryfunc nb_absolute;
};


/* INSTANCE METHODS */

/*
 * Given a numc.Matrix self, parse `args` to (int) row, (int) col, and (double/int) val.
 * Return None in Python (this is different from returning null).
 */
PyObject *Matrix61c_set_value(Matrix61c *self, PyObject* args) {
    /* TODO: YOUR CODE HERE */
    int row, col;
    double val;
    if (!PyArg_ParseTuple(args, "iid", &row, &col, &val))
    {
        PyErr_SetString(PyExc_TypeError, "Invalid arguments");
        return NULL;
    }
    if (row < 0 || col < 0 || row >= self->mat->rows || col >= self->mat->cols)
    {
        PyErr_SetString(PyExc_IndexError, "row or column index out of range");
        return NULL;
    }
    set(self->mat, row, col, val);
    return Py_BuildValue("");
}

/*
 * Given a numc.Matrix `self`, parse `args` to (int) row and (int) col.
 * Return the value at the `row`th row and `col`th column, which is a Python
 * float/int.
 */
PyObject *Matrix61c_get_value(Matrix61c *self, PyObject* args) {
    /* TODO: YOUR CODE HERE */
    int row, col;
    double val;
    if (!PyArg_ParseTuple(args, "iid", &row, &col, &val))
    {
        PyErr_SetString(PyExc_TypeError, "Invalid arguments");
        return NULL;
    }
    if (row < 0 || col < 0 || row >= self->mat->rows || col >= self->mat->cols)
    {
        PyErr_SetString(PyExc_IndexError, "row or column index out of range");
        return NULL;
    }
    return Py_BuildValue("d", get(self->mat, row, col));
}

/*
 * Create an array of PyMethodDef structs to hold the instance methods.
 * Name the python function corresponding to Matrix61c_get_value as "get" and Matrix61c_set_value
 * as "set"
 * You might find this link helpful: https://docs.python.org/3.6/c-api/structures.html
 */
PyMethodDef Matrix61c_methods[] = {
    /* TODO: YOUR CODE HERE */
    {NULL, NULL, 0, NULL},
    {"set", (PyCFunction)Matrix61c_set_value, METH_VARARGS,"Change the value at a specific row and column index"},
    {"get", (PyCFunction)Matrix61c_get_value, METH_VARARGS,"Get the value at a specific row and column index"},
};

/* INDEXING */


/*
 * Given a numc.Matrix `self`, index into it with `key`. Return the indexed result.
 */
PyObject* Matrix61c_subscript(Matrix61c* self, PyObject* key) {
    /* YOUR CODE HERE */

    Py_ssize_t row_offset = 0, col_offset = 0;
    Py_ssize_t row = self->mat->rows, col = self->mat->cols;

    int is_single_num = parse_subscript(self, key, &row_offset, &col_offset, &row, &col);

    if (is_single_num == 1) {
        double result = get(self->mat, row_offset, col_offset);
        return Py_BuildValue("d", result);
    } else if (is_single_num == 0) {
        matrix* tmp_mat = NULL;
        Matrix61c* rv = (Matrix61c*)Matrix61c_new(&Matrix61cType, NULL, NULL);

        int allocate_failed = allocate_matrix_ref(&tmp_mat, self->mat, row_offset, col_offset, row, col);
        if (allocate_failed) return NULL;

        rv->mat = tmp_mat;
        rv->shape = get_shape(tmp_mat->rows, tmp_mat->cols);
        return (PyObject*)rv;
    } else {
        /* Return value is -1, execution failed */
        PyErr_SetString(PyExc_RuntimeError, "Something wrong happened when parsing subscripts");
        return NULL;
    }
}

/*
 * Given a numc.Matrix `self`, index into it with `key`, and set the indexed result to `v`.
 */
int Matrix61c_set_subscript(Matrix61c* self, PyObject* key, PyObject* v) {
    /* TODO: YOUR CODE HERE */
    Py_ssize_t row_offset = 0, col_offset = 0;
    Py_ssize_t row = self->mat->rows, col = self->mat->cols;

    int is_single_num = parse_subscript(self, key, &row_offset, &col_offset, &row, &col);

    if (is_single_num == -1) {
        PyErr_SetString(PyExc_RuntimeError, "Something wrong happened when parsing subscripts");
        return -1;
    } else if (is_single_num == 1) {
        if (!PyLong_Check(v) && !PyFloat_Check(v)) {
            PyErr_SetString(PyExc_TypeError, "Resulting slice is 1 by 1, but v is not a float or int.");
            return -1;
        }
        double target;
        PyArg_Parse(v, "d", &target);
        set(self->mat, row_offset, col_offset, target);
        return 0;
    } else {
        /* 2D matrix */
        if (!PyList_Check(v)) {
            PyErr_SetString(PyExc_TypeError, "Resulting slice is not 1 by 1, but v is not a list.");
            return -1;
        }
        /* Set the values */
        if (row == 1 || col == 1) {
            /* Resulting slice is a 1D array.
             * row_label == 1 indicates this is a row_array,
             * col_label == 1 indicates this is a col_array.
             */
            int row_label = (row == 1) ? 1 : 0;
            int col_label = 1 - row_label;

            int total_num = col * row_label + row * col_label;
            if (PyList_Size(v) != total_num) {
                PyErr_SetString(PyExc_ValueError, "Wrong size of the input list");
                return -1;
            }
            /* fill the array */
            PyObject* element = NULL;
            double* tmp = (double*)malloc(sizeof(double) * total_num);

            for (Py_ssize_t p = 0; p < total_num; p++) {
                element = PyList_GetItem(v, p);
                if (!PyLong_Check(element) && !PyFloat_Check(element)) {
                    PyErr_SetString(PyExc_ValueError, "Element should be a single number");
                    return -1;
                }
                PyArg_Parse(element, "d", tmp + p);
            }
            /* Prepare to fill in */
            for (int position = 0; position < total_num; position++) {
                set(self->mat, row_offset + position * col_label,
                    col_offset + position * row_label, tmp[position]);
            }

            free(tmp);
            return 0;
        } else {
            /* TODO: Fill in a 2D slice */
            if (PyList_Size(v) != row) {
                PyErr_SetString(PyExc_ValueError, "Wrong size of the input list");
                return -1;
            }

            double** tmp = (double**)malloc(sizeof(double*) * row);
            PyObject* row_array = NULL;
            PyObject* element = NULL;
            double target;
            for (int r = 0; r < row; r++) {
                tmp[r] = (double*)malloc(sizeof(double) * col);
                row_array = PyList_GetItem(v, r);
                if (!PyList_Check(row_array) || PyList_Size(row_array) != col) {
                    PyErr_SetString(PyExc_RuntimeError, "Some row in the list is invalid");
                }
                for (int c = 0; c < col; c++) {
                    element = PyList_GetItem(row_array, c);
                    if (!PyLong_Check(element) && !PyFloat_Check(element)) {
                        PyErr_SetString(PyExc_TypeError, "Element should be a single number");
                        return -1;
                    }
                    PyArg_Parse(element, "d", &target);
                    tmp[r][c] = target;
                }
            }

            /* Fill in the elements */
            for (int r = 0; r < row; r++) {
                for (int c = 0; c < col; c++) {
                    set(self->mat, row_offset + r, col_offset + c, tmp[r][c]);
                }
                free(tmp[r]);
            }
            free(tmp);
            return 0;
        }
    }
}

PyMappingMethods Matrix61c_mapping = {
    NULL,
    (binaryfunc) Matrix61c_subscript,
    (objobjargproc) Matrix61c_set_subscript,
};

/* INSTANCE ATTRIBUTES*/
PyMemberDef Matrix61c_members[] = {
    {
        "shape", T_OBJECT_EX, offsetof(Matrix61c, shape), 0,
        "(rows, cols)"
    },
    {NULL}  /* Sentinel */
};

PyTypeObject Matrix61cType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "numc.Matrix",
    .tp_basicsize = sizeof(Matrix61c),
    .tp_dealloc = (destructor)Matrix61c_dealloc,
    .tp_repr = (reprfunc)Matrix61c_repr,
    .tp_as_number = &Matrix61c_as_number,
    .tp_flags = Py_TPFLAGS_DEFAULT |
    Py_TPFLAGS_BASETYPE,
    .tp_doc = "numc.Matrix objects",
    .tp_methods = Matrix61c_methods,
    .tp_members = Matrix61c_members,
    .tp_as_mapping = &Matrix61c_mapping,
    .tp_init = (initproc)Matrix61c_init,
    .tp_new = Matrix61c_new
};


struct PyModuleDef numcmodule = {
    PyModuleDef_HEAD_INIT,
    "numc",
    "Numc matrix operations",
    -1,
    Matrix61c_class_methods
};

/* Initialize the numc module */
PyMODINIT_FUNC PyInit_numc(void) {
    PyObject* m;

    if (PyType_Ready(&Matrix61cType) < 0)
        return NULL;

    m = PyModule_Create(&numcmodule);
    if (m == NULL)
        return NULL;

    Py_INCREF(&Matrix61cType);
    PyModule_AddObject(m, "Matrix", (PyObject *)&Matrix61cType);
    printf("CS61C Fall 2020 Project 4: numc imported!\n");
    fflush(stdout);
    return m;
}

/* return 0 if a new matrix needs to be created.
 * if there's only a single number, return 1.
 */
int parse_subscript(Matrix61c* self, PyObject* key,
    Py_ssize_t* row_offset,
    Py_ssize_t* col_offset,
    Py_ssize_t* row,
    Py_ssize_t* col) {
    if (self->mat->is_1d) {
        if (PyTuple_Check(key)) {
            PyErr_SetString(PyExc_TypeError, "1D matrix only support single slice");
            PyErr_Print();
            return -1;
        }

        Py_ssize_t start, length;
        parse_basic_info(key, &start, &length);

        if (self->mat->rows == 1) {
            *row_offset = 0;
            *col_offset = start;
            *row = 1;
            *col = length;
            return length == 1 ? 1 : 0;
        } else {
            *row_offset = start;
            *col_offset = 0;
            *row = length;
            *col = 1;
            return length == 1 ? 1 : 0;
        }
    } else {
        if (PyLong_Check(key) || PySlice_Check(key)) {
            Py_ssize_t start, length;
            parse_basic_info(key, &start, &length);
            *row_offset = start;
            *col_offset = 0;
            *row = length;
            *col = self->mat->cols;
            /* Bug:
             * return length == 1 ? 1 : 0;
             */
            return 0;
        } else if (PyTuple_Check(key) && PyTuple_Size(key) == 2) {

            PyObject* row_info = NULL;
            PyObject* col_info = NULL;

            Py_ssize_t row_start, row_length;
            Py_ssize_t col_start, col_length;

            /* This line below doesn't work and I don't know why :( * PyTuple_GetItem works fine
             */
             // PyArg_ParseTuple(key, "O|O:subscript", row_info, col_info);
            row_info = PyTuple_GetItem(key, 0);
            col_info = PyTuple_GetItem(key, 1);

            /* Parse the basic information of these two dimensions */
            parse_basic_info(row_info, &row_start, &row_length);
            parse_basic_info(col_info, &col_start, &col_length);

            *row_offset = row_start;
            *col_offset = col_start;
            *row = row_length;
            *col = col_length;

            return (row_length == 1 && col_length == 1) ? 1 : 0;
        } else {
            PyErr_SetString(PyExc_TypeError, "The index can only be an integer, a single slice or a tuple of two integers/slices for a 2D matrix");
            PyErr_Print();
            return -1;
        }
    }
}

/* As the function name says */
void parse_basic_info(PyObject* key, Py_ssize_t* start, Py_ssize_t* length) {
    if (PyLong_Check(key)) {
        PyArg_Parse(key, "l", start);
        *length = 1;
    } else if (PySlice_Check(key)) {
        Py_ssize_t stop, step;
        PySlice_Unpack(key, start, &stop, &step);
        *length = stop - *start;
        if (step != 1 || *length < 1) {
            PyErr_SetString(PyExc_ValueError, "Invalid slice info");
            PyErr_Print();
        }
    } else {
        PyErr_SetString(PyExc_TypeError, "Invalid slice info");
    }
}