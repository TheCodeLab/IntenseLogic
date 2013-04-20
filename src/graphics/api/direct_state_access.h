#ifndef ILG_DIRECT_STATE_ACCESS_H
#define ILG_DIRECT_STATE_ACCESS_H

// https://www.opengl.org/registry/specs/EXT/direct_state_access.txt

#include <GL/glew.h>

/*OpenGL 1.1: New client commands*/

GLvoid ilG_ClientAttribDefaultEXT(GLbitfield mask);
GLvoid ilG_PushClientAttribDefaultEXT(GLbitfield mask);

/*OpenGL 1.0: New matrix commands add "Matrix" prefix to name,
    drops "Matrix" suffix from name, and add initial "GLenum matrixMode"
    parameter*/

/*GLvoid ilG_MatrixLoadfEXT(GLenum matrixMode, const GLfloat *m);
GLvoid ilG_MatrixLoaddEXT(GLenum matrixMode, const GLdouble *m);

GLvoid ilG_MatrixMultfEXT(GLenum matrixMode, const GLfloat *m);
GLvoid ilG_MatrixMultdEXT(GLenum matrixMode, const GLdouble *m);

GLvoid ilG_MatrixLoadIdentityEXT(GLenum matrixMode);

GLvoid ilG_MatrixRotatefEXT(GLenum matrixMode, GLfloat angle,
                        GLfloat x, GLfloat y, GLfloat z);
GLvoid ilG_MatrixRotatedEXT(GLenum matrixMode, GLdouble angle,
                        GLdouble x, GLdouble y, GLdouble z);

GLvoid ilG_MatrixScalefEXT(GLenum matrixMode,
                       GLfloat x, GLfloat y, GLfloat z);
GLvoid ilG_MatrixScaledEXT(GLenum matrixMode,
                       GLdouble x, GLdouble y, GLdouble z);

GLvoid ilG_MatrixTranslatefEXT(GLenum matrixMode,
                           GLfloat x, GLfloat y, GLfloat z);
GLvoid ilG_MatrixTranslatedEXT(GLenum matrixMode,
                           GLdouble x, GLdouble y, GLdouble z);

GLvoid ilG_MatrixOrthoEXT(GLenum matrixMode, GLdouble l, GLdouble r,
                      GLdouble b, GLdouble t, GLdouble n, GLdouble f);
GLvoid ilG_MatrixFrustumEXT(GLenum matrixMode, GLdouble l, GLdouble r,
                        GLdouble b, GLdouble t, GLdouble n, GLdouble f);

GLvoid ilG_MatrixPushEXT(GLenum matrixMode);
GLvoid ilG_MatrixPopEXT(GLenum matrixMode);*/

/*OpenGL 1.1: New texture object commands and queries replace "Tex"
in name with "Texture" and add initial "GLuint texture" parameter*/

GLvoid ilG_TextureParameteriEXT(GLuint texture, GLenum target,
                            GLenum pname, GLint param);
GLvoid ilG_TextureParameterivEXT(GLuint texture, GLenum target,
                             GLenum pname, const GLint *param);

GLvoid ilG_TextureParameterfEXT(GLuint texture, GLenum target,
                            GLenum pname, GLfloat param);
GLvoid ilG_TextureParameterfvEXT(GLuint texture, GLenum target,
                             GLenum pname, const GLfloat *param);

GLvoid ilG_TextureImage1DEXT(GLuint texture, GLenum target,
                         GLint level, GLint internalformat,
                         GLsizei width, GLint border,
                         GLenum format, GLenum type, const GLvoid *pixels);
GLvoid ilG_TextureImage2DEXT(GLuint texture, GLenum target,
                         GLint level, GLint internalformat,
                         GLsizei width, GLsizei height, GLint border,
                         GLenum format, GLenum type, const GLvoid *pixels);

GLvoid ilG_TextureSubImage1DEXT(GLuint texture, GLenum target,
                            GLint level, GLint xoffset, GLsizei width,
                            GLenum format, GLenum type,
                            const GLvoid *pixels);
GLvoid ilG_TextureSubImage2DEXT(GLuint texture, GLenum target,
                            GLint level, GLint xoffset, GLint yoffset,
                            GLsizei width, GLsizei height,
                            GLenum format, GLenum type,
                            const GLvoid *pixels);

GLvoid ilG_CopyTextureImage1DEXT(GLuint texture, GLenum target,
                             GLint level, GLenum internalformat,
                             GLint x, GLint y, GLsizei width, GLint border);
GLvoid ilG_CopyTextureImage2DEXT(GLuint texture, GLenum target,
                             GLint level, GLenum internalformat,
                             GLint x, GLint y,
                             GLsizei width, GLsizei height, GLint border);

GLvoid ilG_CopyTextureSubImage1DEXT(GLuint texture, GLenum target,
                                GLint level, GLint xoffset,
                                GLint x, GLint y, GLsizei width);
GLvoid ilG_CopyTextureSubImage2DEXT(GLuint texture, GLenum target,
                                GLint level, GLint xoffset, GLint yoffset,
                                GLint x, GLint y,
                                GLsizei width, GLsizei height);

