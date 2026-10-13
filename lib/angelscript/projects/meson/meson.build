project('angelscript', 'cpp',
        version : run_command(find_program('python3'), 'detect_ver.py').stdout().strip(),
        meson_version : '>=0.28.0',
        license : 'zlib')

threads = dependency('threads')

angel_srcs = [
  '../../source/as_atomic.cpp',
  '../../source/as_builder.cpp',
  '../../source/as_bytecode.cpp',
  '../../source/as_callfunc.cpp',
  '../../source/as_callfunc_mips.cpp',
  '../../source/as_callfunc_ppc.cpp',
  '../../source/as_callfunc_ppc_64.cpp',
  '../../source/as_callfunc_sh4.cpp',
  '../../source/as_callfunc_x86.cpp',
  '../../source/as_callfunc_x64_gcc.cpp',
  '../../source/as_callfunc_x64_mingw.cpp',
  '../../source/as_compiler.cpp',
  '../../source/as_context.cpp',
  '../../source/as_configgroup.cpp',
  '../../source/as_datatype.cpp',
  '../../source/as_generic.cpp',
  '../../source/as_gc.cpp',
  '../../source/as_globalproperty.cpp',
  '../../source/as_memory.cpp',
  '../../source/as_module.cpp',
  '../../source/as_objecttype.cpp',
  '../../source/as_outputbuffer.cpp',
  '../../source/as_parser.cpp',
  '../../source/as_restore.cpp',
  '../../source/as_scriptcode.cpp',
  '../../source/as_scriptengine.cpp',
  '../../source/as_scriptfunction.cpp',
  '../../source/as_scriptnode.cpp',
  '../../source/as_scriptobject.cpp',
  '../../source/as_string.cpp',
  '../../source/as_string_util.cpp',
  '../../source/as_thread.cpp',
  '../../source/as_tokenizer.cpp',
  '../../source/as_typeinfo.cpp',
  '../../source/as_variablescope.cpp',
]
if host_machine.cpu_family() == 'arm'
  add_languages('c')
  angel_srcs += [
    '../../source/as_callfunc_arm.cpp',
    '../../source/as_callfunc_arm_gcc.S',
  ]
endif

if host_machine.cpu_family() == 'aarch64'
  add_languages('c')
  angel_srcs += [
    '../../source/as_callfunc_arm64.cpp',
    '../../source/as_callfunc_arm64_gcc.S',
  ]
endif

angelscript_version_num = run_command(find_program('python3'), 'detect_ver.py', '--num').stdout().strip()
angelscript_lib = library(
  'angelscript',
  sources : angel_srcs,
  dependencies : threads,
  version : angelscript_version_num,
  install : true,
  cpp_args:'-Wfatal-errors'
)
angelscript_inc = include_directories('../../include')
angelscript_dep  = declare_dependency(
  link_with : angelscript_lib,
  include_directories : angelscript_inc,
  version : meson.project_version(),
)

install_headers('../../include/angelscript.h')
