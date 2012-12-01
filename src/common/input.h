#ifndef COMMON_INPUT_H
#define COMMON_INPUT_H

#define IL_INPUT_RANGE      (1)
#define IL_INPUT_KEYDOWN    ((IL_INPUT_RANGE << 8) + 0) // int
#define IL_INPUT_KEYUP      ((IL_INPUT_RANGE << 8) + 1) // int
#define IL_INPUT_MOUSEDOWN  ((IL_INPUT_RANGE << 8) + 2) // int
#define IL_INPUT_MOUSEUP    ((IL_INPUT_RANGE << 8) + 3) // int
#define IL_INPUT_MOUSEMOVE  ((IL_INPUT_RANGE << 8) + 4) // ilI_mouseMove
#define IL_INPUT_MOUSEWHEEL ((IL_INPUT_RANGE << 8) + 5) // ilI_mouseWheel
// note: the UP/DOWN functions use the glfw values.

typedef struct ilI_mouseMove {
  int x, y;
} ilI_mouseMove;

typedef struct ilI_mouseWheel {
  int x, y;
} ilI_mouseWheel;

int ilI_isKeySet(int key);
int ilI_isButtonSet(int button);

void ilI_grabMouse(int mode);

#endif
