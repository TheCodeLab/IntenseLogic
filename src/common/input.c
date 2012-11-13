#include "input.h"

#include <string.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/glfw.h>

#include "common/log.h"
#include "common/event.h"

void il_Input_init() {
  // not needed as of writing
}

int il_Input_isKeySet(int key) {
  return glfwGetKey(key);
}

int il_Input_isButtonSet(int button) {
  return glfwGetMouseButton(button);
}

void il_Input_GrabMouse(int mode){
	if(mode){
		glfwDisable(GLFW_MOUSE_CURSOR);
	} else{
		glfwEnable(GLFW_MOUSE_CURSOR);
	}
}
