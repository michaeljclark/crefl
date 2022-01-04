#!/usr/bin/python3

import os
import platform
import argparse
import subprocess
import itertools

bin_path = '/usr/bin'
search_prefixes = [ '/opt/llvm']
_platform_lib_prefix = { 'Linux': 'lib', 'Darwin': 'lib', 'Windows': '' }
_platform_lib_ext = { 'Linux': '.so', 'Darwin': '.dylib', 'Windows': '.dll' }

for prefix in search_prefixes:
    if os.path.isfile(prefix + '/bin/clang'):
        bin_path = prefix + '/bin'
        os.environ['LD_LIBRARY_PATH'] = prefix + '/lib'

def xclang_c():
    return '%s/clang' % (bin_path)

def xclang_cxx():
    return '%s/clang++' % (bin_path)

def xclang_args(args):
    return list(itertools.chain(*zip([ '-Xclang' ] * len(args), args)))

def xplugin_arg(arg):
    return xclang_args(['-plugin-arg-crefl', arg ])

def xclang_plugin(plugin, name):
    sysname = platform.system();
    return "%s/%s%s%s" % ( plugin, _platform_lib_prefix[sysname], name, _platform_lib_ext[sysname] )

def xclang_cmd(is_cpp, plugin):
    cmd = [ xclang_cxx(), '-c', '-xc++' ] if is_cpp else [ xclang_c(), '-c' ]
    cmd += xclang_args(['-load', xclang_plugin(plugin, 'crefl'), '-plugin', 'crefl'])
    return cmd

def crefl_meta_cmd(sources, output, includes, is_cpp, is_debug, plugin):
    cmd = xclang_cmd(is_cpp, plugin)
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

def crefl_meta(sources, output, includes, is_cpp, is_debug, plugin, no_exec):
    cmd = crefl_meta_cmd(sources, output, includes, is_cpp, is_debug, plugin)
    if no_exec:
        print(format_cmd(cmd))
    else:
        return subprocess.run(cmd, check=True)

parser = argparse.ArgumentParser(description='invoke crefl clang plugin')
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
                    help='enable crefl debug output')
parser.add_argument('files', nargs='*',
                    help='files to be processed')
args = parser.parse_args()

if len(args.files) == 0:
    parser.error("no input files")

crefl_meta(args.files, args.output, args.include, args.cpp, args.debug, args.plugin, args.no_exec)
