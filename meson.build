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

lpg_sources = [
    'src/lpg/cli_util.cpp',
    'src/lpg/error_reporter.cpp',
    'src/lpg/main.cpp',
    'src/lpg/parser.cpp',
    'src/lpg/token_mapping.cpp',
    'src/lpg/lexer/char_range.cpp',
    'src/lpg/lexer/fsa.cpp',
    'src/lpg/lexer/interpreter.cpp',
    'src/lpg/lexer/lexer_parser.cpp',
    'src/lpg/lexer/lexical_grammar.cpp',
    'src/lpg/lexer/parallel_lexer.cpp',
    'src/lpg/lexer/regex.cpp',
    'src/lpg/lexer/regex_parser.cpp',
    'src/lpg/lexer/render.cpp',
    'src/lpg/parser/grammar.cpp',
    'src/lpg/parser/grammar_parser.cpp',
    'src/lpg/parser/terminal_set_functions.cpp',
    'src/lpg/parser/ll/generator.cpp',
    'src/lpg/parser/ll/parsing_table.cpp',
    'src/lpg/parser/llp/admissible_pair.cpp',
    'src/lpg/parser/llp/generator.cpp',
    'src/lpg/parser/llp/item.cpp',
    'src/lpg/parser/llp/item_set.cpp',
    'src/lpg/parser/llp/parsing_table.cpp',
    'src/lpg/parser/llp/psls_table.cpp',
    'src/lpg/parser/llp/render.cpp',
    'src/lpg/parser/llp/test_parser.cpp',
]

pareas_lpg = executable(
    'pareas-lpg',
    lpg_sources,
    dependencies: fmt_dep,
    include_directories: inc,
)

# Generated sources
run_target(
    'gen',
    command: [
        pareas_lpg,
        '--lexer',  files('src/compiler/lexer/pareas.lex'),
        '--parser', files('src/compiler/parser/pareas.g'),
        '-o', 'gen/pareas_grammar'
    ]
)

# Compiler
dependencies = [fmt_dep]

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
