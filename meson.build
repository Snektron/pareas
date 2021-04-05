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

dependencies = []

# Futhark sources
futhark_sources = [
    'src/datatypes.fut',
    'src/instr.fut',
    'src/instr_count.fut',
    'src/lisp.fut',
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
    'src/codegen/token.cpp',
    'src/main.cpp'
]

# Test executable
test_sources = [
    'src/test/coderenderer.cpp',
    'src/test/commitrenderstate.cpp',
    'src/test/idcreaterenderstate.cpp',
    'src/test/idrenderstate.cpp',
    'src/test/indentrenderstate.cpp',
    'src/test/noderenderstate.cpp',
    'src/test/randomfloatrenderstate.cpp',
    'src/test/randomintrenderstate.cpp',
    'src/test/renderstate.cpp',
    'src/test/repeatrenderstate.cpp',
    'src/test/scoperenderstate.cpp',
    'src/test/testgen.cpp',
    'src/test/textrenderstate.cpp'
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
    'pareas-testgen',
    test_sources,
    install: true,
    build_by_default: true,
    include_directories: [include_directories('src'), include_directories('include')]
)