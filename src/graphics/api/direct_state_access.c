#include "direct_state_access.h"

/*OpenGL 1.5: New buffer commands and queries replace "Buffer" with
"NamedBuffer" in name and replace "GLenum target" parameter with
"GLuint buffer"*/

GLvoid ilG_NamedBufferDataEXT(GLuint buffer, GLsizeiptr size,
                          const GLvoid *data, GLenum usage)
{
    if (GLEW_EXT_direct_state_access) {
        glNamedBufferDataEXT(buffer, size, data, usage);
    } else {
        GLuint oldbuf;
        glGetIntegerv(GL_ARRAY_BUFFER_BINDING, (GLint*)&oldbuf);
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glBufferData(GL_ARRAY_BUFFER, size, data, usage);
        glBindBuffer(GL_ARRAY_BUFFER, oldbuf);
    }
}


