# Crefl

> _Crefl_ - a C-type-reflection-API and clang plug-in to write reflection metadata.

The _Crefl_ API provides access to runtime reflection metadata for C
structure declarations with support for arbitrarily nested combinations
of: intrinsic, set, enum, struct, union, field, array, constant, variable.

_Crefl_ addresses the following three areas:

- The _Crefl_ reflection graph database format for portable reflection metadata.
- The _Crefl_ clang plug-in outputs C reflection metadata used by the library.
- The _Crefl_ API provides task-oriented query access to C reflection metadata.

![crefl](/images/crefl.svg)

---

## Crefl example

This example has an outer-loop iterating through _struct types_ and an
inner-loop iterating through _struct fields_.

```C
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <assert.h>

#include "cmodel.h"
#include "cfileio.h"

int main(int argc, const char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "error: usage: %s <filename>\n", argv[0]);
        exit(1);
    }

    decl_db *db = crefl_db_new();
    crefl_db_read_file(db, argv[1]);

    size_t ntypes = 0;
    crefl_list_types(db, NULL, &ntypes);
    decl_ref *_types = calloc(ntypes, sizeof(decl_ref));
    assert(_types);
    crefl_list_types(db, _types, &ntypes);

    for (size_t i = 0; i < ntypes; i++) {
        size_t nfields = 0;
        if (crefl_is_struct(_types[i])) {
            printf("%s %s : %zu\n",
                crefl_tag_name(crefl_decl_tag(_types[i])),
                crefl_decl_name(_types[i]),
                crefl_type_width(_types[i]));

            crefl_struct_fields(_types[i], NULL, &nfields);
            decl_ref *_fields = calloc(nfields, sizeof(decl_ref));
            assert(_fields);
            crefl_struct_fields(_types[i], _fields, &nfields);
            for (size_t j = 0; j < nfields; j++) {
                printf("\t%s %s : %zu\n",
                    crefl_tag_name(crefl_decl_tag(_fields[j])),
                    crefl_decl_name(_fields[j]),
                    crefl_type_width(_fields[j]));
            }
        }
    }

    crefl_db_destroy(db);
}
```

---

## Crefl model

_Crefl_ implements a data model based on the description of the C data types
in ISO/IEC 9899:9999 with minor changes. The following sections describe:

- primary types to model the type system
- decl node type to model the metadata graph
- decl node subtypes to model C structures and interfaces
  - _intrinsic, typedef, set, enum, struct, union, field, array, pointer,
    constant, function, param, attribute, value_

One variation from the C normative terminology is the use of _field_
instead of _member_ for structure elements.

### primary types

The _Crefl_ API uses a small number of primary data types for the reflection
graph database, the declaration graph nodes and their properties.

| Type       | Description                                    |
| :--------- | :--------------------------------------------- |
| `decl_node`| graph database declaration node type           |
| `decl_db`  | graph database containing the declarations     |
| `decl_ref` | reference to a single declaration graph node   |
| `decl_tag` | enumeration indicating graph node type         |
| `decl_id`  | indice of a graph node in the graph database   |
| `decl_sz`  | size type used for array size and bit widths   |
| `decl_set` | type used to indicate many-of set enumerations |
| `decl_raw` | raw value used to store constants              |

### decl node

The _Crefl_ data structure consists of an array of _decl_ nodes which have a
type tag, a set of properties, an interned name, a link to the next item in a
list, a link to a child item and a link to an optional attribute list.

| Type       | Name               | Description                                    |
| :--------- | :----------------- | :--------------------------------------------- |
| `decl_tag` | `tag`              | tagged union node type                         |
| `decl_set` | `props`            | type specific properties                       |
| `decl_id`  | `name`             | link to node name                              |
| `decl_id`  | `next`             | link to next item (_south_)                    |
| `decl_id`  | `link`             | link to child item (_east_)                    |
| `decl_id`  | `attr`             | link to attribute list (_south-east_)          |

