/*
Just playing around with embedding python in C
https://docs.python.org/3.11/c-api/index.html
*/
#define PY_SSIZE_T_CLEAN
#include <Python.h>

static double run_test_module()
{
    /* this is the code that does the stuff we care about! */
    PyObject *TestModule = PyImport_ImportModule("test");
    if (!TestModule)
    {
        printf("null TestModule\n");
        return 1;
    }
    PyObject *TestFunction = PyObject_GetAttrString(TestModule, "test_all");
    if (!TestFunction || !PyCallable_Check(TestFunction))
    {
        printf("pFunc not callable\n");
        return 1;
    }
    PyObject *Args = PyTuple_Pack(0);
    PyObject *TestResult = PyObject_CallObject(TestFunction, Args);
    double TestResultValue = PyFloat_AsDouble(TestResult);
    return TestResultValue;
}

int main(int argc, char **argv)
{
    /* copy-paste config stuff from docs */
    PyStatus status;
    PyConfig config;
    PyConfig_InitPythonConfig(&config);
    /* isolated means python cannot look at environement variables or something like that? */
    config.isolated = 1;

    /* the following config code is copy-pasta from docs, with gotos to shared cleanup code at end of function */
    /* Decode command line arguments. Implicitly preinitialize Python (in isolated mode). */
    status = PyConfig_SetBytesArgv(&config, argc, argv);
    if (PyStatus_Exception(status))
    {
        goto exception;
    }
    status = Py_InitializeFromConfig(&config);
    if (PyStatus_Exception(status))
    {
        goto exception;
    }
    PyConfig_Clear(&config);

    // For some reason when python is embedded, the current directory is not added to sys.path,
    // so we insert an empty string into sys.path, which will make python look at the current directory when importing modules.
    // We have to add the local directory so that python can find our local modules.
    PyRun_SimpleString("import sys");
    PyRun_SimpleString("sys.path.insert(0,'')");

    double TestResult = run_test_module();
    return TestResult;

exception:
    /* exception code copied from docs */
    PyConfig_Clear(&config);
    if (PyStatus_IsExit(status)) {
        return status.exitcode;
    }
    /* Display the error message and exit the process with non-zero exit code */
    Py_ExitStatusException(status);
}
