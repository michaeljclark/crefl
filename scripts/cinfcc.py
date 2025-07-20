#!/usr/bin/env python3

import os
import platform
import argparse
import subprocess
import itertools

lib_ext = { 'Linux': '.so', 'FreeBSD': '.so', 'Darwin': '.dylib', 'Windows': '.dll' }

def xclang_c(prefix):
    return '%s/bin/clang' % (prefix)

def xclang_cxx(prefix):
    return '%s/bin/clang++' % (prefix)

def xclang_args(args):
    return list(itertools.chain(*zip([ '-Xclang' ] * len(args), args)))

def xplugin_arg(arg):
    return xclang_args(['-plugin-arg-cinfcc', arg ])

def xclang_plugin(plugin, name):
    sysname = platform.system();
    return "%s/%s%s" % ( plugin, name, lib_ext[sysname] )

def xclang_cmd(is_cpp, plugin, prefix):
    cmd = [ xclang_cxx(prefix), '-c', '-xc++' ] if is_cpp else [ xclang_c(prefix), '-c' ]
    cmd += xclang_args(['-load', xclang_plugin(plugin, 'cinfcc'), '-plugin', 'cinfcc'])
    return cmd

def cinf_meta_cmd(sources, output, includes, is_cpp, is_debug, plugin, prefix):
    cmd = xclang_cmd(is_cpp, plugin, prefix)
    if includes:
        for include in includes:
            cmd += ['-I%s' % (include)]
    if is_debug:
        cmd += xplugin_arg('-debug')
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

def cinf_meta(sources, output, includes, is_cpp, is_debug, plugin, prefix, no_exec):
    cmd = cinf_meta_cmd(sources, output, includes, is_cpp, is_debug, plugin, prefix)
    if no_exec:
        print(format_cmd(cmd))
    else:
        return subprocess.run(cmd, check=True)

parser = argparse.ArgumentParser(description='invoke cinf clang plugin')
parser.add_argument('-n', '--no-exec', default=False, action='store_true',
                    help='show the comand line invocation')
parser.add_argument('--cpp', default=False, action='store_true',
                    help='enable c++ mode')
parser.add_argument('-I', '--include', action='append',
                    help='include directory')
parser.add_argument('-o', '--output', required=True,
                    help='reflection metadata output')
parser.add_argument('-p', '--plugin', action='store', default='build',
                    help='directory containing plugin')
parser.add_argument('-d', '--debug', default=False, action='store_true',
                    help='enable cinf debug output')
parser.add_argument('-P', '--prefix', type=str, default='/usr',
                    help='prefix for clang toolchain')
parser.add_argument('files', nargs='*',
                    help='files to be processed')
args = parser.parse_args()

if len(args.files) == 0:
    parser.error("no input files")

cinf_meta(args.files, args.output, args.include, args.cpp, args.debug, args.plugin, args.prefix, args.no_exec)
