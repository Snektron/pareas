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

# Build futhark library
futhark = find_program('futhark')
futhark_backend = get_option('futhark-backend')
futhark_generated = custom_target(
    'futhark',
    input: 'src/main.fut',
    output: ['futhark-generated.c', 'futhark-generated.h'],
    command: [futhark, futhark_backend, '@INPUT@', '--library', '-o', 'futhark-generated']
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
    'src/codegen/depthtree.cpp',
    'src/codegen/lexer.cpp',
    'src/codegen/parser.cpp',
    'src/codegen/token.cpp',
    'src/main.cpp'
]

executable(
    'pareas',
    [sources, futhark_generated],
    install: true,
    build_by_default: true,
    dependencies: dependencies,
    include_directories: [include_directories('src'), include_directories('include')],
)
