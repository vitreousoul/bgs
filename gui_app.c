/* TODO: go though functions calls to the python lib, and if it requres a DECREF call, put the DECREF call in somewhere... */

#define PY_SSIZE_T_CLEAN
#include <Python.h>

// NOTE: https://www.raylib.com/
#include "./clibs/raylib.h"

#include "./gui_app.h"

PyObject *DebugBoardClass = 0; // should we just have globals of stuff we expect to use?
PyObject *Board = 0;

const int SCREEN_WIDTH = 960;
const int SCREEN_HEIGHT = 540;
const int TARGET_FPS = 30;

#define BOARD_SIZE 8
#define BOARD_PADDING 40
const int SQUARE_SIZE_IN_PIXELS = (SCREEN_HEIGHT - (2 * BOARD_PADDING)) / 8;
#define BOARD_SIZE_IN_PIXELS (BOARD_SIZE * SQUARE_SIZE_IN_PIXELS)

const Color DARK_SQUARE_COLOR = {68,100,66,255};
const Color LIGHT_SQUARE_COLOR = {128,208,140,255};
const Color SELECTED_SQUARE_COLOR = {105,85,205,59};
const Color SELECTED_SQUARE_OUTLINE_COLOR = {100,20,100,255};

double BOARD[BOARD_SIZE][BOARD_SIZE];
static app_state APP_STATE;

#define PIECE_TEXTURE_SIZE 120

char RANK_TABLE[8] = {'1','2','3','4','5','6','7','8'};
char FILE_TABLE[8] = {'a','b','c','d','e','f','g','h'};

ivec2 PIECE_TEXTURE_OFFSET[255] = {
    ['p'] = {4,0},
    ['P'] = {4,PIECE_TEXTURE_SIZE+8},
    ['n'] = {PIECE_TEXTURE_SIZE+12,0},
    ['N'] = {PIECE_TEXTURE_SIZE+12,PIECE_TEXTURE_SIZE+8},
    ['b'] = {2*PIECE_TEXTURE_SIZE+20,0},
    ['B'] = {2*PIECE_TEXTURE_SIZE+20,PIECE_TEXTURE_SIZE+8},
    ['r'] = {3*PIECE_TEXTURE_SIZE+28,0},
    ['R'] = {3*PIECE_TEXTURE_SIZE+28,PIECE_TEXTURE_SIZE+8},
    ['q'] = {4*PIECE_TEXTURE_SIZE+36,0},
    ['Q'] = {4*PIECE_TEXTURE_SIZE+36,PIECE_TEXTURE_SIZE+8},
    ['k'] = {5*PIECE_TEXTURE_SIZE+44,0},
    ['K'] = {5*PIECE_TEXTURE_SIZE+44,PIECE_TEXTURE_SIZE+8},
};

int PIECE_EXISTS[255] = {
    ['p'] = 1,
    ['P'] = 1,
    ['k'] = 1,
    ['K'] = 1,
    ['q'] = 1,
    ['Q'] = 1,
    ['b'] = 1,
    ['B'] = 1,
    ['n'] = 1,
    ['N'] = 1,
    ['r'] = 1,
    ['R'] = 1,
};

static void DecRef(PyObject *Object)
{
    if (Object) Py_DECREF(Object);
}

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

static int IVec2Equal(ivec2 A, ivec2 B)
{
    return A.X == B.X && A.Y == B.Y;
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
    DebugBoardClass = BoardClass;
    PyObject *BoardInitResult = PyObject_CallNoArgs(BoardClass);
    if (!BoardInitResult) return ErrorMessageAndPyObject("Board init failed\n", 0);
    return BoardInitResult;
}

static void InitAppState()
{
    APP_STATE.MousePosition = (Vector2){-1,-1};
    APP_STATE.MousePrimaryDown = 0;
    APP_STATE.HoverSquare = (ivec2){-1,-1};
    APP_STATE.SelectedSquare = (ivec2){-1,-1};
    APP_STATE.MoveSquare = (ivec2){-1,-1};
    APP_STATE.ChessPieceTexture = LoadTexture("./assets/chess_pieces.png");
}

static int SquareValueOnBoard(int X, int Y)
{
    return X >= 0 && X < BOARD_SIZE && Y >= 0 && Y < BOARD_SIZE;
}

static void SetSquareValue(int RowIndex, int ColIndex, double Value)
{
    if (SquareValueOnBoard(ColIndex, RowIndex))
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
                        DecRef(SquareValue);
                    }
                    DecRef(Value);
                }
            }
            DecRef(Row);
        }
    }
    DecRef(BoardState);
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
    OffsetPosition.y = BOARD_SIZE - ((Position.y - BOARD_PADDING) / SQUARE_SIZE_IN_PIXELS);
    if(PositionInsideBoard(OffsetPosition))
    {
        Result = (Vector2){OffsetPosition.x, OffsetPosition.y};
    }
    else
    {
        Result = (Vector2){-1,-1};
    }
    return Result;
}

static void UpdateInput()
{
    APP_STATE.MousePosition = GetMousePosition();
    APP_STATE.MousePrimaryDown = IsMouseButtonPressed(0);
    Vector2 MouseSquarePosition = PositionToSquarePosition(APP_STATE.MousePosition);
    APP_STATE.HoverSquare = (ivec2){(int)MouseSquarePosition.x, (int)MouseSquarePosition.y};
    int OnBoard = SquareValueOnBoard(APP_STATE.HoverSquare.X, APP_STATE.HoverSquare.Y);
    if (APP_STATE.MousePrimaryDown && OnBoard)
    {
        if (APP_STATE.SelectedSquare.X == -1 && APP_STATE.SelectedSquare.Y == -1)
        {
            printf("Selected Square %d %d\n", APP_STATE.HoverSquare.X, APP_STATE.HoverSquare.Y);
            APP_STATE.SelectedSquare = (ivec2){APP_STATE.HoverSquare.X, APP_STATE.HoverSquare.Y};
        }
        else if (!IVec2Equal(APP_STATE.SelectedSquare, APP_STATE.HoverSquare))
        {
            APP_STATE.MoveSquare = (ivec2){APP_STATE.HoverSquare.X, APP_STATE.HoverSquare.Y};
        }
    }
}

