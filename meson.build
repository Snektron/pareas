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
    'src/llpgen/error_reporter.cpp',
    'src/llpgen/grammar.cpp',
    'src/llpgen/grammar_parser.cpp',
    'src/llpgen/ll.cpp',
    'src/llpgen/llp/admissible_pair.cpp',
    'src/llpgen/llp/generator.cpp',
    'src/llpgen/llp/item.cpp',
    'src/llpgen/llp/item_set.cpp',
    'src/llpgen/llp/llp_table.cpp',
    'src/llpgen/llp/psls_table.cpp',
    'src/llpgen/main.cpp',
    'src/llpgen/terminal_set_functions.cpp',
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