GLvoid ilG_GetTextureImageEXT(GLuint texture, GLenum target,
                          GLint level,
                          GLenum format, GLenum type, GLvoid *pixels);

GLvoid ilG_GetTextureParameterfvEXT(GLuint texture, GLenum target,
                                GLenum pname, GLfloat *params);
GLvoid ilG_GetTextureParameterivEXT(GLuint texture, GLenum target,
                                GLenum pname, GLint *params);

GLvoid ilG_GetTextureLevelParameterfvEXT(GLuint texture, GLenum target,
                                     GLint level,
                                     GLenum pname, GLfloat *params);
GLvoid ilG_GetTextureLevelParameterivEXT(GLuint texture, GLenum target,
                                     GLint level,
                                     GLenum pname, GLint *params);

/*OpenGL 1.2: New 3D texture object commands replace "Tex" in name with
"Texture" and adds initial "GLuint texture" parameter*/

GLvoid ilG_TextureImage3DEXT(GLuint texture, GLenum target,
                         GLint level, GLint internalformat,
                         GLsizei width, GLsizei height, GLsizei depth,
                         GLint border,
                         GLenum format, GLenum type, const GLvoid *pixels);

GLvoid ilG_TextureSubImage3DEXT(GLuint texture, GLenum target,
                            GLint level,
                            GLint xoffset, GLint yoffset, GLint zoffset,
                            GLsizei width, GLsizei height, GLsizei depth,
                            GLenum format, GLenum type,
                            const GLvoid *pixels);
GLvoid ilG_CopyTextureSubImage3DEXT(GLuint texture, GLenum target,
                                GLint level,
                                GLint xoffset, GLint yoffset, GLint zoffset,
                                GLint x, GLint y,
                                GLsizei width, GLsizei height);

/*OpenGL 1.2.1: New multitexture commands and queries prefix "Multi"
before "Tex" and add an initial "GLenum texunit" parameter (to identify
the texture unit).*/

GLvoid ilG_BindMultiTextureEXT(GLenum texunit, GLenum target, GLuint texture);

GLvoid ilG_MultiTexCoordPointerEXT(GLenum texunit, GLint size, GLenum type,
                               GLsizei stride, const GLvoid *pointer);

GLvoid ilG_MultiTexEnvfEXT(GLenum texunit, GLenum target, GLenum pname,
                       GLfloat param);
GLvoid ilG_MultiTexEnvfvEXT(GLenum texunit, GLenum target, GLenum pname,
                        const GLfloat *params);
GLvoid ilG_MultiTexEnviEXT(GLenum texunit, GLenum target, GLenum pname,
                       GLint param);
GLvoid ilG_MultiTexEnvivEXT(GLenum texunit, GLenum target, GLenum pname,
                        const GLint *params);

GLvoid ilG_MultiTexGendEXT(GLenum texunit, GLenum coord, GLenum pname,
                       GLdouble param);
GLvoid ilG_MultiTexGendvEXT(GLenum texunit, GLenum coord, GLenum pname,
                        const GLdouble *params);
GLvoid ilG_MultiTexGenfEXT(GLenum texunit, GLenum coord, GLenum pname,
                       GLfloat param);
GLvoid ilG_MultiTexGenfvEXT(GLenum texunit, GLenum coord, GLenum pname,
                        const GLfloat *params);
GLvoid ilG_MultiTexGeniEXT(GLenum texunit, GLenum coord, GLenum pname,
                       GLint param);
GLvoid ilG_MultiTexGenivEXT(GLenum texunit, GLenum coord, GLenum pname,
                        const GLint *params);

GLvoid ilG_GetMultiTexEnvfvEXT(GLenum texunit, GLenum target, GLenum pname,
                           GLfloat *params);
GLvoid ilG_GetMultiTexEnvivEXT(GLenum texunit, GLenum target, GLenum pname,
                           GLint *params);

GLvoid ilG_GetMultiTexGendvEXT(GLenum texunit, GLenum coord, GLenum pname,
                           GLdouble *params);
GLvoid ilG_GetMultiTexGenfvEXT(GLenum texunit, GLenum coord, GLenum pname,
                           GLfloat *params);
GLvoid ilG_GetMultiTexGenivEXT(GLenum texunit, GLenum coord, GLenum pname,
                           GLint *params);

GLvoid ilG_MultiTexParameteriEXT(GLenum texunit, GLenum target,
                             GLenum pname, GLint param);
GLvoid ilG_MultiTexParameterivEXT(GLenum texunit, GLenum target,
                              GLenum pname, const GLint *param);

GLvoid ilG_MultiTexParameterfEXT(GLenum texunit, GLenum target,
                             GLenum pname, GLfloat param);
GLvoid ilG_MultiTexParameterfvEXT(GLenum texunit, GLenum target,
                              GLenum pname, const GLfloat *param);

GLvoid ilG_MultiTexImage1DEXT(GLenum texunit, GLenum target,
                          GLint level, GLint internalformat,
                          GLsizei width, GLint border,
                          GLenum format, GLenum type, const GLvoid *pixels);
GLvoid ilG_MultiTexImage2DEXT(GLenum texunit, GLenum target,
                          GLint level, GLint internalformat,
                          GLsizei width, GLsizei height, GLint border,
                          GLenum format, GLenum type, const GLvoid *pixels);

