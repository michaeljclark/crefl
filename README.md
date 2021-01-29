# crefl

> _crefl_ is a runtime library and compiler plug-in to support reflection in C.

The _crefl_ library provides an interface to allow runtime reflection on C
interface declarations with support for arbitrarily nested combination of
intrinsic, struct, union, enum, array and function declarations. The _crefl_
clang compiler plug-in outputs C reflection meta-data used by the library.

### crefl data types

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
| variable   | `id decl, sz addr` | named variable that is unique to each thread   |
| uniform    | `id decl, sz addr` | named variable that is uniform across threads  |
| function   | `id parm, sz addr` | function with input and output parameter list  |
| param      | `id decl`          | named parameter with link to next              |

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

tested on ubuntu 20.04 LTS:

```
clang test/h1.h \
      -Xclang -load -Xclang build/libcrefl.so \
      -Xclang -plugin -Xclang crefl
clang test/h2.h \
      -Xclang -load -Xclang build/libcrefl.so \
      -Xclang -plugin -Xclang crefl
```
