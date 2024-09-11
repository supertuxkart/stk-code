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
 } catch (e) { //note: i don't know if this patch has any side effects. it seems to work for now even though i don't really understand why
  if (!GLctx.currentElementArrayBufferBinding) {
    GLctx.bindBuffer(0x8893 /*GL_ELEMENT_ARRAY_BUFFER*/, null);
  }
}