GLvoid ilG_MultiTexSubImage1DEXT(GLenum texunit, GLenum target,
                             GLint level, GLint xoffset, GLsizei width,
                             GLenum format, GLenum type,
                             const GLvoid *pixels);
GLvoid ilG_MultiTexSubImage2DEXT(GLenum texunit, GLenum target,
                             GLint level, GLint xoffset, GLint yoffset,
                             GLsizei width, GLsizei height,
                             GLenum format, GLenum type,
                             const GLvoid *pixels);

GLvoid ilG_CopyMultiTexImage1DEXT(GLenum texunit, GLenum target,
                              GLint level, GLenum internalformat,
                              GLint x, GLint y, GLsizei width, GLint border);
GLvoid ilG_CopyMultiTexImage2DEXT(GLenum texunit, GLenum target,
                              GLint level, GLenum internalformat,
                              GLint x, GLint y,
                              GLsizei width, GLsizei height, GLint border);

GLvoid ilG_CopyMultiTexSubImage1DEXT(GLenum texunit, GLenum target,
                                 GLint level, GLint xoffset,
                                 GLint x, GLint y, GLsizei width);
GLvoid ilG_CopyMultiTexSubImage2DEXT(GLenum texunit, GLenum target,
                                 GLint level, GLint xoffset, GLint yoffset,
                                 GLint x, GLint y,
                                 GLsizei width, GLsizei height);

GLvoid ilG_GetMultiTexImageEXT(GLenum texunit, GLenum target,
                           GLint level,
                           GLenum format, GLenum type, GLvoid *pixels);

GLvoid ilG_GetMultiTexParameterfvEXT(GLenum texunit, GLenum target,
                                 GLenum pname, GLfloat *params);
GLvoid ilG_GetMultiTexParameterivEXT(GLenum texunit, GLenum target,
                                 GLenum pname, GLint *params);

GLvoid ilG_GetMultiTexLevelParameterfvEXT(GLenum texunit, GLenum target,
                                      GLint level,
                                      GLenum pname, GLfloat *params);
GLvoid ilG_GetMultiTexLevelParameterivEXT(GLenum texunit, GLenum target,
                                      GLint level,
                                      GLenum pname, GLint *params);

GLvoid ilG_MultiTexImage3DEXT(GLenum texunit, GLenum target,
                          GLint level, GLint internalformat,
                          GLsizei width, GLsizei height, GLsizei depth,
                          GLint border,
                          GLenum format, GLenum type, const GLvoid *pixels);

GLvoid ilG_MultiTexSubImage3DEXT(GLenum texunit, GLenum target,
                             GLint level,
                             GLint xoffset, GLint yoffset, GLint zoffset,
                             GLsizei width, GLsizei height, GLsizei depth,
                             GLenum format, GLenum type,
                             const GLvoid *pixels);
GLvoid ilG_CopyMultiTexSubImage3DEXT(GLenum texunit, GLenum target,
                                 GLint level,
                                 GLint xoffset, GLint yoffset, GLint zoffset,
                                 GLint x, GLint y,
                                 GLsizei width, GLsizei height);

/*OpenGL 1.2.1: New indexed texture commands and queries append
"Indexed" to name and add "GLuint index" parameter (to identify the
texture unit index) after state name parameters (if any) and before
state value parameters*/

GLvoid ilG_EnableClientStateIndexedEXT(GLenum array, GLuint index);
GLvoid ilG_DisableClientStateIndexedEXT(GLenum array, GLuint index);

/*OpenGL 3.0: New indexed texture commands and queries append "i"
to name and add "GLuint index" parameter (to identify the texture
unit index) after state name parameters (if any) and before state
value parameters*/

GLvoid ilG_EnableClientStateiEXT(GLenum array, GLuint index);
GLvoid ilG_DisableClientStateiEXT(GLenum array, GLuint index);

/*OpenGL 1.2.1: New indexed generic queries (added for indexed texture
state) append "Indexed" to name and add "GLuint index" parameter
(to identify the texture unit) after state name parameters (if any) and
before state value parameters*/

GLvoid ilG_GetFloatIndexedvEXT(GLenum target, GLuint index, GLfloat *params);
GLvoid ilG_GetDoubleIndexedvEXT(GLenum target, GLuint index, GLdouble *params);

GLvoid ilG_GetPointerIndexedvEXT(GLenum target, GLuint index, GLvoid **params);

/*OpenGL 3.0: New indexed generic queries (added for indexed texture
state) replace "v" for "i_v" to name and add "GLuint index" parameter
(to identify the texture unit) after state name parameters (if any)
and before state value parameters*/

GLvoid ilG_GetFloati_vEXT(GLenum pname, GLuint index, GLfloat *params);
GLvoid ilG_GetDoublei_vEXT(GLenum pname, GLuint index, GLdouble *params);

GLvoid ilG_GetPointeri_vEXT(GLenum pname, GLuint index, GLvoid **params);

/*OpenGL 1.2.1:  Extend the functionality of these EXT_draw_buffers2
commands and queries for multitexture*/

GLvoid ilG_EnableIndexedEXT(GLenum cap, GLuint index);
GLvoid ilG_DisableIndexedEXT(GLenum cap, GLuint index);

