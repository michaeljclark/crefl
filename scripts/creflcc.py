#!/usr/bin/python3

import argparse
import subprocess
import itertools

def xclang_args(args):
    return list(itertools.chain(*zip([ '-Xclang' ] * len(args), args)))

def xplugin_arg(arg):
    return xclang_args(['-plugin-arg-crefl', arg ])

def xclang_cmd(is_cpp):
    cmd = [ 'clang++', '-c', '-xc++' ] if is_cpp else [ 'clang', '-c' ]
    cmd += xclang_args(['-load', 'build/libcrefl.so', '-plugin', 'crefl'])
    return cmd

def crefl_meta_cmd(sources, output, is_cpp):
    cmd = xclang_cmd(is_cpp)
    cmd += xplugin_arg('-o')
    cmd += xplugin_arg(output)
    cmd += sources
    return cmd

def crefl_meta(sources, output, is_cpp, is_verbose):
    cmd = crefl_meta_cmd(sources, output, is_cpp)
    if is_verbose:
        print(" ".join(cmd))
    return subprocess.run(cmd, check=True)

parser = argparse.ArgumentParser(description='runs crefl clang plugin on test cases')
parser.add_argument('--cpp', default=False, action='store_true',
                    help='enable c++ mode')
parser.add_argument('-v', '--verbose', default=False, action='store_true',
                    help='show comand invocation')
parser.add_argument('-o', '--output', required=True,
                    help='reflection metadata output')
parser.add_argument('files', nargs='*',
                    help='files to be processed')
args = parser.parse_args()

if len(args.files) == 0:
    parser.error("no input files")

crefl_meta(args.files, args.output, args.cpp, args.verbose)
