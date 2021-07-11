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

futhark = find_program('futhark')
futhark_wrapper = find_program('src/tools/compile_futhark.py')

inc = include_directories('include')

fmt_dep = subproject('fmt').get_variable('fmt_dep')

lpg_sources = files(
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
)

pareas_lpg_exe = executable(
    'pareas-lpg',
    lpg_sources,
    build_by_default: not meson.is_subproject(),
    dependencies: fmt_dep,
    include_directories: inc,
)

# Profiling library
pareas_prof_dep = declare_dependency(
    include_directories: inc,
    sources: files('src/profiler/profiler.cpp'),
    dependencies: fmt_dep,
)

# Compiler
futhark_deps = [dependency('threads')]

# Build futhark library
futhark_wrapper_exe = find_program('src/tools/compile_futhark.py')
futhark_backend = get_option('futhark-backend')

if futhark_backend == 'opencl'
    futhark_deps += dependency('OpenCL')
elif futhark_backend == 'cuda'
    futhark_deps += dependency('cuda', modules: ['cuda', 'cudart', 'nvrtc'])
endif

# Compiler
grammar = custom_target(
    'grammar',
    input: ['src/compiler/lexer/pareas.lex', 'src/compiler/parser/pareas.g'],
    output: ['pareas_grammar.hpp', 'pareas_grammar.cpp', 'pareas_grammar.fut', 'pareas_grammar.S', 'pareas_grammar.dat'],
    build_by_default: false,
    command: [
        pareas_lpg_exe,
        '--lexer', '@INPUT0@',
        '--parser', '@INPUT1@',
        '-o', '@OUTDIR@/pareas_grammar',
        '--namespace', 'grammar',
    ],
)
grammar_hpp = grammar[0]
grammar_cpp = grammar[1]
grammar_fut = grammar[2]
grammar_asm = grammar[3]

futhark_sources = [
    'lib/github.com/diku-dk/sorts/radix_sort.fut',
    'lib/github.com/diku-dk/segmented/segmented.fut',
    'src/compiler/string.fut',
    'src/compiler/util.fut',
    'src/compiler/datatypes.fut',
    'src/compiler/frontend.fut',
    'src/compiler/lexer/lexer.fut',
    'src/compiler/parser/binary_tree.fut',
    'src/compiler/parser/bracket_matching.fut',
    'src/compiler/parser/parser.fut',
    'src/compiler/passes/util.fut',
    'src/compiler/passes/tokenize.fut',
    'src/compiler/passes/fix_bin_ops.fut',
    'src/compiler/passes/fix_if_else.fut',
    'src/compiler/passes/fns_and_assigns.fut',
    'src/compiler/passes/flatten_lists.fut',
    'src/compiler/passes/remove_marker_nodes.fut',
    'src/compiler/passes/compactify.fut',
    'src/compiler/passes/reorder.fut',
    'src/compiler/passes/symbol_resolution.fut',
    'src/compiler/passes/type_resolution.fut',
    'src/compiler/passes/check_return_paths.fut',
    'src/compiler/passes/ids.fut',
]

futhark_compile_command = [
    futhark_wrapper_exe,
    '--futhark', find_program('futhark'),
    '--futhark-backend', futhark_backend,
    '--output', '@OUTDIR@/futhark_generated',
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

sources = files(
    'src/compiler/main.cpp',
    'src/compiler/frontend.cpp',
    'src/compiler/ast.cpp',
)

pareas_exe = executable(
    'pareas',
    [grammar_hpp, grammar_cpp, grammar_asm, sources, futhark_generated],
    build_by_default: not meson.is_subproject(),
    dependencies: [pareas_prof_dep, fmt_dep, futhark_deps],
    include_directories: inc,
)

# JSON test

json_sources = [
    'src/json/main.cpp',
]

json_futhark_sources = [
    'lib/github.com/diku-dk/sorts/radix_sort.fut',
    'src/compiler/string.fut',
    'src/json/main.fut',
    'src/compiler/lexer/lexer.fut',
    'src/compiler/parser/binary_tree.fut',
    'src/compiler/parser/bracket_matching.fut',
    'src/compiler/parser/parser.fut',
    'src/compiler/util.fut',
]

json_grammar = custom_target(
    'json-grammar',
    input: ['src/json/json.lex', 'src/json/json.g'],
    output: ['json_grammar.hpp', 'json_grammar.cpp', 'json_grammar.fut', 'json_grammar.S', 'json_grammar.dat'],
    command: [
        pareas_lpg_exe,
        '--lexer', '@INPUT0@',
        '--parser', '@INPUT1@',
        '-o', '@OUTDIR@/json_grammar',
        '--namespace', 'json',
    ],
)
json_grammar_hpp = json_grammar[0]
json_grammar_cpp = json_grammar[1]
json_grammar_fut = json_grammar[2]
json_grammar_asm = json_grammar[3]

json_futhark_compile_command = [
    futhark_wrapper,
    '--futhark', futhark,
    '--futhark-backend', futhark_backend,
    '--output', '@OUTDIR@/json_futhark_generated',
    '--dir', '@PRIVATE_DIR@',
    '--main', 'src/json/main.fut',
]

json_inputs = []

foreach source : json_futhark_sources
    json_futhark_compile_command += ['-f', '@INPUT@0@@'.format(json_inputs.length()), source]
    json_inputs += source
endforeach

json_futhark_compile_command += ['-f', '@INPUT@0@@'.format(json_inputs.length()), 'gen/json_grammar.fut']
json_inputs += json_grammar_fut

json_futhark_generated = custom_target(
    'json-futhark',
    input: json_inputs,
    output: ['json_futhark_generated.c', 'json_futhark_generated.h'],
    command: json_futhark_compile_command,
)

pareas_json_exe = executable(
    'pareas-json',
    [json_grammar_hpp, json_grammar_cpp, json_grammar_asm, json_sources, json_futhark_generated],
    install: true,
    build_by_default: true,
    dependencies: [pareas_prof_dep, fmt_dep, futhark_deps],
    include_directories: inc,
)