GLboolean IsEnabledIndexedEXT(GLenum target, GLuint index);

GLvoid ilG_GetIntegerIndexedvEXT(GLenum target, GLuint index, GLint *params);
GLvoid ilG_GetBooleanIndexedvEXT(GLenum target, GLuint index,
                             GLboolean *params);

/*ARB_vertex_program: New program commands and queries add "Named"
prefix to name and adds initial "GLuint program" parameter*/

GLvoid ilG_NamedProgramStringEXT(GLuint program, GLenum target, GLenum format,
                             GLsizei len, const GLvoid *string);

GLvoid ilG_NamedProgramLocalParameter4dEXT(GLuint program, GLenum target,
                                       GLuint index,
                                       GLdouble x, GLdouble y,
                                       GLdouble z, GLdouble w);
GLvoid ilG_NamedProgramLocalParameter4dvEXT(GLuint program, GLenum target,
                                        GLuint index,
                                        const GLdouble *params);
GLvoid ilG_NamedProgramLocalParameter4fEXT(GLuint program, GLenum target,
                                       GLuint index,
                                       GLfloat x, GLfloat y,
                                       GLfloat z, GLfloat w);
GLvoid ilG_NamedProgramLocalParameter4fvEXT(GLuint program, GLenum target,
                                        GLuint index,
                                        const GLfloat *params);

GLvoid ilG_GetNamedProgramLocalParameterdvEXT(GLuint program, GLenum target,
        GLuint index,
        GLdouble *params);
GLvoid ilG_GetNamedProgramLocalParameterfvEXT(GLuint program, GLenum target,
        GLuint index,
        GLfloat *params);
GLvoid ilG_GetNamedProgramivEXT(GLuint program, GLenum target,
                            GLenum pname, GLint *params);
GLvoid ilG_GetNamedProgramStringEXT(GLuint program, GLenum target,
                                GLenum pname, GLvoid *string);

/*OpenGL 1.3: New compressed texture object commands replace "Tex"
in name with "Texture" and add initial "GLuint texture" parameter*/

GLvoid ilG_CompressedTextureImage3DEXT(GLuint texture, GLenum target, GLint level,
                                   GLenum internalformat,
                                   GLsizei width, GLsizei height,
                                   GLsizei depth, GLint border,
                                   GLsizei imageSize, const GLvoid *data);
GLvoid ilG_CompressedTextureImage2DEXT(GLuint texture, GLenum target, GLint level,
                                   GLenum internalformat,
                                   GLsizei width, GLsizei height,
                                   GLint border, GLsizei imageSize,
                                   const GLvoid *data);
GLvoid ilG_CompressedTextureImage1DEXT(GLuint texture, GLenum target, GLint level,
                                   GLenum internalformat,
                                   GLsizei width, GLint border,
                                   GLsizei imageSize, const GLvoid *data);
GLvoid ilG_CompressedTextureSubImage3DEXT(GLuint texture, GLenum target, GLint level,
                                      GLint xoffset, GLint yoffset,
                                      GLint zoffset,
                                      GLsizei width, GLsizei height,
                                      GLsizei depth, GLenum format,
                                      GLsizei imageSize, const GLvoid *data);
GLvoid ilG_CompressedTextureSubImage2DEXT(GLuint texture, GLenum target, GLint level,
                                      GLint xoffset, GLint yoffset,
                                      GLsizei width, GLsizei height,
                                      GLenum format,
                                      GLsizei imageSize, const GLvoid *data);
GLvoid ilG_CompressedTextureSubImage1DEXT(GLuint texture, GLenum target, GLint level,
                                      GLint xoffset, GLsizei width,
                                      GLenum format,
                                      GLsizei imageSize, const GLvoid *data);

GLvoid ilG_GetCompressedTextureImageEXT(GLuint texture, GLenum target,
                                    GLint level, GLvoid *img);

/*OpenGL 1.3: New multitexture compressed texture commands and queries
prefix "Multi" before "Tex" and add an initial "GLenum texunit"
parameter (to identify the texture unit).*/

GLvoid ilG_CompressedMultiTexImage3DEXT(GLenum texunit, GLenum target, GLint level,
                                    GLenum internalformat,
                                    GLsizei width, GLsizei height,
                                    GLsizei depth, GLint border,
                                    GLsizei imageSize, const GLvoid *data);
GLvoid ilG_CompressedMultiTexImage2DEXT(GLenum texunit, GLenum target, GLint level,
                                    GLenum internalformat,
                                    GLsizei width, GLsizei height,
                                    GLint border, GLsizei imageSize,
                                    const GLvoid *data);
GLvoid ilG_CompressedMultiTexImage1DEXT(GLenum texunit, GLenum target, GLint level,
                                    GLenum internalformat,
                                    GLsizei width, GLint border,
                                    GLsizei imageSize, const GLvoid *data);
GLvoid ilG_CompressedMultiTexSubImage3DEXT(GLenum texunit, GLenum target, GLint level,
                                       GLint xoffset, GLint yoffset,
                                       GLint zoffset,
                                       GLsizei width, GLsizei height,
                                       GLsizei depth, GLenum format,
                                       GLsizei imageSize, const GLvoid *data);
