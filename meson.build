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

inc = include_directories('include')

# LLPGEN
llpgen_sources = [
    'src/llpgen/admissible_pair.cpp',
    'src/llpgen/generator.cpp',
    'src/llpgen/grammar.cpp',
    'src/llpgen/grammar_parser.cpp',
    'src/llpgen/item.cpp',
    'src/llpgen/item_set.cpp',
    'src/llpgen/ll_table.cpp',
    'src/llpgen/llp_table.cpp',
    'src/llpgen/main.cpp',
    'src/llpgen/psls_table.cpp',
]

executable(
    'pareas-llpgen',
    llpgen_sources,
    include_directories: inc,
)

dependencies = []

# Build futhark library
futhark = find_program('futhark')
futhark_backend = get_option('futhark-backend')
futhark_generated = custom_target(
    'futhark',
    input: 'src/compiler/main.fut',
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
    'src/compiler/main.cpp',
]

executable(
    'pareas',
    [sources, futhark_generated],
    install: true,
    build_by_default: true,
    dependencies: dependencies,
    include_directories: inc,
)
