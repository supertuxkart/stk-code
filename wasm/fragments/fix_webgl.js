/* INSERT
GLctx.getAttachedShaders\(\S+?\)
*/
 || []

/* INSERT_BEFORE
GL.preDrawHandleClientVertexAttribBindings\([a-z]+?\)
*/
try {

/* INSERT
GL.preDrawHandleClientVertexAttribBindings\([a-z]+?\)
*/
 } catch (e) {
  if (!GLctx.currentElementArrayBufferBinding) {
    GLctx.bindBuffer(0x8893 /*GL_ELEMENT_ARRAY_BUFFER*/, null);
  }
}