GLvoid ilG_CompressedMultiTexSubImage2DEXT(GLenum texunit, GLenum target, GLint level,
                                       GLint xoffset, GLint yoffset,
                                       GLsizei width, GLsizei height,
                                       GLenum format,
                                       GLsizei imageSize, const GLvoid *data);
GLvoid ilG_CompressedMultiTexSubImage1DEXT(GLenum texunit, GLenum target, GLint level,
                                       GLint xoffset, GLsizei width,
                                       GLenum format,
                                       GLsizei imageSize, const GLvoid *data);

GLvoid ilG_GetCompressedMultiTexImageEXT(GLenum texunit, GLenum target,
                                     GLint level, GLvoid *img);

/*OpenGL 1.3: New transpose matrix commands add "Matrix" suffix
to name, drops "Matrix" suffix from name, and add initial "GLenum
matrixMode" parameter*/

/*GLvoid ilG_MatrixLoadTransposefEXT(GLenum matrixMode, const GLfloat *m);
GLvoid ilG_MatrixLoadTransposedEXT(GLenum matrixMode, const GLdouble *m);

GLvoid ilG_MatrixMultTransposefEXT(GLenum matrixMode, const GLfloat *m);
GLvoid ilG_MatrixMultTransposedEXT(GLenum matrixMode, const GLdouble *m);*/

/*OpenGL 1.5: New buffer commands and queries replace "Buffer" with
"NamedBuffer" in name and replace "GLenum target" parameter with
"GLuint buffer"*/

GLvoid ilG_NamedBufferDataEXT(GLuint buffer, GLsizeiptr size,
                          const GLvoid *data, GLenum usage);
GLvoid ilG_NamedBufferSubDataEXT(GLuint buffer, GLintptr offset,
                             GLsizeiptr size, const GLvoid *data);
GLvoid* MapNamedBufferEXT(GLuint buffer, GLenum access);
GLboolean UnmapNamedBufferEXT(GLuint buffer);
GLvoid ilG_GetNamedBufferParameterivEXT(GLuint buffer,
                                    GLenum pname, GLint *params);
GLvoid ilG_GetNamedBufferPointervEXT(GLuint buffer,
                                 GLenum pname, GLvoid* *params);
GLvoid ilG_GetNamedBufferSubDataEXT(GLuint buffer,
                                GLintptr offset, GLsizeiptr size, GLvoid *data);

/*OpenGL 2.0: New uniform commands add "Program" prefix to name and
add initial "GLuint program" parameter*/

GLvoid ilG_ProgramUniform1fEXT(GLuint program, GLint location, GLfloat v0);
GLvoid ilG_ProgramUniform2fEXT(GLuint program, GLint location,
                           GLfloat v0, GLfloat v1);
GLvoid ilG_ProgramUniform3fEXT(GLuint program, GLint location,
                           GLfloat v0, GLfloat v1, GLfloat v2);
GLvoid ilG_ProgramUniform4fEXT(GLuint program, GLint location,
                           GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);

GLvoid ilG_ProgramUniform1iEXT(GLuint program, GLint location, GLint v0);
GLvoid ilG_ProgramUniform2iEXT(GLuint program, GLint location,
                           GLint v0, GLint v1);
GLvoid ilG_ProgramUniform3iEXT(GLuint program, GLint location,
                           GLint v0, GLint v1, GLint v2);
GLvoid ilG_ProgramUniform4iEXT(GLuint program, GLint location,
                           GLint v0, GLint v1, GLint v2, GLint v3);

GLvoid ilG_ProgramUniform1fvEXT(GLuint program, GLint location,
                            GLsizei count, const GLfloat *value);
GLvoid ilG_ProgramUniform2fvEXT(GLuint program, GLint location,
                            GLsizei count, const GLfloat *value);
GLvoid ilG_ProgramUniform3fvEXT(GLuint program, GLint location,
                            GLsizei count, const GLfloat *value);
GLvoid ilG_ProgramUniform4fvEXT(GLuint program, GLint location,
                            GLsizei count, const GLfloat *value);

GLvoid ilG_ProgramUniform1ivEXT(GLuint program, GLint location,
                            GLsizei count, const GLint *value);
GLvoid ilG_ProgramUniform2ivEXT(GLuint program, GLint location,
                            GLsizei count, const GLint *value);
GLvoid ilG_ProgramUniform3ivEXT(GLuint program, GLint location,
                            GLsizei count, const GLint *value);
GLvoid ilG_ProgramUniform4ivEXT(GLuint program, GLint location,
                            GLsizei count, const GLint *value);

GLvoid ilG_ProgramUniformMatrix2fvEXT(GLuint program, GLint location,
                                  GLsizei count, GLboolean transpose,
                                  const GLfloat *value);
GLvoid ilG_ProgramUniformMatrix3fvEXT(GLuint program, GLint location,
                                  GLsizei count, GLboolean transpose,
                                  const GLfloat *value);
GLvoid ilG_ProgramUniformMatrix4fvEXT(GLuint program, GLint location,
                                  GLsizei count, GLboolean transpose,
                                  const GLfloat *value);

/*OpenGL 2.1: New uniform matrix commands add "Program" prefix to
name and add initial "GLuint program" parameter*/

