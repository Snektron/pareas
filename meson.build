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
    ['-fno-math-errno'],
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
    'src/lpg/renderer.cpp',
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

# Compiler
dependencies = [fmt_dep]

# Build futhark library
futhark_backend = get_option('futhark-backend')

if futhark_backend == 'multicore'
    dependencies += dependency('threads')
elif futhark_backend == 'opencl'
    dependencies += dependency('OpenCL')
elif futhark_backend == 'cuda'
    dependencies += dependency('cuda', modules: ['cuda', 'cudart', 'nvrtc'])
endif

grammar = custom_target(
    'grammar',
    input: ['src/compiler/lexer/pareas.lex', 'src/compiler/parser/pareas.g'],
    output: ['pareas_grammar.cpp', 'pareas_grammar.hpp', 'pareas_grammar.fut', 'pareas_grammar.lexer.in'],
    command: [
        pareas_lpg,
        '--lexer', '@INPUT0@',
        '--parser', '@INPUT1@',
        '-o', 'pareas_grammar',
        '--namespace', 'grammar',
    ],
)
grammar_cpp = grammar[0]
grammar_hpp = grammar[1]
grammar_fut = grammar[2]
grammar_dat = grammar[3]

futhark_sources = [
    'lib/github.com/diku-dk/sorts/radix_sort.fut',
    'src/compiler/lexer/lexer.fut',
    'src/compiler/parser/binary_tree.fut',
    'src/compiler/parser/bracket_matching.fut',
    'src/compiler/parser/parser.fut',
    'src/compiler/string.fut',
    'src/compiler/frontend.fut',
    'src/compiler/util.fut',
]

futhark_compile_command = [
    find_program('src/tools/compile_futhark.py'),
    '--futhark', find_program('futhark'),
    '--futhark-backend', futhark_backend,
    '--output', 'futhark_generated',
    '--dir', '@PRIVATE_DIR@',
    '--main', 'src/compiler/frontend.fut',
]

inputs = []

foreach source : futhark_sources
    futhark_compile_command += ['-f', '@INPUT@0@@'.format(inputs.length()), source]
    inputs += source
endforeach

futhark_compile_command += ['-f', '@INPUT@0@@'.format(inputs.length()), 'gen/pareas_grammar.fut']
inputs += grammar_fut

futhark_generated = custom_target(
    'futhark',
    input: inputs,
    output: ['futhark_generated.c', 'futhark_generated.h'],
    command: futhark_compile_command,
)

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
