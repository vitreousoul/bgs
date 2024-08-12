#ifndef __RYN_PYTHON__
#define __RYN_PYTHON__

#define PY_SSIZE_T_CLEAN
#include <Python.h>

static int ryn_InitializePython(void);

/* NOTE: Copy-paste config stuff from docs */
static int ryn_InitializePython(void)
{
    PyStatus status;
    PyConfig config;
    PyConfig_InitPythonConfig(&config);
    /* NOTE: Isolated means python cannot look at environement variables or something like that? */
    config.isolated = 1;
    /* NOTE: The following config code is copy-pasta from docs, with gotos to shared cleanup code at end of function */

    /* Decode command line arguments. Implicitly preinitialize Python (in isolated mode). */
    status = PyConfig_SetBytesArgv(&config, 0, 0);
    if (PyStatus_Exception(status)) goto exception;
    status = Py_InitializeFromConfig(&config);
    if (PyStatus_Exception(status)) goto exception;
    PyConfig_Clear(&config);
    // For some reason when python is embedded, the current directory is not added to sys.path,
    // so we insert an empty string into sys.path, which will make python look at the current directory when importing modules.
    // We have to add the local directory so that python can find our local modules.
    PyRun_SimpleString("import sys");
    PyRun_SimpleString("sys.path.insert(0,'')");
    return 0;
exception:
    /* exception code copied from docs */
    PyConfig_Clear(&config);
    if (PyStatus_IsExit(status)) {
        return status.exitcode;
    }
    /* Display the error message and exit the process with non-zero exit code */
    Py_ExitStatusException(status);
}

#endif /* __RYN_PYTHON__ */