GLvoid ilG_ProgramUniformMatrix2x3fvEXT(GLuint program, GLint location,
                                    GLsizei count, GLboolean transpose,
                                    const GLfloat *value);
GLvoid ilG_ProgramUniformMatrix3x2fvEXT(GLuint program, GLint location,
                                    GLsizei count, GLboolean transpose,
                                    const GLfloat *value);
GLvoid ilG_ProgramUniformMatrix2x4fvEXT(GLuint program, GLint location,
                                    GLsizei count, GLboolean transpose,
                                    const GLfloat *value);
GLvoid ilG_ProgramUniformMatrix4x2fvEXT(GLuint program, GLint location,
                                    GLsizei count, GLboolean transpose,
                                    const GLfloat *value);
GLvoid ilG_ProgramUniformMatrix3x4fvEXT(GLuint program, GLint location,
                                    GLsizei count, GLboolean transpose,
                                    const GLfloat *value);
GLvoid ilG_ProgramUniformMatrix4x3fvEXT(GLuint program, GLint location,
                                    GLsizei count, GLboolean transpose,
                                    const GLfloat *value);

/*EXT_texture_buffer_object:  New texture buffer object command
replaces "Tex" in name with "Texture" and adds initial "GLuint texture"
parameter*/

GLvoid ilG_TextureBufferEXT(GLuint texture, GLenum target,
                        GLenum internalformat, GLuint buffer);

/*EXT_texture_buffer_object: New multitexture texture buffer command
prefixes "Multi" before "Tex" and add an initial "GLenum texunit"
parameter (to identify the texture unit).*/

GLvoid ilG_MultiTexBufferEXT(GLenum texunit, GLenum target,
                         GLenum internalformat, GLuint buffer);

/*EXT_texture_integer: New integer texture object commands and queries
replace "Tex" in name with "Texture" and add initial "GLuint texture"
parameter*/

GLvoid ilG_TextureParameterIivEXT(GLuint texture, GLenum target,
                              GLenum pname, const GLint *params);
GLvoid ilG_TextureParameterIuivEXT(GLuint texture, GLenum target,
                               GLenum pname, const GLuint *params);

GLvoid ilG_GetTextureParameterIivEXT(GLuint texture, GLenum target,
                                 GLenum pname, GLint *params);
GLvoid ilG_GetTextureParameterIuivEXT(GLuint texture, GLenum target,
                                  GLenum pname, GLuint *params);

/*EXT_texture_integer: New multitexture integer texture commands and
queries prefix "Multi" before "Tex" and add an initial "GLenum texunit"
parameter (to identify the texture unit).*/

GLvoid ilG_MultiTexParameterIivEXT(GLenum texunit, GLenum target,
                               GLenum pname, const GLint *params);
GLvoid ilG_MultiTexParameterIuivEXT(GLenum texunit, GLenum target,
                                GLenum pname, const GLuint *params);

GLvoid ilG_GetMultiTexParameterIivEXT(GLenum texunit, GLenum target,
                                  GLenum pname, GLint *params);
GLvoid ilG_GetMultiTexParameterIuivEXT(GLenum texunit, GLenum target,
                                   GLenum pname, GLuint *params);

/*EXT_gpu_shader4: New integer uniform commands add "Program" prefix
to name and add initial "GLuint program" parameter*/

GLvoid ilG_ProgramUniform1uiEXT(GLuint program, GLint location, GLuint v0);
GLvoid ilG_ProgramUniform2uiEXT(GLuint program, GLint location,
                            GLuint v0, GLuint v1);
GLvoid ilG_ProgramUniform3uiEXT(GLuint program, GLint location,
                            GLuint v0, GLuint v1, GLuint v2);
GLvoid ilG_ProgramUniform4uiEXT(GLuint program, GLint location,
                            GLuint v0, GLuint v1, GLuint v2, GLuint v3);

GLvoid ilG_ProgramUniform1uivEXT(GLuint program, GLint location,
                             GLsizei count, const GLuint *value);
GLvoid ilG_ProgramUniform2uivEXT(GLuint program, GLint location,
                             GLsizei count, const GLuint *value);
GLvoid ilG_ProgramUniform3uivEXT(GLuint program, GLint location,
                             GLsizei count, const GLuint *value);
GLvoid ilG_ProgramUniform4uivEXT(GLuint program, GLint location,
                             GLsizei count, const GLuint *value);

/*EXT_gpu_program_parameters: New program command adds "Named" prefix
to name and adds "GLuint program" parameter*/

GLvoid ilG_NamedProgramLocalParameters4fvEXT(GLuint program, GLenum target,
        GLuint index, GLsizei count,
        const GLfloat *params);

/*NV_gpu_program4: New program commands and queries add "Named"
prefix to name and replace "GLenum target" with "GLuint program"*/

GLvoid ilG_NamedProgramLocalParameterI4iEXT(GLuint program, GLenum target,
                                        GLuint index,
                                        GLint x, GLint y,
                                        GLint z, GLint w);
GLvoid ilG_NamedProgramLocalParameterI4ivEXT(GLuint program, GLenum target,
        GLuint index,
        const GLint *params);
