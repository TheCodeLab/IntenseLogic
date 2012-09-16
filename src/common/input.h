#ifndef COMMON_INPUT_H
#define COMMON_INPUT_H

#define IL_INPUT_RANGE      (1)
#define IL_INPUT_KEYDOWN    ((IL_INPUT_RANGE << 8) + 0) // int
#define IL_INPUT_KEYUP      ((IL_INPUT_RANGE << 8) + 1) // int
#define IL_INPUT_MOUSEDOWN  ((IL_INPUT_RANGE << 8) + 2) // int
#define IL_INPUT_MOUSEUP    ((IL_INPUT_RANGE << 8) + 3) // int
#define IL_INPUT_MOUSEMOVE  ((IL_INPUT_RANGE << 8) + 4) // il_Input_MouseMove
#define IL_INPUT_MOUSEWHEEL ((IL_INPUT_RANGE << 8) + 5) // il_Input_MouseWheel
// note: the UP/DOWN functions use the SDL return values.

typedef struct il_Input_MouseMove {
  int x, y;
} il_Input_MouseMove;

typedef struct il_Input_MouseWheel {
  int x, y;
} il_Input_MouseWheel;

int il_Input_isKeySet(int key);
int il_Input_isButtonSet(int button);

#endif
