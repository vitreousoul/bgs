/* TODO: go though functions calls to the python lib, and if it requres a DECREF call, put the DECREF call in somewhere... */

#define PY_SSIZE_T_CLEAN
#include <Python.h>

// NOTE: https://www.raylib.com/
#include "./clibs/raylib.h"

#include "./gui_app.h"

const int SCREEN_WIDTH = 960;
const int SCREEN_HEIGHT = 540;
const int TARGET_FPS = 30;

#define BOARD_SIZE 8
#define BOARD_PADDING 40
const int SQUARE_SIZE_IN_PIXELS = (SCREEN_HEIGHT - (2 * BOARD_PADDING)) / 8;

double BOARD[BOARD_SIZE][BOARD_SIZE];
app_state APP_STATE;

Color PIECE_COLOR[255] = {
  ['p'] = { 200, 60, 60, 255 },
  ['P'] = { 250, 90, 90, 255 },
  ['k'] = { 90, 250, 90, 255 },
  ['K'] = { 60, 200, 60, 255 },
  ['q'] = { 90, 90, 250, 255 },
  ['Q'] = { 60, 60, 200, 255 },
  ['b'] = { 90, 250, 250, 255 },
  ['B'] = { 60, 200, 200, 255 },
  ['n'] = { 250, 250, 90, 255 },
  ['N'] = { 200, 200, 60, 255 },
  ['r'] = { 250, 90, 250, 255 },
  ['R'] = { 200, 60, 200, 255 },
};

static int ErrorMessageAndCode(const char *message, int code)
{
    puts(message);
    return code;
}

static PyObject *ErrorMessageAndPyObject(const char *message, PyObject *object)
{
    puts(message);
    return object;
}

static int InitPython(int argc, char **argv)
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

static void PrintObjectChecks(PyObject *Object)
{
    if (PyAIter_Check(Object)) printf("PyAIter_Check\n");
    if (PyCallable_Check(Object)) printf("PyCallable_Check\n");
    if (PyIndex_Check(Object)) printf("PyIndex_Check\n");
    if (PyIter_Check(Object)) printf("PyIter_Check\n");
    if (PyMapping_Check(Object)) printf("PyMapping_Check\n");
    if (PyNumber_Check(Object)) printf("PyNumber_Check\n");
    if (PySequence_Check(Object)) printf("PySequence_Check\n");
}

static PyObject *InitBoard()
{
    PyObject *BoardModule = PyImport_ImportModule("Board");
    if (!BoardModule) return ErrorMessageAndPyObject("Error initing board module\n", 0);
    PyObject *BoardClass = PyObject_GetAttrString(BoardModule, "Board");
    if (!BoardClass || !PyCallable_Check(BoardClass)) return ErrorMessageAndPyObject("BoardClass not callable\n", 0);
    PyObject *BoardInitResult = PyObject_CallNoArgs(BoardClass);
    if (!BoardInitResult) return ErrorMessageAndPyObject("Board init failed\n", 0);
    return BoardInitResult;
}

static void InitAppState()
{
    APP_STATE.MousePosition.x = -1;
    APP_STATE.MousePosition.y = -1;
    APP_STATE.MousePrimaryDown = 0;
    APP_STATE.HoverSquare.X = -1;
    APP_STATE.HoverSquare.Y = -1;
    APP_STATE.SelectedSquare.X = -1;
    APP_STATE.SelectedSquare.Y = -1;
}

static void SetSquareValue(int RowIndex, int ColIndex, double Value)
{
    if (RowIndex >= 0 && RowIndex < BOARD_SIZE && ColIndex >= 0 && ColIndex < BOARD_SIZE)
    {
        BOARD[RowIndex][ColIndex] = Value;
    }
}

static int UpdateBoardState(PyObject *Board)
{
    int Result = 0;
    PyObject *BoardState = PyObject_GetAttrString(Board, "board_state");
    if (!BoardState) return ErrorMessageAndCode("null BoardState\n", 1);
    int IsSequence = PySequence_Check(BoardState);
    if (IsSequence)
    {
        Py_ssize_t FooLength = PySequence_Length(BoardState);
        int RowIndex, ColIndex;
        for (RowIndex = 0; RowIndex < FooLength; ++RowIndex)
        {
            PyObject *Row = PySequence_GetItem(BoardState, RowIndex);
            if (Row)
            {
                Py_ssize_t FooRowLength = PySequence_Length(Row);
                for (ColIndex = 0; ColIndex < FooRowLength; ++ColIndex)
                {
                    PyObject *Value = PySequence_GetItem(Row, ColIndex);
                    int ValueIsTreatedLikeAString = PySequence_Check(Value);
                    if (ValueIsTreatedLikeAString)
                    {
                        PyObject *SquareValue = PySequence_GetItem(Value, 0);
                        double SquareValueAsDouble = SquareValue ? PyFloat_AsDouble(SquareValue) : -1.0f;
                        SetSquareValue(RowIndex, ColIndex, SquareValueAsDouble);
                    }
                }
            }
        }
    }
    return Result;
}