GLvoid ilG_NamedProgramLocalParametersI4ivEXT(GLuint program, GLenum target,
        GLuint index, GLsizei count,
        const GLint *params);
GLvoid ilG_NamedProgramLocalParameterI4uiEXT(GLuint program, GLenum target,
        GLuint index,
        GLuint x, GLuint y,
        GLuint z, GLuint w);
GLvoid ilG_NamedProgramLocalParameterI4uivEXT(GLuint program, GLenum target,
        GLuint index,
        const GLuint *params);
GLvoid ilG_NamedProgramLocalParametersI4uivEXT(GLuint program, GLenum target,
        GLuint index, GLsizei count,
        const GLuint *params);

GLvoid ilG_GetNamedProgramLocalParameterIivEXT(GLuint program, GLenum target,
        GLuint index,
        GLint *params);
GLvoid ilG_GetNamedProgramLocalParameterIuivEXT(GLuint program, GLenum target,
        GLuint index,
        GLuint *params);

/*OpenGL 3.0: New renderbuffer commands add "Named" prefix to name
and replace "GLenum target" with "GLuint renderbuffer"*/

GLvoid ilG_NamedRenderbufferStorageEXT(GLuint renderbuffer,
                                   GLenum internalformat,
                                   GLsizei width, GLsizei height);

GLvoid ilG_GetNamedRenderbufferParameterivEXT(GLuint renderbuffer,
        GLenum pname, GLint *params);

/*OpenGL 3.0: New renderbuffer commands add "Named"
prefix to name and replace "GLenum target" with "GLuint renderbuffer"*/

GLvoid ilG_NamedRenderbufferStorageMultisampleEXT(GLuint renderbuffer,
        GLsizei samples,
        GLenum internalformat,
        GLsizei width, GLsizei height);

/*NV_framebuffer_multisample_coverage: New renderbuffer commands
add "Named" prefix to name and replace "GLenum target" with "GLuint
renderbuffer"*/

GLvoid ilG_NamedRenderbufferStorageMultisampleCoverageEXT(GLuint renderbuffer,
        GLsizei coverageSamples,
        GLsizei colorSamples,
        GLenum internalformat,
        GLsizei width,
        GLsizei height);

/*OpenGL 3.0: New framebuffer commands add "Named" prefix to name
and replace "GLenum target" with "GLuint framebuffer"*/

GLenum CheckNamedFramebufferStatusEXT(GLuint framebuffer,
                                      GLenum target);

GLvoid ilG_NamedFramebufferTexture1DEXT(GLuint framebuffer,
                                    GLenum attachment,
                                    GLenum textarget, GLuint texture,
                                    GLint level);
GLvoid ilG_NamedFramebufferTexture2DEXT(GLuint framebuffer,
                                    GLenum attachment,
                                    GLenum textarget, GLuint texture,
                                    GLint level);
GLvoid ilG_NamedFramebufferTexture3DEXT(GLuint framebuffer,
                                    GLenum attachment,
                                    GLenum textarget, GLuint texture,
                                    GLint level, GLint zoffset);

GLvoid ilG_NamedFramebufferRenderbufferEXT(GLuint framebuffer,
                                       GLenum attachment,
                                       GLenum renderbuffertarget,
                                       GLuint renderbuffer);

GLvoid ilG_GetNamedFramebufferAttachmentParameterivEXT(GLuint framebuffer,
        GLenum attachment,
        GLenum pname,
        GLint *params);

/*OpenGL 3.0: New texture commands add "Texture" within name and
replace "GLenum target" with "GLuint texture"*/

GLvoid ilG_GenerateTextureMipmapEXT(GLuint texture, GLenum target);

/*OpenGL 3.0: New texture commands add "MultiTex" within name and
replace "GLenum target" with "GLenum texunit"*/

GLvoid ilG_GenerateMultiTexMipmapEXT(GLenum texunit, GLenum target);

/*OpenGL 3.0: New framebuffer commands*/

GLvoid ilG_FramebufferDrawBufferEXT(GLuint framebuffer, GLenum mode);
GLvoid ilG_FramebufferDrawBuffersEXT(GLuint framebuffer, GLsizei n,
                                 const GLenum *bufs);
GLvoid ilG_FramebufferReadBufferEXT(GLuint framebuffer, GLenum mode);

/*OpenGL 3.0: New framebuffer query*/

GLvoid ilG_GetFramebufferParameterivEXT(GLuint framebuffer, GLenum pname,
                                    GLint *param);

/*OpenGL 3.0: New buffer data copy command*/
GLvoid ilG_NamedCopyBufferSubDataEXT(GLuint readBuffer, GLuint writeBuffer,
                                 GLintptr readOffset, GLintptr writeOffset,
                                 GLsizeiptr size);

/*EXT_geometry_shader4 or NV_gpu_program4: New framebuffer commands
add "Named" prefix to name and replace "GLenum target" with "GLuint
framebuffer"*/

GLvoid ilG_NamedFramebufferTextureEXT(GLuint framebuffer, GLenum attachment,
                                  GLuint texture, GLint level);
GLvoid ilG_NamedFramebufferTextureLayerEXT(GLuint framebuffer,
                                       GLenum attachment,
                                       GLuint texture,
                                       GLint level, GLint layer);
