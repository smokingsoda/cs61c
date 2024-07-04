#include "numc.h"
#include <structmember.h>
//#include "/opt/homebrew/Cellar/python@3.12/3.12.4/Frameworks/Python.framework/Versions/3.12/include/python3.12/structmember.h"


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
    rv->shape = get_shape(new_mat->rows, new_mat->cols);
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
    rv->shape = get_shape(new_mat->rows, new_mat->cols);
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
    rv->shape = get_shape(new_mat->rows, new_mat->cols);
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
    rv->shape = get_shape(new_mat->rows, new_mat->cols);
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
    rv->shape = get_shape(new_mat->rows, new_mat->cols);
    flag = abs_matrix(rv->mat, self->mat);
    if (flag == -1) {
        PyErr_SetString(PyExc_ValueError, "Expected the equal dimensions");
        return NULL;
    }
    return (PyObject *) rv;
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
    rv->shape = get_shape(new_mat->rows, new_mat->cols);
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
    return Py_BuildValue(NULL);
}

/*
 * Given a numc.Matrix `self`, parse `args` to (int) row and (int) col.
 * Return the value at the `row`th row and `col`th column, which is a Python
 * float/int.
 */
PyObject *Matrix61c_get_value(Matrix61c *self, PyObject* args) {
    /* TODO: YOUR CODE HERE */
    int row, col;
    PyArg_ParseTuple(args, "ii", &row, &col);
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
PyObject *Matrix61c_subscript(Matrix61c* self, PyObject* key) {
    /* TODO: YOUR CODE HERE */
    int flag = parse_key(key);
    if (self->mat->is_1d) {
        if (flag == 0) {
            //Single int
            int index = PyLong_AsLong(key);
            if (self->mat->rows == 1) {
                return Matrix61c_get_value(self, get_shape(0, index));
            } else {
                return Matrix61c_get_value(self, get_shape(index, 0));
            }
        } else if (flag == 1) {
            //Single slice
            Matrix61c *rv = (Matrix61c *) Matrix61c_new(&Matrix61cType, NULL, NULL);
            Py_ssize_t start, stop, step;
            if (PySlice_Unpack(key, &start, &stop, &step) < 0) {
                PyErr_SetString(PyExc_ValueError, "Invalid slice object");
                return NULL;
            }
            if (step != 1 || stop - start < 1) {
                PyErr_SetString(PyExc_ValueError, "Slice info not valid");
                return NULL;
            }
            if (self->mat->rows == 1) {
                flag = allocate_matrix_ref(&(rv->mat),self->mat, 0, start, 1, stop - start);
            } else {
                flag = allocate_matrix_ref(&(rv->mat),self->mat, start, 0, stop - start, 1);
            }
            if (flag == -1) {
                PyErr_SetString(PyExc_IndexError, "Index out of range");
                return NULL;
            } else if (flag == -2) {
                PyErr_SetString(PyExc_RuntimeError, "Malloc fails");
                return NULL;
            }
            return (PyObject*) rv;
        } else {
            PyErr_SetString(PyExc_TypeError, "1D matrices only support single slice");
            return NULL;
        }
    } 
    if (flag == -3) {
        PyErr_SetString(PyExc_TypeError, "Key is not a tuple");
        return NULL;
    } else if (flag == -2) {
        PyErr_SetString(PyExc_TypeError, "Key is not an integer, a slice, or a length-2 tuple of slices/ints.");
        return NULL;
    } else if (flag == -1) {
        PyErr_SetString(PyExc_TypeError, "Key is not a two-element tuple");
        return NULL;
    } else if (flag == 0) {
        //Single number
        int row_index = PyLong_AsLong(key);
        if (row_index >= self->mat->rows) {
            PyErr_SetString(PyExc_IndexError, "Index out of range");
            return NULL;
        }
        Matrix61c *rv = (Matrix61c *) Matrix61c_new(&Matrix61cType, NULL, NULL);
        flag = allocate_matrix_ref(&(rv->mat),self->mat, row_index, 0, 1, self->mat->cols);
        if (flag == -1) {
            PyErr_SetString(PyExc_IndexError, "Index out of range");
            return NULL;
        } else if (flag == -2) {
            PyErr_SetString(PyExc_RuntimeError, "Malloc fails");
            return NULL;
        }
        return (PyObject*) rv;
    } else if (flag == 1) {
        //Single slice
        PyObject *slice = key;
        Py_ssize_t start, stop, step;
        if (PySlice_Unpack(slice, &start, &stop, &step) < 0) {
            PyErr_SetString(PyExc_ValueError, "Invalid slice object");
            return NULL;
        }
        if (step != 1 || stop - start < 1) {
            PyErr_SetString(PyExc_ValueError, "Slice info not valid");
            return NULL;
        }
        Matrix61c *rv = (Matrix61c *) Matrix61c_new(&Matrix61cType, NULL, NULL);
        flag = allocate_matrix_ref(&(rv->mat),self->mat, start, 0, stop - start, self->mat->cols);
        if (flag == -1) {
            PyErr_SetString(PyExc_IndexError, "Index out of range");
            return NULL;
        } else if (flag == -2) {
            PyErr_SetString(PyExc_RuntimeError, "Malloc fails");
            return NULL;
        }
        return (PyObject*) rv;
    } else if (flag == 2) {
        //int and slice
        PyObject *item0;
        PyObject *slice;
        item0 = PyTuple_GetItem(key, 0);
        slice = PyTuple_GetItem(key, 1);

        int row_index = PyLong_AsLong(item0);
        if (row_index >= self->mat->rows) {
            PyErr_SetString(PyExc_IndexError, "Index out of range");
            return NULL;
        }
        Py_ssize_t start, stop, step;
        if (PySlice_Unpack(slice, &start, &stop, &step) < 0) {
            PyErr_SetString(PyExc_ValueError, "Invalid slice object");
            return NULL;
        }
        if (step != 1 || stop - start < 1) {
            PyErr_SetString(PyExc_ValueError, "Slice info not valid");
            return NULL;
        }

        Matrix61c *rv = (Matrix61c *) Matrix61c_new(&Matrix61cType, NULL, NULL);
        flag = allocate_matrix_ref(&(rv->mat),self->mat, row_index, start, 1, stop - start);
        if (flag == -1) {
            PyErr_SetString(PyExc_IndexError, "Index out of range");
            return NULL;
        } else if (flag == -2) {
            PyErr_SetString(PyExc_RuntimeError, "Malloc fails");
            return NULL;
        }
        return (PyObject*) rv;
    } else if (flag == 3) {
        //slice and int
        PyObject *slice;
        PyObject *item1;
        slice = PyTuple_GetItem(key, 0);
        item1 = PyTuple_GetItem(key, 1);

        int col_index = PyLong_AsLong(item1);
        if (col_index >= self->mat->cols) {
            PyErr_SetString(PyExc_IndexError, "Index out of range");
            return NULL;
        }
        Py_ssize_t start, stop, step;
        if (PySlice_Unpack(slice, &start, &stop, &step) < 0) {
            PyErr_SetString(PyExc_ValueError, "Invalid slice object");
            return NULL;
        }
        if (step != 1 || stop - start < 1) {
            PyErr_SetString(PyExc_ValueError, "Slice info not valid");
            return NULL;
        }

        Matrix61c *rv = (Matrix61c *) Matrix61c_new(&Matrix61cType, NULL, NULL);
        flag = allocate_matrix_ref(&(rv->mat),self->mat, start, col_index, stop - start, 1);
        if (flag == -1) {
            PyErr_SetString(PyExc_IndexError, "Index out of range");
            return NULL;
        } else if (flag == -2) {
            PyErr_SetString(PyExc_RuntimeError, "Malloc fails");
            return NULL;
        }
        return (PyObject*) rv;
    } else if (flag == 4) {
        //two slices
        PyObject *slice0;
        PyObject *slice1;
        slice0 = PyTuple_GetItem(key, 0);
        slice1 = PyTuple_GetItem(key, 1);

        Py_ssize_t start0, stop0, step0;
        if (PySlice_Unpack(slice0, &start0, &stop0, &step0) < 0) {
            PyErr_SetString(PyExc_ValueError, "Invalid slice object");
            return NULL;
        }
        if (start0 >= self->mat->rows || stop0 >= self->mat->rows) {
            PyErr_SetString(PyExc_IndexError, "Index out of range");
            return NULL;
        }

        Py_ssize_t start1, stop1, step1;
        if (PySlice_Unpack(slice1, &start1, &stop1, &step1) < 0) {
            PyErr_SetString(PyExc_ValueError, "Invalid slice object");
            return NULL;
        }

        if (start1 >= self->mat->cols || stop1 >= self->mat->rows) {
            PyErr_SetString(PyExc_IndexError, "Index out of range");
            return NULL;
        }
        if (step0 != 1 || step1 != 1 || stop0 - start0 < 1 || stop1 - start1 < 1) {
            PyErr_SetString(PyExc_ValueError, "Slice info not valid");
            return NULL;
        }

        if (stop0 - start0 == 1 && stop1 - start1 == 1) {
            //return single number
            PyObject *rv = Matrix61c_get_value(self, get_shape(start0, start1));
            return (PyObject*) rv;
        } else {
            Matrix61c *rv = (Matrix61c *) Matrix61c_new(&Matrix61cType, NULL, NULL);
            flag = allocate_matrix_ref(&(rv->mat),self->mat, start0, start1, stop0 - start0, stop1 - start1);
            if (flag == -1) {
                PyErr_SetString(PyExc_IndexError, "Index out of range");
                return NULL;
            } else if (flag == -2) {
                PyErr_SetString(PyExc_RuntimeError, "Malloc fails");
                return NULL;
            }
            return (PyObject*) rv;
        }
    } else if (flag == 5) {
        //two ints
        return Matrix61c_get_value(self, key);
    } else {
        return NULL;
    }
    
}

/*
 * Given a numc.Matrix `self`, index into it with `key`, and set the indexed result to `v`.
 */
int Matrix61c_set_subscript(Matrix61c* self, PyObject *key, PyObject *v) {
    /* TODO: YOUR CODE HERE */
    int key_flag = parse_key(key);
    int v_flag = parse_v(v);
    if (key_flag < 0 || v_flag < 0) {
        return -1;
    }
    if (key_flag == 0 && (v_flag >= 0 && v_flag <= 2)) {
        //a[5]
        int row_index = PyLong_AsLong(key);
        if (v_flag == 0) {
            // a[5] = 3
            int value = PyLong_AsLong(v);
            for (int i = 0; i < self->mat->cols; i++) {
                set(self->mat, row_index, i, value);
            }
            return 0;
        } else if (v_flag == 1) {
            // a[5] = 3.3
            double value = PyFloat_AsDouble(v);
            for (int i = 0; i < self->mat->cols; i++) {
                set(self->mat, row_index, i, value);
            }
            return 0;
        } else if (v_flag == 2) {
            //a[5] = [3, 4, 5.5]
            int length = PyList_Size(v);
            if (self->mat->cols != length) {
                PyErr_SetString(PyExc_ValueError, "Range error");
                return -1;
            }
            for (int i = 0; i < length; i++) {
                PyObject *item = PyList_GetItem(v, i);
                if (PyLong_Check(item)) {
                    int value = PyLong_AsLong(item);
                    set(self->mat, row_index, i, value);
                } else if (PyFloat_Check(item)) {
                    double value = PyFloat_AsDouble(item);
                    set(self->mat, row_index, i, value);
                } else {
                    PyErr_SetString(PyExc_IndexError, "Something wrong_a[5] = [3, 4, 5.5]");
                    return -1;
                }
            }
            return 0;
        }
    } else if (key_flag == 1 && v_flag > 2) {
        //a[0:3] = [[], [], []]
        Py_ssize_t start, stop, step;
        if (PySlice_Unpack(key, &start, &stop, &step) < 0) {
            PyErr_SetString(PyExc_ValueError, "Invalid slice object");
            return -1;
        }
        if (step != 1 || stop - start < 1) {
            PyErr_SetString(PyExc_ValueError, "Slice info not valid");
            return -1;
        }
        int sub_length = PyList_Size((PyList_GetItem(v, 0)));
        if (sub_length != self->mat->cols || stop - start != v_flag - 1) {
            PyErr_SetString(PyExc_ValueError, "Given list has wrong length");
            return -1;
        }
        for (int i = start; i < stop; i++) {
            PyObject *sub_list = PyList_GetItem(v, i - start);
            for (int j = 0; i < self->mat->cols; j++) {
                PyObject *value_object = PyList_GetItem(sub_list, j);
                if (PyLong_Check(value_object)) {
                    int value = PyLong_AsLong(value_object);
                    set(self->mat, i, j, value);
                } else {
                    double value = PyFloat_AsDouble(value_object);
                    set(self->mat, i, j, value);
                }
            }
        }
        return 0;
    } else if (key_flag == 2 && (v_flag >= 0 && v_flag <= 2)) {
        //a[2, 1:3]
        int row_index = PyLong_AsLong(PyTuple_GetItem(key, 0));
        Py_ssize_t start, stop, step;
        if (PySlice_Unpack(PyTuple_GetItem(key, 1), &start, &stop, &step) < 0) {
            PyErr_SetString(PyExc_ValueError, "Invalid slice object");
            return -1;
        }
        if (step != 1 || stop - start < 1) {
            PyErr_SetString(PyExc_ValueError, "Slice info not valid");
            return -1;
        }
        if (v_flag == 0) {
            // a[2, 1:3] = 3
            int value = PyLong_AsLong(v);
            for (int i = start; i < stop; i++) {
                set(self->mat, row_index, i, value);
            }
            return 0;
        } else if (v_flag == 1) {
            // a[2, 1:3] = 3.3
            double value = PyFloat_AsDouble(v);
            for (int i = start; i < stop; i++) {
                set(self->mat, row_index, i, value);
            }
            return 0;
        } else if (v_flag == 2) {
            //a[2, 1:3] = [1, 2.4]
            int length = PyList_Size(v);
            if (stop - start != length) {
                PyErr_SetString(PyExc_ValueError, "Range error");
                return -1;
            }
            for (int i = 0; i < length; i++) {
                PyObject *item = PyList_GetItem(v, i);
                if (PyLong_Check(item)) {
                    int value = PyLong_AsLong(item);
                    set(self->mat, row_index, i, value);
                } else if (PyFloat_Check(item)) {
                    double value = PyFloat_AsDouble(item);
                    set(self->mat, row_index, i, value);
                } else {
                    PyErr_SetString(PyExc_IndexError, "Something wrong_a[2, 1:3] = [3, 4, 5.5]");
                    return -1;
                }
            }
        }
    } else if (key_flag == 3 && (v_flag >= 0 && v_flag <= 2)) {
        int col_index = PyLong_AsLong(PyTuple_GetItem(key, 1));
        Py_ssize_t start, stop, step;
        if (PySlice_Unpack(PyTuple_GetItem(key, 0), &start, &stop, &step) < 0) {
            PyErr_SetString(PyExc_ValueError, "Invalid slice object");
            return -1;
        }
        if (step != 1 || stop - start < 1) {
            PyErr_SetString(PyExc_ValueError, "Slice info not valid");
            return -1;
        }
        if (v_flag == 0) {
            // a[1:3, 2] = 3
            int value = PyLong_AsLong(v);
            for (int i = start; i < stop; i++) {
                set(self->mat, i, col_index, value);
            }
            return 0;
        } else if (v_flag == 1) {
            // a[1:3, 2] = 3.3
            double value = PyFloat_AsDouble(v);
            for (int i = start; i < stop; i++) {
                set(self->mat, i, col_index, value);
            }
            return 0;
        } else if (v_flag == 2) {
            //a[1:3, 2] = [1, 2.4]
            int length = PyList_Size(v);
            if (stop - start != length) {
                PyErr_SetString(PyExc_ValueError, "Range error");
                return -1;
            }
            for (int i = 0; i < length; i++) {
                PyObject *item = PyList_GetItem(v, i);
                if (PyLong_Check(item)) {
                    int value = PyLong_AsLong(item);
                    set(self->mat, i, col_index, value);
                } else if (PyFloat_Check(item)) {
                    double value = PyFloat_AsDouble(item);
                    set(self->mat, i, col_index, value);
                } else {
                    PyErr_SetString(PyExc_IndexError, "Something wrong_a[2, 1:3] = [3, 4, 5.5]");
                    return -1;
                }
            }
            return 0;
        }
    } else if (key_flag == 4 && (v_flag >= 0 && v_flag <= 2)) {
        //a[0:3, 2:5]
        Py_ssize_t start0, stop0, step0;
        Py_ssize_t start1, stop1, step1;
        if (PySlice_Unpack(PyTuple_GetItem(key, 0), &start0, &stop0, &step0) < 0 ||
            PySlice_Unpack(PyTuple_GetItem(key, 1), &start1, &stop1, &step1) < 0) {
            PyErr_SetString(PyExc_ValueError, "Invalid slice object");
            return -1;
        }
        if (step0 != 1 || stop0 - start0 < 1 || 
            step1 != 1 || stop1 - start1 < 1) {
            PyErr_SetString(PyExc_ValueError, "Slice info not valid");
            return -1;
        }
        if (v_flag == 0) {
            // a[0:3, 2:5] = 3
            int value = PyLong_AsLong(v);
            for (int i = start0; i < stop0; i++) {
                for (int j = 0; j < stop1; j++) {
                    set(self->mat, i, j, value);
                }
            }
            return 0;
        } else if (v_flag == 1) {
            // a[1:3, 2] = 3.3
            double value = PyFloat_AsDouble(v);
            for (int i = start0; i < stop0; i++) {
                for (int j = 0; j < stop1; j++) {
                    set(self->mat, i, j, value);
                }
            }
            return 0;
        } else if (v_flag == 2) {
            int length = PyList_Size(v);
            if (stop0 - start0 == 1 && stop1 - start1 == length) {
                //a[0:1, 2:10] = [...]
                for (int i = start1; i < stop1; i++) {
                    PyObject *item = PyList_GetItem(v, i);
                    if (PyLong_Check(item)) {
                        int value = PyLong_AsLong(item);
                        set(self->mat, start0, i, value);
                    } else if (PyFloat_Check(item)) {
                        double value = PyFloat_AsDouble(item);
                        set(self->mat, start0, i, value);
                    } else {
                        PyErr_SetString(PyExc_IndexError, "Something wrong_a[2, 1:3] = [3, 4, 5.5]");
                        return -1;
                    }
                }
                return 0;
            } else if (stop1 - start1 == 1 && stop0 - start0 == length) {
                //a[2:10, 0:1] = [...]
                for (int i = start0; i < stop0; i++) {
                    PyObject *item = PyList_GetItem(v, i);
                    if (PyLong_Check(item)) {
                        int value = PyLong_AsLong(item);
                        set(self->mat, i, start1, value);
                    } else if (PyFloat_Check(item)) {
                        double value = PyFloat_AsDouble(item);
                        set(self->mat, i, start1, value);
                    } else {
                        PyErr_SetString(PyExc_IndexError, "Something wrong_a[1:2, 1:3] = [3, 4, 5.5]");
                        return -1;
                    }
                }
                return 0;
            } else {
                PyErr_SetString(PyExc_IndexError, "Something wrong_a[1:2, 1:3] = [3, 4, 5.5]");
                return -1;
            }
        }
    } else if (key_flag == 4 && v_flag > 2) {
        Py_ssize_t start0, stop0, step0;
        Py_ssize_t start1, stop1, step1;
        if (PySlice_Unpack(PyTuple_GetItem(key, 0), &start0, &stop0, &step0) < 0 ||
            PySlice_Unpack(PyTuple_GetItem(key, 1), &start1, &stop1, &step1) < 0) {
            PyErr_SetString(PyExc_ValueError, "Invalid slice object");
            return -1;
        }
        if (step0 != 1 || stop0 - start0 < 1 || 
            step1 != 1 || stop1 - start1 < 1) {
            PyErr_SetString(PyExc_ValueError, "Slice info not valid");
            return -1;
        }
        int length = v_flag - 1;
        int sub_length = PyList_Size(PyList_GetItem(v, 0));
        if (stop0 - start0 != length || stop1 - start1 != sub_length) {
            PyErr_SetString(PyExc_IndexError, "Something wrong_a[0:2, 1:3] = [[3, 4, 5.5], [???]]");
            return -1;
        }
        for (int i = start0; i < stop0; i++) {
            PyObject *sub_list = PyList_GetItem(v, i - start0);
            for (int j = start1; j < stop1; j++) {
                PyObject *element = PyList_GetItem(sub_list, j - start1);
                if (PyLong_Check(element)) {
                    int value = PyLong_AsLong(element);
                    set(self->mat, i, j, value);
                } else if (PyFloat_Check(element)) {
                    double value = PyFloat_AsDouble(element);
                    set(self->mat, i, j, value);
                } else {
                    PyErr_SetString(PyExc_IndexError, "Something wrong_a[1:2, 1:3] = [3, 4, 5.5]");
                    return -1;
                }
            }
        }
        return 0;
    } else if (key_flag == 5 && (v_flag >= 0 && v_flag <= 2)) {
        int row = PyLong_AsLong(PyTuple_GetItem(key, 0));
        int col = PyLong_AsLong(PyTuple_GetItem(key, 1));
        if (v_flag == 0) {
            int value = PyLong_AsLong(v);
            set(self->mat, row, col, value);
        } else {
            double value = PyLong_AsLong(v);
            set(self->mat, row, col, value);
        }
        return 0;
    }
    char s[50];
    sprintf(s, "key_flag: %d, v_flag: %d", key_flag, v_flag);
    PyErr_SetString(PyExc_IndexError, s);
    return -1;
}

/*
 * Return 0 if key is a single number, 1 if key is a single slice, 2 if int and slice, 
 * 3 if slice and int, 4 for two slices, 5 for two ints, -1 if tuple size is wrong, -2 if tuple element type is wrong, 
 * -3 if key is not a tuple or an int or a slice.
 */
int parse_key(PyObject* key) {
    if (!PyTuple_Check(key)) {
        if (PyLong_Check(key)) {
            return 0;
        } else if (PySlice_Check(key)) {
            return 1;
        } else {
            return -3;
        }
    }
    Py_ssize_t size = PyTuple_Size(key);
    if (size != 2) {
        return -1;
    } else {
        PyObject *item0 = PyTuple_GetItem(key, 0);
        PyObject *item1 = PyTuple_GetItem(key, 1);
        if (PyLong_Check(item0) && PySlice_Check(item1)) {
            return 2;
        } else if (PySlice_Check(item0) && PyLong_Check(item1)) {
            return 3;
        } else if (PySlice_Check(item0) && PySlice_Check(item1)) {
            return 4;
        } else if (PyLong_Check(item0) && PyLong_Check(item1)) {
            return 5;
        } else {
            return -2;
        }
    }
}

/*
 * Return 0 if v is an int, 1 if v is a float, 2 if v is a valid 1D list (float or int), 
 * (n + 1) if v is a valid N-D(n >= 2) list, 
 * -1 if list length is less than 1, -2 if the elements of the list are not consistent
 * 
 */
int parse_v(PyObject* v) {
    if (PyLong_Check(v)) {
        return 0;
    } else if (PyFloat_Check(v)) {
        return 1;
    } else if (PyList_Check(v)) {
        Py_ssize_t length = PyList_Size(v);
        if (length < 1) {
            return -1;
        }
        PyObject *item0 = PyList_GetItem(v, 0);
        if (PyLong_Check(item0) || PyFloat_Check(item0)) {
            //[1, 2, 3, 4.4, ...]
            PyObject *item;
            for (int i = 0; i < length; i++) {
                item = PyList_GetItem(v, i);
                if (!PyLong_Check(item) && !PyFloat_Check(item)) {
                    return -2;
                }
            }
            return 2;
        } else if (PyList_Check(item0) && length >= 2) {
            //[[1, 2, 3, 4.4], [5.5, 6, 7.7, 8], [...]]
            int sub_length = PyList_Size(item0);
            for (int i = 0; i < length; i++) {
                PyObject *item = PyList_GetItem(v, i);
                int item_length = PyList_Size(item);
                if (!PyList_Check(item) || item_length != sub_length) {
                    return -2;
                }
                for (int j = 0; j < sub_length; j++) {
                    PyObject *element = PyList_GetItem(item, j);
                    if (!PyLong_Check(element) && !PyFloat_Check(element)) {
                        return -2;
                    }
                }
            }
            return length + 1;
        } else {
            return -3;
        }
    } else {
        return -4;
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