static int PositionInsideBoard(Vector2 Position)
{
    return (Position.x >= 0.0f && Position.x <= BOARD_SIZE &&
            Position.y >= 0.0f && Position.y <= BOARD_SIZE);
}

static Vector2 PositionToSquarePosition(Vector2 Position)
{
    Vector2 Result;
    Vector2 OffsetPosition;
    OffsetPosition.x = (Position.x - BOARD_PADDING) / SQUARE_SIZE_IN_PIXELS;
    OffsetPosition.y = (Position.y - BOARD_PADDING) / SQUARE_SIZE_IN_PIXELS;
    if(PositionInsideBoard(OffsetPosition))
    {
        Result.x = OffsetPosition.x;
        Result.y = OffsetPosition.y;
    }
    else
    {
        Result.x = -1;
        Result.y = -1;
    }
    return Result;
}

static void UpdateInput()
{
    APP_STATE.MousePosition = GetMousePosition();
    APP_STATE.MousePrimaryDown = IsMouseButtonPressed(0);
    Vector2 MouseSquarePosition = PositionToSquarePosition(APP_STATE.MousePosition);
    APP_STATE.HoverSquare.X = (int)MouseSquarePosition.x;
    APP_STATE.HoverSquare.Y = (int)MouseSquarePosition.y;
}

static void DrawBoard()
{
    int Row, Col;
    for (Row = 0; Row < BOARD_SIZE; ++Row)
    {
        for (Col = 0; Col < BOARD_SIZE; ++Col)
        {
            double SquareFloatValue = BOARD[Row][Col] > 0.0f ? BOARD[Row][Col] : 0.0f;
            // truncate float value and treat as ASCII value, we may want to be more careful about this
            int ColorIndex = (int)SquareFloatValue;
            Color SquareColor = PIECE_COLOR[ColorIndex];
            int X = (Col * SQUARE_SIZE_IN_PIXELS) + BOARD_PADDING;
            int Y = (Row * SQUARE_SIZE_IN_PIXELS) + BOARD_PADDING;
            DrawRectangle(X, Y, SQUARE_SIZE_IN_PIXELS, SQUARE_SIZE_IN_PIXELS, SquareColor);
            if (APP_STATE.HoverSquare.X == Col && APP_STATE.HoverSquare.Y == Row)
            {
                // draw outline of square if the mouse position is inside the square
                DrawRectangleLines(X, Y, SQUARE_SIZE_IN_PIXELS, SQUARE_SIZE_IN_PIXELS, BLACK);
                if (APP_STATE.MousePrimaryDown)
                {
                    APP_STATE.SelectedSquare.X = X;
                    APP_STATE.SelectedSquare.Y = Y;
                }
            }
            if (APP_STATE.SelectedSquare.X == X && APP_STATE.SelectedSquare.Y == Y)
            {
                DrawRectangle(X, Y, SQUARE_SIZE_IN_PIXELS, SQUARE_SIZE_IN_PIXELS, (Color){85,85,85,99});
                DrawRectangleLines(X, Y, SQUARE_SIZE_IN_PIXELS, SQUARE_SIZE_IN_PIXELS, (Color){20,20,20,255});
            }
        }
    }
}

int main(int argc, char **argv)
{
    InitAppState();
    int InitStatus = InitPython(argc, argv);
    if (InitStatus) return ErrorMessageAndCode("Error initing python!\n", InitStatus);
    PyObject *Board = InitBoard();
    if (!Board) return ErrorMessageAndCode("Error initing Board\n", 1);
    int BoardStateCode = UpdateBoardState(Board);
    if (BoardStateCode) return ErrorMessageAndCode("Error getting board-state!\n", BoardStateCode);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "bgs gui");
    SetTargetFPS(TARGET_FPS);
    while (!WindowShouldClose())
    {
        UpdateInput();
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawBoard();
        EndDrawing();
    }
    CloseWindow();
    return 0;
}