static int MakeMove(PyObject *Board, char *Move)
{
    PyObject *MoveMethod = PyObject_GetAttrString(DebugBoardClass, "move");
    if (!MoveMethod) return ErrorMessageAndCode("null MoveMethod\n", 0);
    if (!PyCallable_Check(MoveMethod)) return ErrorMessageAndCode("MoveMethod not callable\n", 1);
    PyObject *ArgValue = PyUnicode_FromString(Move);
    PyObject *Args = PyTuple_Pack(2, Board, ArgValue);
    PyObject *MoveResult = PyObject_CallObject(MoveMethod, Args);
    DecRef(MoveMethod);
    return 0;
}

static void HandleMove(PyObject *Board)
{
    int HasSelectedSquare = APP_STATE.SelectedSquare.X >= 0 && APP_STATE.SelectedSquare.Y >= 0;
    int HasMoveSquare = APP_STATE.MoveSquare.X >= 0 && APP_STATE.MoveSquare.Y >= 0;
    if (HasSelectedSquare && HasMoveSquare)
    {
        char MoveString[5] = {
            FILE_TABLE[APP_STATE.SelectedSquare.X],
            RANK_TABLE[APP_STATE.SelectedSquare.Y],
            FILE_TABLE[APP_STATE.MoveSquare.X],
            RANK_TABLE[APP_STATE.MoveSquare.Y],
            0
        };
        MakeMove(Board, MoveString);
        APP_STATE.SelectedSquare = (ivec2){-1,-1};
        APP_STATE.MoveSquare = (ivec2){-1,-1};
    }
    UpdateBoardState(Board);
}

static void DrawBoard()
{
    int Row, Col;
    for (Row = 0; Row < BOARD_SIZE; ++Row)
    {
        for (Col = 0; Col < BOARD_SIZE; ++Col)
        {
            double SquareFloatValue = BOARD[Row][Col] > 0.0f ? BOARD[Row][Col] : 0.0f;
            int SquareIntValue = (int)SquareFloatValue;
            int X = (Col * SQUARE_SIZE_IN_PIXELS) + BOARD_PADDING;
            int Y = (((BOARD_SIZE - 1) - Row) * SQUARE_SIZE_IN_PIXELS) + BOARD_PADDING;
            int IsDarkSquare = (X % 2) != (Y % 2);
            Color SquareColor = IsDarkSquare ? DARK_SQUARE_COLOR : LIGHT_SQUARE_COLOR;
            DrawRectangle(X, Y, SQUARE_SIZE_IN_PIXELS, SQUARE_SIZE_IN_PIXELS, SquareColor);
            if (PIECE_EXISTS[SquareIntValue] && IsTextureReady(APP_STATE.ChessPieceTexture))
            {
                ivec2 PieceTextureOffset = PIECE_TEXTURE_OFFSET[SquareIntValue];
                Color Tint = {255,212,255,255};
                Vector2 Origin = {0,0};
                Rectangle Source, Dest;
                Source = (Rectangle){PieceTextureOffset.X, PieceTextureOffset.Y, PIECE_TEXTURE_SIZE, PIECE_TEXTURE_SIZE};
                Dest = (Rectangle){X, Y, SQUARE_SIZE_IN_PIXELS, SQUARE_SIZE_IN_PIXELS};
                DrawTexturePro(APP_STATE.ChessPieceTexture, Source, Dest, Origin, 0.0f, Tint);
            }
            if (APP_STATE.HoverSquare.X == Col && APP_STATE.HoverSquare.Y == Row)
            {
                // draw outline of square if the mouse position is inside the square
                DrawRectangleLines(X, Y, SQUARE_SIZE_IN_PIXELS, SQUARE_SIZE_IN_PIXELS, BLACK);
            }
            if (APP_STATE.SelectedSquare.X == Col && APP_STATE.SelectedSquare.Y == Row)
            {
                DrawRectangle(X, Y, SQUARE_SIZE_IN_PIXELS, SQUARE_SIZE_IN_PIXELS, SELECTED_SQUARE_COLOR);
                DrawRectangleLines(X, Y, SQUARE_SIZE_IN_PIXELS, SQUARE_SIZE_IN_PIXELS, SELECTED_SQUARE_OUTLINE_COLOR);
            }
        }
    }
}

int main(int argc, char **argv)
{
    int InitStatus = InitPython(argc, argv);
    if (InitStatus) return ErrorMessageAndCode("Error initing python!\n", InitStatus);
    PyObject *Board = InitBoard();
    if (!Board) return ErrorMessageAndCode("Error initing Board\n", 1);
    int BoardStateCode = UpdateBoardState(Board);
    if (BoardStateCode) return ErrorMessageAndCode("Error getting board-state!\n", BoardStateCode);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "bgs gui");
    SetTargetFPS(TARGET_FPS);
    InitAppState();
    while (!WindowShouldClose())
    {
        UpdateInput();
        HandleMove(Board);
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawBoard();
        EndDrawing();
    }
    CloseWindow();
    return 0;
}
