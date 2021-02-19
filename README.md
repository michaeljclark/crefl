# crefl

> _crefl_ is a runtime library and compiler plug-in to support reflection in C.

The _crefl_ API provides access to runtime reflection metadata for C
structure declarations with support for arbitrarily nested combinations
of: intrinsic, set, enum, struct, union, field, array, constant, variable.
The _crefl_ clang plug-in outputs C reflection meta-data used by the library.

### crefl data types

The _crefl_ API graph database use a small number of primary data types
for the reflection database it graph nodes and their types and attributes.

| Type       | Description                                    |
| :--------- | :--------------------------------------------- |
| `decl_db`  | graph database containing the declarations     |
| `decl_ref` | reference to a single declaration graph node   |
| `decl_tag` | enumeration indicating graph node type         |
| `decl_id`  | indice of a graph node in the graph database   |
| `decl_sz`  | size type used for array size and bit widths   |
| `decl_set` | type used to indicate many-of set enumerations |

The _crefl_ data structure consists of an array of _decl_ nodes which have a
type tag, a set of attributes, an interned name, and a link to the next item.

| Type       | Properties         | Description                                    |
| :--------- | :----------------- | :--------------------------------------------- |
| tag        | `tag tag`          | tagged union node type                         |
| attributes | `set attrs`        | type specific attributes                       |
| name       | `id name`          | interned node name                             |
| next       | `id next`          | link to next item                              |

The _decl_ nodes also contain a union with type specific properties such as a
width or size quantifier or address and a link to child nodes.

| Type       | Properties         | Description                                    |
| :--------- | :----------------- | :--------------------------------------------- |
| void       |                    | empty type                                     |
| typedef    | `id decl`          | alias to another type definition               |
| intrinsic  | `sz width`         | machine type quantified with width in bits     |
| set        | `id constant`      | machine type with many-of sequence of masks    |
| enum       | `id constant`      | machine type with one-of sequence of integers  |
| struct     | `id link`          | sequence of non-overlapping types              |
| union      | `id link`          | sequence of overlapping types                  |
| field      | `id decl, sz width`| named field within struct or union             |
| array      | `id decl, sz size` | sequence of one type                           |
| constant   | `id decl, sz val`  | named constant                                 |
| function   | `id parm, sz addr` | function with input and output parameter list  |
| param      | `id decl`          | named parameter with link to next              |

### crefl implementation notes

The _crefl_ implementation is currently _alpha software_.

- intrinsic types such as integer and float are stored desugared.
- const, volatile and restrict are not yet supported.
- GNU attributes (`__attribute__`) are not yet supported.
- ~~binary format is subject to change and needs to be more compact.~~
  - format was reduced ~20% in size by eliding builtin types.
  - format could be made even smaller using LEB128 or ASN.1.
- the pointed-to type for pointers are not yet supported.
- complex number types are not yet supported.
- function addresses are not yet supported.

### crefl dependencies

tested on ubuntu 20.04 LTS:

```
sudo apt-get install llvm libclang-dev
```

### building crefl

tested on ubuntu 20.04 LTS:

```
cmake -B build -G Ninja
cmake --build build -- --verbose
```

### invoking crefl plugin

to run the crefl plugin and dump the reflection table to stdout:

```
clang test/simple-struct-1.h \
      -Xclang -load -Xclang build/libcrefl.so \
      -Xclang -plugin -Xclang crefl \
      -Xclang -plugin-arg-crefl -Xclang -dump
```

to run the crefl plugin and write the reflection data to a file:

```
clang test/simple-struct-1.h \
      -Xclang -load -Xclang build/libcrefl.so \
      -Xclang -plugin -Xclang crefl \
      -Xclang -plugin-arg-crefl -Xclang -o \
      -Xclang -plugin-arg-crefl -Xclang tmp/simple-struct-1.refl
```

to enable crefl plugin debugging, add the following option:

```
      -Xclang -plugin-arg-crefl -Xclang -debug
```
