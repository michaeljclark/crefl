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

def format_cmd(cmd):
    str = ""
    lines = []
    for comp in cmd:
        if len(str + comp) > 72:
            lines.append(str)
            str = ""
        str += " " + comp if len(str) > 0 else comp
    if len(str) > 0:
        lines.append(str)
    return " \\\n    ".join(lines)

def crefl_meta(sources, output, is_cpp, no_exec):
    cmd = crefl_meta_cmd(sources, output, is_cpp)
    if no_exec:
        print(format_cmd(cmd))
    else:
        return subprocess.run(cmd, check=True)

parser = argparse.ArgumentParser(description='invoke crefl clang plugin')
parser.add_argument('-n', '--no-exec', default=False, action='store_true',
                    help='show the comand line invocation')
parser.add_argument('--cpp', default=False, action='store_true',
                    help='enable c++ mode')
parser.add_argument('-o', '--output', required=True,
                    help='reflection metadata output')
parser.add_argument('files', nargs='*',
                    help='files to be processed')
args = parser.parse_args()

if len(args.files) == 0:
    parser.error("no input files")

crefl_meta(args.files, args.output, args.cpp, args.no_exec)