The semantic topology of the graph links (_south_, _south-east_, _east_)
are used to think about graph layout. On a board, the _next_ node would be
_south_, or below the current node. A child or _link_ node would increment
to the _east_ (field type, function parameters, etc). Attributes can exist
on any node type and thus are semantically referred to as _south-east_.

### decl subtypes

The _decl_ node contains a union with type specific properties such as a
count, a width or a size. Not all types use the link field or the quantifer.
The type none is used in the link field of an empty struct or union, in the
next field to indicate the end of lists, and in the attr field to indicate
the absense of attributes.

This table table lists the properties used by each subtype:

| Type        | Link | Properties | Description                                    |
| :---------- | :--- | :--------- | :--------------------------------------------- |
| `none`      |      |            | empty type                                     |
| `typedef`   | ✓    |            | alias to another type definition               |
| `intrinsic` |      | `sz width` | machine type quantified with width in bits     |
| `set`       | ✓    | `sz width` | machine type with many-of sequence of masks    |
| `enum`      | ✓    | `sz width` | machine type with one-of sequence of integers  |
| `struct`    | ✓    |            | sequence of non-overlapping types              |
| `union`     | ✓    |            | sequence of overlapping types                  |
| `field`     | ✓    | `sz width` | named field within struct or union             |
| `array`     | ✓    | `sz count` | sequence of one type                           |
| `pointer`   | ✓    | `sz width` | pointer type                                   |
| `constant`  | ✓    | `sz value` | named constant                                 |
| `function`  | ✓    | `sz addr`  | function with input and output parameter list  |
| `param`     | ✓    |            | named parameter with link to next              |
| `attribute` | ✓    |            | custom attribute name                          |
| `value`     |      |            | custom attribute value                         |

---

## Crefl implementation notes

The _Crefl_ implementation is currently _alpha software_.

- binary format is subject to change and needs to be more compact.
  - format was reduced ~20% in size by eliding builtin types.
  - format could be made even smaller using LEB128 or ASN.1.

### Crefl features

- [x] C intrinsic data types.
  - [x] integer types.
    - _bit, byte, short, int, long, cent_
    - _sign, ubyte, ushort, uint, ulong, ucent_
  - [x] floating point types.
    - _half, float, double, quad_
  - [x] complex number types.
    - _chalf, cfloat, cdouble, cquad_
  - [x] boolean type.
    - _bool_
  - [ ] vector types.
    - _vec2, vec3, vec4, ..._
- [x] nested struct, union, field and intrinsic types.
- [x] bitfield widths.
- [x] arrays and pointers.
- [x] typedef type aliases.
- [x] enum and enum constants.
- [x] functions and function parameters.
- [x] const, volatile and restrict.
- [ ] static, inline, extern.
- [ ] function addresses.
- [x] attributes (`__attribute__`).
  - `alias(`_"symbol"_`)`
  - `aligned(`_alignment_`)`
  - `always_inline`
  - `annotate(`_"string"_`)`
  - `deprecated`
  - `packed`
  - `pure`
  - `used`
  - `unused`

---

## Crefl build instructions

crefl has been tested on ubuntu 20.04 LTS.

### install dependencies

ubuntu 20.04 LTS:

```shell
sudo apt-get install llvm libclang-dev
```

### building crefl

ubuntu 20.04 LTS:

```shell
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build -- --verbose
```

### invoking plugin

to run the crefl plugin and dump the reflection table to stdout:

```shell
clang test/simple-struct-1.h \
      -Xclang -load -Xclang build/libcrefl.so \
      -Xclang -plugin -Xclang crefl \
      -Xclang -plugin-arg-crefl -Xclang -dump
```

to run the crefl plugin and write the reflection data to a file:

```shell
clang test/simple-struct-1.h \
      -Xclang -load -Xclang build/libcrefl.so \
      -Xclang -plugin -Xclang crefl \
      -Xclang -plugin-arg-crefl -Xclang -o \
      -Xclang -plugin-arg-crefl -Xclang tmp/simple-struct-1.refl
```

to enable crefl plugin debugging, add the following option:

```shell
      -Xclang -plugin-arg-crefl -Xclang -debug
```
