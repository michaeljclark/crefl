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

def crefl_debug_cmd(hdr, is_cpp):
    cmd = xclang_cmd(hdr, is_cpp)
    cmd += xplugin_arg('-debug')
    return cmd

def crefl_meta_cmd(hdr, is_cpp):
    cmd = xclang_cmd(hdr, is_cpp)
    cmd += xplugin_arg('-o')
    cmd += xplugin_arg(crefl_file(hdr))
    return cmd

def crefl_cat(hdr):
    with open (crefl_source(hdr), "r") as f:
        data = f.read()
        print(data)

def crefl_meta(hdr, is_cpp):
    cmd = crefl_meta_cmd(hdr, is_cpp)
    return subprocess.run(cmd, check=True)

def crefl_debug(hdr, is_cpp):
    cmd = crefl_debug_cmd(hdr, is_cpp)
    return subprocess.run(cmd, check=True)

def crefl_dump(hdr):
    cmd = [ './build/crefltool', "--dump", crefl_file(hdr)]
    out = subprocess.run(cmd)

def crefl_stats(hdr):
    cmd = [ './build/crefltool', "--stats", crefl_file(hdr)]
    out = subprocess.run(cmd)

parser = argparse.ArgumentParser(description='runs crefl clang plugin on test cases')
parser.add_argument('--cpp', default=False, action='store_true',
                    help='enable c++ mode')
parser.add_argument('--debug', default=False, action='store_true',
                    help='enable crefl debug output')
parser.add_argument('--stats', default=False, action='store_true',
                    help='enable crefl stats output')
parser.add_argument('files', nargs='*', default=['test/*.h'],
                    help='files to be processed')
args = parser.parse_args()

if not os.path.exists('build/tmp'):
    os.makedirs('build/tmp')

for f in args.files:
    g = glob.glob(f)
    for hdr in g:
        print("===== INPUT  test-case: %s =====\n" % (hdr))
        crefl_cat(hdr)
        if args.debug:
            print("===== DEBUG test-case: %s =====\n" % (hdr))
            crefl_debug(hdr, args.cpp)
        print("===== OUTPUT test-case: %s =====\n" % (hdr))
        crefl_meta(hdr, args.cpp)
        crefl_dump(hdr)
        if args.stats:
            print("===== STATS test-case: %s =====\n" % (hdr))
            crefl_stats(hdr)
