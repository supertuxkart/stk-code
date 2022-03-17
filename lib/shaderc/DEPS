use_relative_paths = True

vars = {
  'google_git':  'https://github.com/google',
  'khronos_git': 'https://github.com/KhronosGroup',

  'effcee_revision' : '2ec8f8738118cc483b67c04a759fee53496c5659',
  'glslang_revision': 'c34bb3b6c55f6ab084124ad964be95a699700d34',
  'googletest_revision': '389cb68b87193358358ae87cc56d257fd0d80189',
  're2_revision': '7107ebc4fbf7205151d8d2a57b2fc6e7853125d4',
  'spirv_headers_revision': 'b42ba6d92faf6b4938e6f22ddd186dbdacc98d78',
  'spirv_tools_revision': 'b846f8f1dc2d79f2b5ce27d5ad901f885da1cf82',
}

deps = {
  'third_party/effcee': Var('google_git') + '/effcee.git@' +
      Var('effcee_revision'),

  'third_party/googletest': Var('google_git') + '/googletest.git@' +
      Var('googletest_revision'),

  'third_party/glslang': Var('khronos_git') + '/glslang.git@' +
      Var('glslang_revision'),

  'third_party/re2': Var('google_git') + '/re2.git@' +
      Var('re2_revision'),

  'third_party/spirv-headers': Var('khronos_git') + '/SPIRV-Headers.git@' +
      Var('spirv_headers_revision'),

  'third_party/spirv-tools': Var('khronos_git') + '/SPIRV-Tools.git@' +
      Var('spirv_tools_revision'),
}
