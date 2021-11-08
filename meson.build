project(
    'pareas',
    ['c', 'cpp'],
    version: '0.0.0',
    default_options: [
        'cpp_std=c++2a',
        'buildtype=debugoptimized',
    ]
)

add_project_arguments(
    ['-Wno-unused-parameter'],
    language: ['c', 'cpp'],
)

fmt_dep = subproject('fmt').get_variable('fmt_dep')

dependencies = [fmt_dep]

# Futhark sources
futhark_sources = [
    'src/bridge.fut',
    'src/datatypes.fut',
    'src/instr.fut',
    'src/instr_count.fut',
    'src/lisp.fut',
    'src/optimizer.fut',
    'src/postprocess.fut',
    'src/preprocess.fut',
    'src/register.fut',
    'src/symtab.fut',
    'src/tree.fut'
]

# Build futhark library
futhark = find_program('futhark')
futhark_backend = get_option('futhark-backend')
futhark_generated = custom_target(
    'futhark',
    input: 'src/main.fut',
    output: ['futhark-generated.c', 'futhark-generated.h'],
    command: [futhark, futhark_backend, '@INPUT@', '--library', '-o', 'futhark-generated'],
    depend_files: futhark_sources
)

if futhark_backend == 'multicore'
    dependencies += dependency('threads')
elif futhark_backend == 'opencl'
    dependencies += dependency('OpenCL')
elif futhark_backend == 'cuda'
    dependencies += dependency('cuda', modules: ['cuda', 'cudart', 'nvrtc'])
endif

# Final executable
sources = [
    'src/codegen/astnode.cpp',
    'src/codegen/datatype.cpp',
    'src/codegen/depthtree.cpp',
    'src/codegen/lexer.cpp',
    'src/codegen/parser.cpp',
    'src/codegen/symtab.cpp',
    'src/codegen/treeproperties.cpp',
    'src/codegen/token.cpp',
    'src/profiler/profiler.cpp',
    'src/main.cpp'
]

#Conversion executable
convert_sources = [
    'src/codegen/astnode.cpp',
    'src/codegen/datatype.cpp',
    'src/codegen/depthtree.cpp',
    'src/codegen/lexer.cpp',
    'src/codegen/parser.cpp',
    'src/codegen/symtab.cpp',
    'src/codegen/treeproperties.cpp',
    'src/codegen/token.cpp',
    'src/converter/converter.cpp',
    'src/profiler/profiler.cpp',
    'src/conversion.cpp'
]

# Test executable
test_old_sources = [
    'src/test/old/coderenderer.cpp',
    'src/test/old/commitrenderstate.cpp',
    'src/test/old/idcreaterenderstate.cpp',
    'src/test/old/idrenderstate.cpp',
    'src/test/old/indentrenderstate.cpp',
    'src/test/old/noderenderstate.cpp',
    'src/test/old/randomfloatrenderstate.cpp',
    'src/test/old/randomintrenderstate.cpp',
    'src/test/old/renderstate.cpp',
    'src/test/old/repeatrenderstate.cpp',
    'src/test/old/scoperenderstate.cpp',
    'src/test/old/testgen.cpp',
    'src/test/old/textrenderstate.cpp'
]

test_sources = [
    'src/test/assignexprastnodetempl.cpp',
    'src/test/astgenerator.cpp',
    'src/test/astnodetempl.cpp',
    'src/test/castexprastnodetempl.cpp',
    'src/test/compexprastnodetempl.cpp',
    'src/test/condastnodetempl.cpp',
    'src/test/declareexprastnodetempl.cpp',
    'src/test/derefexprastnodetempl.cpp',
    'src/test/defaulttreeprinter.cpp',
    'src/test/exprastnodetempl.cpp',
    'src/test/fixedastnodetempl.cpp',
    'src/test/funccallastnodetempl.cpp',
    'src/test/funcdeclastnodetempl.cpp',
    'src/test/idexprastnodetempl.cpp',
    'src/test/litastnodetempl.cpp',
    'src/test/returnastnodetempl.cpp',
    'src/test/sourcefileprinter.cpp',
    'src/test/statementlistastnodetempl.cpp',
    'src/test/testmain.cpp',
    'src/test/treeproperties.cpp',
    'src/test/unaryexprastnodetempl.cpp'
]

executable(
    'pareas',
    [sources, futhark_generated],
    install: true,
    build_by_default: true,
    dependencies: dependencies,
    include_directories: [include_directories('src'), include_directories('include')],
)

executable(
    'pareas-testgen-old',
    test_old_sources,
    install: true,
    build_by_default: false,
    include_directories: [include_directories('src'), include_directories('include')]
)

executable(
    'pareas-testgen',
    test_sources,
    install: true,
    build_by_default: true,
    include_directories: [include_directories('src'), include_directories('include')]
)

executable(
    'pareas-conversion',
    convert_sources,
    install: true,
    build_by_default: true,
    dependencies: [fmt_dep],
    include_directories: [include_directories('src'), include_directories('include')]
)
