#!/usr/bin/python3

import os
import glob
import argparse
import subprocess

def crefl_source(hdr):
    return hdr

def crefl_file(hdr):
    return 'build/tmp/%s.refl' % (os.path.basename(hdr))

def crefl_debug_cmd(hdr):
    return [
        'clang', crefl_source(hdr),
        '-Xclang', '-load',
        '-Xclang', 'build/libcrefl.so',
        '-Xclang', '-plugin', '-Xclang', 'crefl',
        '-Xclang', '-plugin-arg-crefl', '-Xclang', '-debug'
    ]

def crefl_meta_cmd(hdr):
    return [
        'clang', crefl_source(hdr),
        '-Xclang', '-load',
        '-Xclang', 'build/libcrefl.so',
        '-Xclang', '-plugin', '-Xclang', 'crefl',
        '-Xclang', '-plugin-arg-crefl', '-Xclang', '-o',
        '-Xclang', '-plugin-arg-crefl', '-Xclang', crefl_file(hdr)
    ]

def crefl_cat(hdr):
    with open (crefl_source(hdr), "r") as f:
        data = f.read()
        print(data)

def crefl_meta(hdr):
    cmd = crefl_meta_cmd(hdr)
    return subprocess.check_output(cmd)

def crefl_debug(hdr):
    cmd = crefl_debug_cmd(hdr)
    return subprocess.check_output(cmd)

def crefl_dump(hdr):
    cmd = [ './build/crefltool', crefl_file(hdr)]
    out = subprocess.run(cmd)

parser = argparse.ArgumentParser(description='runs crefl clang plugin on test cases')
parser.add_argument('--debug', default=False, action='store_true',
                    help='enable crefl debug output')
parser.add_argument('files', nargs='*', default=['test/*.h'],
                    help='files to be processed')
args = parser.parse_args()

if not os.path.exists('build/tmp'):
    os.makedirs('build/tmp')

for f in args.files:
    g = glob.glob(f)
    for hdr in g:
        print("===== INPUT  test-case: %s =====" % (hdr))
        crefl_cat(hdr)
        if args.debug:
            print("===== DEBUG test-case: %s =====" % (hdr))
            crefl_debug(hdr)
        print("===== OUTPUT test-case: %s =====" % (hdr))
        crefl_meta(hdr)
        crefl_dump(hdr)
