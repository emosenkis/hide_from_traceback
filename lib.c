#include <Python.h>

/* Marker to check that pointer value was set. */
static const char uninitialized[] = "uninitialized";
#define UNINITIALIZED_PTR ((void *)uninitialized)

static PyObject *method_set_exc_info_impl(PyObject *module, PyObject *new_type,
                                     PyObject *new_value, PyObject *new_tb) {
    PyObject *type = UNINITIALIZED_PTR, *value = UNINITIALIZED_PTR, *tb = UNINITIALIZED_PTR;
    PyErr_GetExcInfo(&type, &value, &tb);

    Py_INCREF(new_type);
    Py_INCREF(new_value);
    Py_INCREF(new_tb);
    PyErr_SetExcInfo(new_type, new_value, new_tb);

    PyObject *orig_exc = PyTuple_Pack(3,
            type  ? type  : Py_None,
            value ? value : Py_None,
            tb    ? tb    : Py_None);
    Py_XDECREF(type);
    Py_XDECREF(value);
    Py_XDECREF(tb);
    return orig_exc;
}

static PyObject *
method_set_exc_info(PyObject *module, PyObject *const *args, Py_ssize_t nargs)
{
    PyObject *return_value = NULL;
    PyObject *new_type;
    PyObject *new_value;
    PyObject *new_tb;

    if (!_PyArg_CheckPositional("set_exc_info", nargs, 3, 3)) {
        goto exit;
    }
    new_type = args[0];
    new_value = args[1];
    new_tb = args[2];
    return_value = method_set_exc_info_impl(module, new_type, new_value, new_tb);

exit:
    return return_value;
}

PyDoc_STRVAR(method_set_exc_info__doc__,
"set_exc_info($module, new_type, new_value, new_tb, /)\n"
"--\n"
"\n");

// Macro to use the more powerful/dangerous C-style cast even in C++.
#define _Py_CAST(type, expr) ((type)(expr))

// Cast an function to the PyCFunction type to use it with PyMethodDef.
//
// This macro can be used to prevent compiler warnings if the first parameter
// uses a different pointer type than PyObject* (ex: METH_VARARGS and METH_O
// calling conventions).
//
// The macro can also be used for METH_FASTCALL and METH_VARARGS|METH_KEYWORDS
// calling conventions to avoid compiler warnings because the function has more
// than 2 parameters. The macro first casts the function to the
// "void func(void)" type to prevent compiler warnings.
//
// If a function is declared with the METH_NOARGS calling convention, it must
// have 2 parameters. Since the second parameter is unused, Py_UNUSED() can be
// used to prevent a compiler warning. If the function has a single parameter,
// it triggers an undefined behavior when Python calls it with 2 parameters
// (bpo-33012).
#define _PyCFunction_CAST(func) \
    _Py_CAST(PyCFunction, _Py_CAST(void(*)(void), (func)))

static PyMethodDef methods[] = {
  {"set_exc_info", _PyCFunction_CAST(method_set_exc_info), METH_FASTCALL, method_set_exc_info__doc__},
  {NULL},
};

int
_Init(PyObject *mod)
{
    if (PyModule_AddFunctions(mod, methods) < 0) {
        return -1;
    }

    return 0;
}

static PyMethodDef Methods[] = {
  {NULL, NULL} /* sentinel */
};

static struct PyModuleDef _module = {
    PyModuleDef_HEAD_INIT,
    .m_name = "_hide_from_traceback",
    .m_size = 0,
    .m_methods = Methods,
};

/* Per PEP 489, this module will not be converted to multi-phase initialization
 */

PyMODINIT_FUNC
PyInit__hide_from_traceback(void)
{
    PyObject *m;

    m = PyModule_Create(&_module);
    if (m == NULL)
        return NULL;
#ifdef Py_GIL_DISABLED
    PyUnstable_Module_SetGIL(m, Py_MOD_GIL_NOT_USED);
#endif
    if (_Init(m) < 0) {
        return NULL;
    }
    PyState_AddModule(m, &_module);
    return m;
}
