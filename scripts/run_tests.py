#!/usr/bin/python3

import os
import glob
import argparse
import subprocess
import itertools

def crefl_source(hdr):
    return hdr

def crefl_file(hdr):
    return 'build/tmp/%s.refl' % (os.path.basename(hdr))

def xclang_args(args):
    return list(itertools.chain(*zip([ '-Xclang' ] * len(args), args)))

def xplugin_arg(arg):
    return xclang_args(['-plugin-arg-crefl', arg ])

def xclang_cmd(hdr, is_cpp):
    cmd = [ 'clang++', '-c', '-xc++' ] if is_cpp else [ 'clang', '-c' ]
    cmd += [ crefl_source(hdr) ]
    cmd += xclang_args(['-load', 'build/libcrefl.so', '-plugin', 'crefl'])
    return cmd

def crefl_debug_cmd(hdr, includes, is_cpp):
    cmd = xclang_cmd(hdr, is_cpp)
    if includes:
        for include in includes:
            cmd += ['-I%s' % (include)]
    cmd += xplugin_arg('-debug')
    return cmd

def crefl_meta_cmd(hdr, includes, is_cpp):
    cmd = xclang_cmd(hdr, is_cpp)
    if includes:
        for include in includes:
            cmd += ['-I%s' % (include)]
    cmd += xplugin_arg('-o')
    cmd += xplugin_arg(crefl_file(hdr))
    return cmd

def crefl_cat(hdr):
    with open (crefl_source(hdr), "r") as f:
        data = f.read()
        print(data)

def crefl_meta(hdr, includes, is_cpp):
    cmd = crefl_meta_cmd(hdr, includes, is_cpp)
    return subprocess.run(cmd, check=True)

def crefl_debug(hdr, includes, is_cpp):
    cmd = crefl_debug_cmd(hdr, includes, is_cpp)
    return subprocess.run(cmd, check=True)

def crefl_dump(hdr):
    cmd = [ './build/crefltool', "--dump", crefl_file(hdr)]
    out = subprocess.run(cmd)

def crefl_dump_fqn(hdr):
    cmd = [ './build/crefltool', "--dump-fqn", crefl_file(hdr)]
    out = subprocess.run(cmd)

def crefl_dump_all(hdr):
    cmd = [ './build/crefltool', "--dump-all", crefl_file(hdr)]
    out = subprocess.run(cmd)

def crefl_dump_ext(hdr):
    cmd = [ './build/crefltool', "--dump-ext", crefl_file(hdr)]
    out = subprocess.run(cmd)

def crefl_stats(hdr):
    cmd = [ './build/crefltool', "--stats", crefl_file(hdr)]
    out = subprocess.run(cmd)

def crefl_header(lab, hdr):
    print("===== %-6s test-case: %s =====\n" % (lab, hdr))

parser = argparse.ArgumentParser(description='runs crefl clang plugin on test cases')
parser.add_argument('--cpp', default=False, action='store_true',
                    help='enable c++ mode')
parser.add_argument('-I', '--include', action='append',
                    help='include directory')
parser.add_argument('--dump', default=True, action='store_true',
                    help='include standard fields in dump')
parser.add_argument('--dump-fqn', default=False, action='store_true',
                    help='include fqn field in dump')
parser.add_argument('--dump-all', default=False, action='store_true',
                    help='include all fields in dump')
parser.add_argument('--dump-ext', default=False, action='store_true',
                    help='include all fields in dump')
parser.add_argument('--debug', default=False, action='store_true',
                    help='enable crefl debug output')
parser.add_argument('--stats', default=False, action='store_true',
                    help='enable crefl stats output')
parser.add_argument('files', nargs='*', default=['test/*.h'],
                    help='files to be processed')
args = parser.parse_args()

if not os.path.exists('build/tmp'):
    os.makedirs('build/tmp')

print()
for f in args.files:
    g = glob.glob(f)
    for hdr in g:
        crefl_header('INPUT', hdr)
        crefl_cat(hdr)
        if args.debug:
            crefl_header('DEBUG', hdr)
            crefl_debug(hdr, args.include, args.cpp)
        crefl_header('OUTPUT', hdr)
        crefl_meta(hdr, args.include, args.cpp)
        if args.dump_ext:
            crefl_dump_ext(hdr)
        elif args.dump_fqn:
            crefl_dump_fqn(hdr)
        elif args.dump_all:
            crefl_dump_all(hdr)
        elif args.dump:
            crefl_dump(hdr)
        if args.stats:
            crefl_header('STATS', hdr)
            crefl_stats(hdr)
print()