GLvoid ilG_NamedFramebufferTextureFaceEXT(GLuint framebuffer, GLenum attachment,
                                      GLuint texture,
                                      GLint level, GLenum face);

/*NV_explicit_multisample:  New texture renderbuffer object command
replaces "Tex" in name with "Texture" and add initial "GLuint texture"
parameter*/

GLvoid ilG_TextureRenderbufferEXT(GLuint texture, GLenum target,
                              GLuint renderbuffer);

/*NV_explicit_multisample: New multitexture texture renderbuffer command
prefixes "Multi" before "Tex" and add an initial "GLenum texunit"
parameter (to identify the texture unit)*/

GLvoid ilG_MultiTexRenderbufferEXT(GLenum texunit, GLenum target,
                               GLuint renderbuffer);

/*OpenGL 3.0: New vertex array specification commands for vertex
array objects prefix "VertexArray", add initial "GLuint vaobj" and
"GLuint buffer" parameters, change "Pointer" suffix to "Offset",
and change the final parameter from "const GLvoid *" to "GLintptr offset"*/

GLvoid ilG_VertexArrayVertexOffsetEXT(GLuint vaobj, GLuint buffer,
                                  GLint size, GLenum type, GLsizei stride,
                                  GLintptr offset);
GLvoid ilG_VertexArrayColorOffsetEXT(GLuint vaobj, GLuint buffer,
                                 GLint size, GLenum type, GLsizei stride,
                                 GLintptr offset);
GLvoid ilG_VertexArrayEdgeFlagOffsetEXT(GLuint vaobj, GLuint buffer,
                                    GLsizei stride, GLintptr offset);
GLvoid ilG_VertexArrayIndexOffsetEXT(GLuint vaobj, GLuint buffer,
                                 GLenum type, GLsizei stride,
                                 GLintptr offset);
GLvoid ilG_VertexArrayNormalOffsetEXT(GLuint vaobj, GLuint buffer,
                                  GLenum type, GLsizei stride,
                                  GLintptr offset);
GLvoid ilG_VertexArrayTexCoordOffsetEXT(GLuint vaobj, GLuint buffer,
                                    GLint size, GLenum type, GLsizei stride,
                                    GLintptr offset);
GLvoid ilG_VertexArrayMultiTexCoordOffsetEXT(GLuint vaobj, GLuint buffer,
        GLenum texunit,
        GLint size, GLenum type, GLsizei stride,
        GLintptr offset);
GLvoid ilG_VertexArrayFogCoordOffsetEXT(GLuint vaobj, GLuint buffer,
                                    GLenum type, GLsizei stride,
                                    GLintptr offset);
GLvoid ilG_VertexArraySecondaryColorOffsetEXT(GLuint vaobj, GLuint buffer,
        GLint size, GLenum type,
        GLsizei stride, GLintptr offset);
GLvoid ilG_VertexArrayVertexAttribOffsetEXT(GLuint vaobj, GLuint buffer,
                                        GLuint index, GLint size,
                                        GLenum type, GLboolean normalized,
                                        GLsizei stride, GLintptr offset);
GLvoid ilG_VertexArrayVertexAttribIOffsetEXT(GLuint vaobj, GLuint buffer,
        GLuint index, GLint size,
        GLenum type, GLsizei stride,
        GLintptr offset);

/*OpenGL 3.0: New vertex array enable commands for vertex array
objects change "ClientState" to "VertexArray" and add an initial
"GLuint vaobj" parameter*/

GLvoid ilG_EnableVertexArrayEXT(GLuint vaobj, GLenum array);
GLvoid ilG_DisableVertexArrayEXT(GLuint vaobj, GLenum array);

/*OpenGL 3.0: New vertex attrib array enable commands for vertex
array objects change "VertexAttribArray" to "VertexArrayAttrib"
and add an initial "GLuint vaobj" parameter*/

GLvoid ilG_EnableVertexArrayAttribEXT(GLuint vaobj, GLuint index);
GLvoid ilG_DisableVertexArrayAttribEXT(GLuint vaobj, GLuint index);

/*OpenGL 3.0: New queries for vertex array objects*/

GLvoid ilG_GetVertexArrayIntegervEXT(GLuint vaobj, GLenum pname,
                                 GLint *param);
GLvoid ilG_GetVertexArrayPointervEXT(GLuint vaobj, GLenum pname,
                                 GLvoid **param);
GLvoid ilG_GetVertexArrayIntegeri_vEXT(GLuint vaobj,
                                   GLuint index,
                                   GLenum pname,
                                   GLint *param);
GLvoid ilG_GetVertexArrayPointeri_vEXT(GLuint vaobj,
                                   GLuint index,
                                   GLenum pname,
                                   GLvoid **param);

/*OpenGL 3.0: New buffer commands replace "Buffer" with "NamedBuffer"
in name and replace "GLenum target" parameter with "GLuint buffer"*/

GLvoid *ilG_MapNamedBufferRangeEXT(GLuint buffer, GLintptr offset,
                               GLsizeiptr length, GLbitfield access);
GLvoid ilG_FlushMappedNamedBufferRangeEXT(GLuint buffer, GLintptr offset,
                                      GLsizeiptr length);

#endif

