#ifndef COMMON_INPUT_H
#define COMMON_INPUT_H

#define IL_INPUT_KEYDOWN    (3) // int
#define IL_INPUT_KEYUP      (4) // int
#define IL_INPUT_MOUSEDOWN  (5) // int
#define IL_INPUT_MOUSEUP    (6) // int
#define IL_INPUT_MOUSEMOVE  (7) // ilI_mouseMove
#define IL_INPUT_MOUSEWHEEL (8) // ilI_mouseWheel
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
