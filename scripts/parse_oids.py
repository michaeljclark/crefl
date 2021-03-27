#!/usr/bin/python3

import argparse

# reads ASN.1 oids from `data/oids.txt` and converts from the
# hexidecimal representation to the decimal representation then
# outputs a map suitable for pasting into a C array of structs

from operator import itemgetter

def parse_oid(v):
    h = v.strip("\"").split("\\x")[1:]
    return list(map(lambda x: int(x, 16), h))

def decode_oid(l):
    c, i, r = 0, 0, []
    for v in l:
        c = (c << 7) + (v & 0x7f)
        if v < 128:
            if i == 0:
                r += [ int(c/40), int(c%40) ]
            else:
                r += [ c ]
            c = 0
        i += 1
    return r

def join_oid_str(l):
    return ".".join(str(v) for v in l)

def join_oid_hex(l):
    return ','.join('0x{:02x}'.format(x) for x in l)

def print_oids_str(oids):
    for o in oids:
        print("{{ {:30} {:42} }},"
            .format(o['oidstr'], o['desc']))

def print_oids_hex(oids):
    for o in oids:
        print("{{ {}, {}, {} }},"
            .format(o['oidhex'], o['length'], o['desc']))

oids = []
with open('data/oids.txt') as fp:
    for l in fp.readlines():
        (k,v) = l.strip().split("=")
        oidb = parse_oid(v)
        oidi = decode_oid(oidb)
        oidstr = "\"{}\",".format(join_oid_str(oidi))
        oidhex = "{{ {} }}".format(join_oid_hex(oidb))
        desc = "\"{}\"".format(k.replace("OBJ_", ""))
        oids.append({
            'oidhex': oidhex, 'oidstr': oidstr,
            'desc': desc, 'length': len(oidb)
        })
oids = sorted(oids, key=itemgetter('oidstr'))

parser = argparse.ArgumentParser(description='decode X.509 oids')
parser.add_argument('--strings', default=False, action='store_true',
                    help='output oids as bytes')
parser.add_argument('--bytes', default=False, action='store_true',
                    help='output oids as bytes')
args = parser.parse_args()

if args.strings:
    print_oids_str(oids)
if args.bytes:
    print_oids_hex(oids)
