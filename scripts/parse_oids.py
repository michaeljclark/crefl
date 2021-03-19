#!/usr/bin/python3

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

def join_oid(l):
	return ".".join(str(v) for v in l)

oids = []
with open('data/oids.txt') as fp:
    for l in fp.readlines():
    	(k,v) = l.strip().split("=")
    	oid = "\"{}\",".format(join_oid(decode_oid(parse_oid(v))))
    	desc = "\"{}\"".format(k.replace("OBJ_", ""))
    	oids.append([ oid, desc])

oids = sorted(oids, key=itemgetter(0))

for row in oids:
	print("{{ {:30} {:42} }},".format(*row))