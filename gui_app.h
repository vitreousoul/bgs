typedef struct
{
    int X;
    int Y;
} ivec2;

typedef struct
{
    Vector2 MousePosition;
    int MousePrimaryDown;
    ivec2 HoverSquare;
    ivec2 SelectedSquare;
} app_state;
