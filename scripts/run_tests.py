#!/usr/bin/env python3

import os
import os.path
import glob
import platform
import argparse
import subprocess
import itertools

bin_path = '/usr/bin'
search_prefixes = [ '/usr' ]
_platform_lib_prefix = { 'Linux': 'lib', 'Darwin': 'lib', 'Windows': '' }
_platform_lib_ext = { 'Linux': '.so', 'Darwin': '.dylib', 'Windows': '.dll' }

for prefix in search_prefixes:
    if os.path.isfile(prefix + '/bin/clang'):
        bin_path = prefix + '/bin'
        break

def xclang_c():
    return '%s/clang' % (bin_path)

def xclang_cxx():
    return '%s/clang++' % (bin_path)

def xclang_args(args):
    return list(itertools.chain(*zip([ '-Xclang' ] * len(args), args)))

def xplugin_arg(arg):
    return xclang_args(['-plugin-arg-cinf', arg ])

def xclang_plugin(plugin, name):
    sysname = platform.system();
    return "%s/%s%s%s" % ( plugin, _platform_lib_prefix[sysname], name, _platform_lib_ext[sysname] )

def xclang_cmd(is_cpp, plugin):
    cmd = [ xclang_cxx(), '-c', '-xc++' ] if is_cpp else [ xclang_c(), '-c' ]
    cmd += xclang_args(['-load', xclang_plugin(plugin, 'cinf'), '-plugin', 'cinf'])
    return cmd

def cinf_source(hdr):
    return hdr

def cinf_file(hdr):
    return 'build/tmp/%s.refl' % (os.path.basename(hdr))

def cinf_meta_cmd(hdr, includes, is_cpp, is_debug, plugin):
    cmd = xclang_cmd(is_cpp, plugin)
    if includes:
        for include in includes:
            cmd += ['-I%s' % (include)]
    if is_debug:
        cmd += xplugin_arg('-debug')
    cmd += xplugin_arg('-o')
    cmd += xplugin_arg(cinf_file(hdr))
    cmd += [ hdr ]
    return cmd

def cinf_cat(hdr):
    with open (cinf_source(hdr), "r") as f:
        data = f.read()
        print(data)

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

def cinf_meta(hdr, includes, is_cpp, is_debug, no_exec, plugin):
    cmd = cinf_meta_cmd(hdr, includes, is_cpp, is_debug, plugin)
    if no_exec:
        print(format_cmd(cmd))
    else:
        return subprocess.run(cmd, check=True)

def cinf_tool(hdr, arg):
    cmd = [ './build/cinftool', arg, cinf_file(hdr)]
    out = subprocess.run(cmd)

def cinf_header(lab, hdr):
    print("===== %-6s test-case: %s =====\n" % (lab, hdr))

parser = argparse.ArgumentParser(description='runs cinf clang plugin on test cases')
parser.add_argument('-n', '--no-exec', default=False, action='store_true',
                    help='show the comand line invocation')
parser.add_argument('--cpp', default=False, action='store_true',
                    help='enable c++ mode')
parser.add_argument('-I', '--include', action='append',
                    help='include directory')
parser.add_argument('-p', '--plugin', action='store', default='build',
                    help='directory containing plugin')
parser.add_argument('--dump', default=True, action='store_true',
                    help='standard width dump')
parser.add_argument('--dump-fqn', default=False, action='store_true',
                    help='standard width dump with fqn field')
parser.add_argument('--dump-sum', default=False, action='store_true',
                    help='standard width dump with sum field')
parser.add_argument('--dump-all', default=False, action='store_true',
                    help='standard width dump with all fields')
parser.add_argument('--dump-ext', default=False, action='store_true',
                    help='extended width dump')
parser.add_argument('--dump-ext-fqn', default=False, action='store_true',
                    help='extended width dump with fqn field')
parser.add_argument('--dump-ext-sum', default=False, action='store_true',
                    help='extended width dump with sum field')
parser.add_argument('--dump-ext-all', default=False, action='store_true',
                    help='extended width dump with all fields')
parser.add_argument('-d', '--debug', default=False, action='store_true',
                    help='enable cinf debug output')
parser.add_argument('--stats', default=False, action='store_true',
                    help='enable cinf stats output')
parser.add_argument('files', nargs='*', default=['test/input/*.h'],
                    help='files to be processed')
args = parser.parse_args()

if not os.path.exists('build/tmp'):
    os.makedirs('build/tmp')

print()
for f in args.files:
    g = glob.glob(f)
    for hdr in g:
        cinf_header('INPUT', hdr)
        cinf_cat(hdr)
        cinf_header('OUTPUT', hdr)
        cinf_meta(hdr, args.include, args.cpp, args.debug, args.no_exec, args.plugin)
        if args.no_exec:
            exit(0)
        if args.dump_fqn:
            cinf_tool(hdr, '--dump-fqn')
        elif args.dump_sum:
            cinf_tool(hdr, '--dump-sum')
        elif args.dump_all:
            cinf_tool(hdr, '--dump-all')
        elif args.dump_ext:
            cinf_tool(hdr, '--dump-ext')
        elif args.dump_ext_fqn:
            cinf_tool(hdr, '--dump-ext-fqn')
        elif args.dump_ext_sum:
            cinf_tool(hdr, '--dump-ext-sum')
        elif args.dump_ext_all:
            cinf_tool(hdr, '--dump-ext-all')
        elif args.dump:
            cinf_tool(hdr, '--dump')
        if args.stats:
            cinf_header('STATS', hdr)
            cinf_tool(hdr, '--stats')
print()
