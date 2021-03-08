/*
 * crefl runtime library and compiler plug-in to support reflection in C.
 *
 * Copyright (c) 2020 Michael Clark <michaeljclark@mac.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "cmodel.h"
#include "ctypes.h"

const _ctype _cvoid =     { _decl_intrinsic,  0, _void   | _pad_bit,  "void"    };
const _ctype _cbool =     { _decl_intrinsic,  1, _sint   | _pad_byte, "bool"    };
const _ctype _uint1 =     { _decl_intrinsic,  1, _uint   | _pad_pow2, "bit"     };
const _ctype _sint1 =     { _decl_intrinsic,  1, _sint   | _pad_pow2, "sign"    };
const _ctype _uint8 =     { _decl_intrinsic,  8, _uint   | _pad_pow2, "ubyte"   };
const _ctype _sint8 =     { _decl_intrinsic,  8, _sint   | _pad_pow2, "byte"    };
const _ctype _uint16 =    { _decl_intrinsic, 16, _uint   | _pad_pow2, "ushort"  };
const _ctype _sint16 =    { _decl_intrinsic, 16, _sint   | _pad_pow2, "short"   };
const _ctype _uint32 =    { _decl_intrinsic, 32, _uint   | _pad_pow2, "uint"    };
const _ctype _sint32 =    { _decl_intrinsic, 32, _sint   | _pad_pow2, "int"     };
const _ctype _uint64 =    { _decl_intrinsic, 64, _uint   | _pad_pow2, "ulong"   };
const _ctype _sint64 =    { _decl_intrinsic, 64, _sint   | _pad_pow2, "long"    };
const _ctype _uint128 =   { _decl_intrinsic, 128,_uint   | _pad_pow2, "ucent"   };
const _ctype _sint128 =   { _decl_intrinsic, 128,_sint   | _pad_pow2, "cent"    };
const _ctype _float16 =   { _decl_intrinsic, 16, _float  | _pad_pow2, "half"    };
const _ctype _float32 =   { _decl_intrinsic, 32, _float  | _pad_pow2, "float"   };
const _ctype _float64 =   { _decl_intrinsic, 64, _float  | _pad_pow2, "double"  };
const _ctype _float128 =  { _decl_intrinsic, 128,_float  | _pad_pow2, "quad"    };
const _ctype _cfloat16 =  { _decl_intrinsic, 32, _cfloat | _pad_pow2, "chalf"   };
const _ctype _cfloat32 =  { _decl_intrinsic, 64, _cfloat | _pad_pow2, "cfloat"  };
const _ctype _cfloat64 =  { _decl_intrinsic, 128,_cfloat | _pad_pow2, "cdouble" };
const _ctype _cfloat128 = { _decl_intrinsic, 256,_cfloat | _pad_pow2, "cquad"   };

const _ctype* _vec2h_el[] = { &_float16, &_float16, 0 };
const _ctype* _vec2f_el[] = { &_float32, &_float32, 0 };
const _ctype* _vec2d_el[] = { &_float64, &_float64, 0 };
const _ctype* _vec2t_el[] = { &_float128,&_float128,0 };
const _ctype* _vec3h_el[] = { &_float16, &_float16, &_float16, 0 };
const _ctype* _vec3f_el[] = { &_float32, &_float32, &_float32, 0 };
const _ctype* _vec3d_el[] = { &_float64, &_float64, &_float64, 0 };
const _ctype* _vec3t_el[] = { &_float128,&_float128,&_float128,0 };
const _ctype* _vec4h_el[] = { &_float16, &_float16, &_float16, &_float16, 0 };
const _ctype* _vec4f_el[] = { &_float32, &_float32, &_float32, &_float32, 0 };
const _ctype* _vec4d_el[] = { &_float64, &_float64, &_float64, &_float64, 0 };
const _ctype* _vec4t_el[] = { &_float128,&_float128,&_float128,&_float128,0 };

const _ctype _vec2h = { _decl_struct, 32,  _pad_pow2, "vec2h", _vec2h_el };
const _ctype _vec2f = { _decl_struct, 64,  _pad_pow2, "vec2f", _vec2f_el };
const _ctype _vec2d = { _decl_struct, 128, _pad_pow2, "vec2d", _vec2d_el };
const _ctype _vec2t = { _decl_struct, 256, _pad_pow2, "vec2t", _vec2t_el };
const _ctype _vec3h = { _decl_struct, 48,  _pad_pow2, "vec3h", _vec3h_el };
const _ctype _vec3f = { _decl_struct, 96,  _pad_pow2, "vec3f", _vec3f_el };
const _ctype _vec3d = { _decl_struct, 192, _pad_pow2, "vec3d", _vec3d_el };
const _ctype _vec3t = { _decl_struct, 384, _pad_pow2, "vec3t", _vec3t_el };
const _ctype _vec4h = { _decl_struct, 64,  _pad_pow2, "vec4h", _vec4h_el };
const _ctype _vec4f = { _decl_struct, 128, _pad_pow2, "vec4f", _vec4f_el };
const _ctype _vec4d = { _decl_struct, 256, _pad_pow2, "vec4d", _vec4d_el };
const _ctype _vec4t = { _decl_struct, 512, _pad_pow2, "vec4t", _vec4t_el };

const _ctype* _vec2b_el[] = { &_sint8,  &_sint8,  0 };
const _ctype* _vec2s_el[] = { &_sint16, &_sint16, 0 };
const _ctype* _vec2i_el[] = { &_sint32, &_sint32, 0 };
const _ctype* _vec2l_el[] = { &_sint64, &_sint64, 0 };
const _ctype* _vec3b_el[] = { &_sint8,  &_sint8,  &_sint8,  0 };
const _ctype* _vec3s_el[] = { &_sint16, &_sint16, &_sint16, 0 };
const _ctype* _vec3i_el[] = { &_sint32, &_sint32, &_sint32, 0 };
const _ctype* _vec3l_el[] = { &_sint64, &_sint64, &_sint64, 0 };
const _ctype* _vec4b_el[] = { &_sint8,  &_sint8,  &_sint8,  &_sint8,  0 };
const _ctype* _vec4s_el[] = { &_sint16, &_sint16, &_sint16, &_sint16, 0 };
const _ctype* _vec4i_el[] = { &_sint32, &_sint32, &_sint32, &_sint32, 0 };
const _ctype* _vec4l_el[] = { &_sint64, &_sint64, &_sint64, &_sint64, 0 };

const _ctype _vec2b = { _decl_struct, 16,  _pad_pow2, "vec2b", _vec2b_el };
const _ctype _vec2s = { _decl_struct, 32,  _pad_pow2, "vec2s", _vec2s_el };
const _ctype _vec2i = { _decl_struct, 64,  _pad_pow2, "vec2i", _vec2i_el };
const _ctype _vec2l = { _decl_struct, 128, _pad_pow2, "vec2l", _vec2l_el };
const _ctype _vec3b = { _decl_struct, 24,  _pad_pow2, "vec3b", _vec3b_el };
const _ctype _vec3s = { _decl_struct, 48,  _pad_pow2, "vec3s", _vec3s_el };
const _ctype _vec3i = { _decl_struct, 96,  _pad_pow2, "vec3i", _vec3i_el };
const _ctype _vec3l = { _decl_struct, 192, _pad_pow2, "vec3l", _vec3l_el };
const _ctype _vec4b = { _decl_struct, 32,  _pad_pow2, "vec4b", _vec4b_el };
const _ctype _vec4s = { _decl_struct, 64,  _pad_pow2, "vec4s", _vec4s_el };
const _ctype _vec4i = { _decl_struct, 128, _pad_pow2, "vec4i", _vec4i_el };
const _ctype _vec4l = { _decl_struct, 256, _pad_pow2, "vec4l", _vec4l_el };

const _ctype* _vec2ub_el[] = { &_uint8,  &_uint8,  0 };
const _ctype* _vec2us_el[] = { &_uint16, &_uint16, 0 };
const _ctype* _vec2ui_el[] = { &_uint32, &_uint32, 0 };
const _ctype* _vec2ul_el[] = { &_uint64, &_uint64, 0 };
const _ctype* _vec3ub_el[] = { &_uint8,  &_uint8,  &_uint8,  0 };
const _ctype* _vec3us_el[] = { &_uint16, &_uint16, &_uint16, 0 };
const _ctype* _vec3ui_el[] = { &_uint32, &_uint32, &_uint32, 0 };
const _ctype* _vec3ul_el[] = { &_uint64, &_uint64, &_uint64, 0 };
const _ctype* _vec4ub_el[] = { &_uint8,  &_uint8,  &_uint8,  &_uint8,  0 };
const _ctype* _vec4us_el[] = { &_uint16, &_uint16, &_uint16, &_uint16, 0 };
const _ctype* _vec4ui_el[] = { &_uint32, &_uint32, &_uint32, &_uint32, 0 };
const _ctype* _vec4ul_el[] = { &_uint64, &_uint64, &_uint64, &_uint64, 0 };

const _ctype _vec2ub = { _decl_struct, 16,  _pad_pow2, "vec2ub", _vec2ub_el };
const _ctype _vec2us = { _decl_struct, 32,  _pad_pow2, "vec2us", _vec2us_el };
const _ctype _vec2ui = { _decl_struct, 64,  _pad_pow2, "vec2ui", _vec2ui_el };
const _ctype _vec2ul = { _decl_struct, 128, _pad_pow2, "vec2ul", _vec2ul_el };
const _ctype _vec3ub = { _decl_struct, 24,  _pad_pow2, "vec3ub", _vec3ub_el };
const _ctype _vec3us = { _decl_struct, 48,  _pad_pow2, "vec3us", _vec3us_el };
const _ctype _vec3ui = { _decl_struct, 96,  _pad_pow2, "vec3ui", _vec3ui_el };
const _ctype _vec3ul = { _decl_struct, 192, _pad_pow2, "vec3ul", _vec3ul_el };
const _ctype _vec4ub = { _decl_struct, 32,  _pad_pow2, "vec4ub", _vec4ub_el };
const _ctype _vec4us = { _decl_struct, 64,  _pad_pow2, "vec4us", _vec4us_el };
const _ctype _vec4ui = { _decl_struct, 128, _pad_pow2, "vec4ui", _vec4ui_el };
const _ctype _vec4ul = { _decl_struct, 256, _pad_pow2, "vec4ul", _vec4ul_el };

const _ctype *all_types[] = {
    &_cvoid,        &_cbool,
    &_uint1,        &_sint1,        &_uint8,        &_sint8,
    &_uint16,       &_sint16,       &_uint32,       &_sint32,
    &_uint64,       &_sint64,       &_uint128,      &_sint128,
    &_float16,      &_float32,      &_float64,      &_float128,
    &_cfloat16,     &_cfloat32,     &_cfloat64,     &_cfloat128,
#if 0
    &_vec2h,        &_vec2f,        &_vec2d,        &_vec2t,
    &_vec3h,        &_vec3f,        &_vec3d,        &_vec3t,
    &_vec4h,        &_vec4f,        &_vec4d,        &_vec4t,
    &_vec2b,        &_vec2s,        &_vec2i,        &_vec2l,
    &_vec3b,        &_vec3s,        &_vec3i,        &_vec3l,
    &_vec4b,        &_vec4s,        &_vec4i,        &_vec4l,
    &_vec2ub,       &_vec2us,       &_vec2ui,       &_vec2ul,
    &_vec3ub,       &_vec3us,       &_vec3ui,       &_vec3ul,
    &_vec4ub,       &_vec4us,       &_vec4ui,       &_vec4ul,
#endif
    0
};
