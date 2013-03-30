#include "input.h"

#include <string.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/glfw.h>

#include "common/event.h"

void ilI_init()
{
    // not needed as of writing
}

int ilI_isKeySet(int key)
{
    return glfwGetKey(key);
}

int ilI_isButtonSet(int button)
{
    return glfwGetMouseButton(button);
}

void ilI_grabMouse(int mode)
{
    if(mode) {
        glfwDisable(GLFW_MOUSE_CURSOR);
    } else {
        glfwEnable(GLFW_MOUSE_CURSOR);
    }
}
