project(
    'pareas',
    ['c', 'cpp'],
    version: '0.0.0',
    default_options: [
        'cpp_std=c++2a',
        'buildtype=debugoptimized',
        'warning_level=2',
    ]
)

add_project_arguments(
    ['-Wno-unused-parameter'],
    language: ['c', 'cpp'],
)

# Build futhark library
futhark = find_program('futhark')
futhark_backend = get_option('futhark-backend')
futhark_generated = custom_target(
    'futhark',
    input: 'src/main.fut',
    output: ['futhark-generated.c', 'futhark-generated.h'],
    command: [futhark, futhark_backend, '@INPUT@', '--library', '-o', 'futhark-generated']
)
futhark_dependencies = []
if futhark_backend == 'multicore'
    futhark_dependencies += dependency('threads')
elif futhark_backend == 'opencl'
    futhark_dependencies += dependency('OpenCL')
endif

futhark_generated_lib = static_library(
    'futhark-generated',
    [futhark_generated],
    dependencies: futhark_dependencies,
)
futhark_generated_dep = declare_dependency(
    sources: futhark_generated,
    link_with: futhark_generated_lib,
)

# Final executable
sources = [
    'src/main.cpp',
]

executable(
    'pareas',
    sources,
    install: true,
    build_by_default: true,
    dependencies: [futhark_generated_dep],
    include_directories: include_directories('src'),
)
