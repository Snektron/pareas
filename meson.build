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

if host_machine.endian() != 'little'
    error('Host machine endianness is required to be litte (required by futhark_dataset.hpp)')
endif

inc = include_directories('include')

fmt_dep = subproject('fmt').get_variable('fmt_dep')

# common
pareas_common_sources = [
    'src/common/error_reporter.cpp',
    'src/common/parser.cpp',
]

pareas_common_dep = declare_dependency(
    dependencies: fmt_dep,
    sources: pareas_common_sources,
    include_directories: inc,
)

# llpgen
llpgen_sources = [
    'src/llpgen/grammar.cpp',
    'src/llpgen/grammar_parser.cpp',
    'src/llpgen/ll/generator.cpp',
    'src/llpgen/ll/parsing_table.cpp',
    'src/llpgen/llp/admissible_pair.cpp',
    'src/llpgen/llp/generator.cpp',
    'src/llpgen/llp/item.cpp',
    'src/llpgen/llp/parsing_table.cpp',
    'src/llpgen/llp/psls_table.cpp',
    'src/llpgen/llp/render.cpp',
    'src/llpgen/llp/test_parser.cpp',
    'src/llpgen/main.cpp',
    'src/llpgen/terminal_set_functions.cpp',
]

executable(
    'pareas-llpgen',
    llpgen_sources,
    dependencies: [pareas_common_dep, fmt_dep],
    include_directories: inc,
)

# lexgen
lexgen_sources = [
    'src/lexgen/char_range.cpp',
    'src/lexgen/fsa.cpp',
    'src/lexgen/interpreter.cpp',
    'src/lexgen/lexer_parser.cpp',
    'src/lexgen/main.cpp',
    'src/lexgen/parallel_lexer.cpp',
    'src/lexgen/regex.cpp',
    'src/lexgen/regex_parser.cpp',
    'src/lexgen/render.cpp',
]

executable(
    'pareas-lexgen',
    lexgen_sources,
    dependencies: [pareas_common_dep, fmt_dep],
    include_directories: inc
)

dependencies = [fmt_dep]

# Compiler
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
