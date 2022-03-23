#!/usr/bin/env python3
import argparse
import shutil
import subprocess
import os
import sys

p = argparse.ArgumentParser(description='Futhark compiler wrapper which deals with generated files')
p.add_argument('--dir', required=True, help='Directory to place futhark sources in')
p.add_argument('--futhark', required=True, help='Futhark compiler binary path')
p.add_argument('--futhark-backend', required=True, help='Futhark compilation backend (cuda, opencl, multicore, c)')
p.add_argument('--output', required=True, help='Output basename')
p.add_argument('--hash', help='Compute source hash and create a configuration file to store it in')
p.add_argument('--main', required=True, help='Main futhark file (relative to --src)')
p.add_argument('-f', dest='sources', nargs=2, metavar=('src', 'relative src'), action='append', help='Source files and their path relative to --dir')

args = p.parse_args()

for (src, f) in args.sources:
    dst = os.path.join(args.dir, f)
    os.makedirs(os.path.dirname(dst), exist_ok=True)
    shutil.copy(src, dst)

path = os.path.join(args.dir, args.main)

try:
    if args.hash is not None:
        result = subprocess.run(
            [
                args.futhark,
                'hash',
                path,
            ],
            check=True,
            capture_output=True,
        )
        with open(args.hash, 'wb') as f:
            f.write(b'#define FUTHARK_SOURCE_HASH "')
            f.write(bytes(result.stdout.strip()))
            f.write(b'"\n')

    subprocess.run(
        [
            args.futhark,
            args.futhark_backend,
            path,
            '--library',
            '-o', args.output,
        ],
        check=True
    )
except subprocess.CalledProcessError:
    sys.exit(1)
