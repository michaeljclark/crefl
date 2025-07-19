/*
 * cinf runtime library and compiler plug-in to support reflection in C.
 *
 * Copyright (c) 2020-2022 Michael Clark <michaeljclark@mac.com>
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

#include <cinf/model.h>
#include <cinf/types.h>

const decl_type type_cvoid =     { decl_intrinsic,  0,  decl_void   | decl_pad_bit,  "void"    };
const decl_type type_cbool =     { decl_intrinsic,  1,  decl_int    | decl_pad_byte, "bool"    };
const decl_type type_uint1 =     { decl_intrinsic,  1,  decl_uint   | decl_pad_pow2, "bit"     };
const decl_type type_int1 =      { decl_intrinsic,  1,  decl_int    | decl_pad_pow2, "sign"    };
const decl_type type_uint8 =     { decl_intrinsic,  8,  decl_uint   | decl_pad_pow2, "ubyte"   };
const decl_type type_int8 =      { decl_intrinsic,  8,  decl_int    | decl_pad_pow2, "byte"    };
const decl_type type_uint16 =    { decl_intrinsic, 16,  decl_uint   | decl_pad_pow2, "ushort"  };
const decl_type type_int16 =     { decl_intrinsic, 16,  decl_int    | decl_pad_pow2, "short"   };
const decl_type type_uint32 =    { decl_intrinsic, 32,  decl_uint   | decl_pad_pow2, "uint"    };
const decl_type type_int32 =     { decl_intrinsic, 32,  decl_int    | decl_pad_pow2, "int"     };
const decl_type type_uint64 =    { decl_intrinsic, 64,  decl_uint   | decl_pad_pow2, "ulong"   };
const decl_type type_int64 =     { decl_intrinsic, 64,  decl_int    | decl_pad_pow2, "long"    };
const decl_type type_uint128 =   { decl_intrinsic, 128, decl_uint   | decl_pad_pow2, "ucent"   };
const decl_type type_int128 =    { decl_intrinsic, 128, decl_int    | decl_pad_pow2, "cent"    };
const decl_type type_float16 =   { decl_intrinsic, 16,  decl_float  | decl_pad_pow2, "half"    };
const decl_type type_float32 =   { decl_intrinsic, 32,  decl_float  | decl_pad_pow2, "float"   };
const decl_type type_float64 =   { decl_intrinsic, 64,  decl_float  | decl_pad_pow2, "double"  };
const decl_type type_float128 =  { decl_intrinsic, 128, decl_float  | decl_pad_pow2, "quad"    };
const decl_type type_cfloat16 =  { decl_intrinsic, 32,  decl_cfloat | decl_pad_pow2, "chalf"   };
const decl_type type_cfloat32 =  { decl_intrinsic, 64,  decl_cfloat | decl_pad_pow2, "cfloat"  };
const decl_type type_cfloat64 =  { decl_intrinsic, 128, decl_cfloat | decl_pad_pow2, "cdouble" };
const decl_type type_cfloat128 = { decl_intrinsic, 256, decl_cfloat | decl_pad_pow2, "cquad"   };

const decl_type* type_vec2h_el[] = { &type_float16, &type_float16, 0 };
const decl_type* type_vec2f_el[] = { &type_float32, &type_float32, 0 };
const decl_type* type_vec2d_el[] = { &type_float64, &type_float64, 0 };
const decl_type* type_vec2t_el[] = { &type_float128,&type_float128,0 };
const decl_type* type_vec3h_el[] = { &type_float16, &type_float16, &type_float16, 0 };
const decl_type* type_vec3f_el[] = { &type_float32, &type_float32, &type_float32, 0 };
const decl_type* type_vec3d_el[] = { &type_float64, &type_float64, &type_float64, 0 };
const decl_type* type_vec3t_el[] = { &type_float128,&type_float128,&type_float128,0 };
const decl_type* type_vec4h_el[] = { &type_float16, &type_float16, &type_float16, &type_float16, 0 };
const decl_type* type_vec4f_el[] = { &type_float32, &type_float32, &type_float32, &type_float32, 0 };
const decl_type* type_vec4d_el[] = { &type_float64, &type_float64, &type_float64, &type_float64, 0 };
const decl_type* type_vec4t_el[] = { &type_float128,&type_float128,&type_float128,&type_float128,0 };

const decl_type type_vec2h = { decl_struct, 32,  decl_pad_pow2, "vec2h", type_vec2h_el };
const decl_type type_vec2f = { decl_struct, 64,  decl_pad_pow2, "vec2f", type_vec2f_el };
const decl_type type_vec2d = { decl_struct, 128, decl_pad_pow2, "vec2d", type_vec2d_el };
const decl_type type_vec2t = { decl_struct, 256, decl_pad_pow2, "vec2t", type_vec2t_el };
const decl_type type_vec3h = { decl_struct, 48,  decl_pad_pow2, "vec3h", type_vec3h_el };
const decl_type type_vec3f = { decl_struct, 96,  decl_pad_pow2, "vec3f", type_vec3f_el };
const decl_type type_vec3d = { decl_struct, 192, decl_pad_pow2, "vec3d", type_vec3d_el };
const decl_type type_vec3t = { decl_struct, 384, decl_pad_pow2, "vec3t", type_vec3t_el };
const decl_type type_vec4h = { decl_struct, 64,  decl_pad_pow2, "vec4h", type_vec4h_el };
const decl_type type_vec4f = { decl_struct, 128, decl_pad_pow2, "vec4f", type_vec4f_el };
const decl_type type_vec4d = { decl_struct, 256, decl_pad_pow2, "vec4d", type_vec4d_el };
const decl_type type_vec4t = { decl_struct, 512, decl_pad_pow2, "vec4t", type_vec4t_el };

const decl_type* type_vec2b_el[] = { &type_int8,  &type_int8,  0 };
const decl_type* type_vec2s_el[] = { &type_int16, &type_int16, 0 };
const decl_type* type_vec2i_el[] = { &type_int32, &type_int32, 0 };
const decl_type* type_vec2l_el[] = { &type_int64, &type_int64, 0 };
const decl_type* type_vec3b_el[] = { &type_int8,  &type_int8,  &type_int8,  0 };
const decl_type* type_vec3s_el[] = { &type_int16, &type_int16, &type_int16, 0 };
const decl_type* type_vec3i_el[] = { &type_int32, &type_int32, &type_int32, 0 };
const decl_type* type_vec3l_el[] = { &type_int64, &type_int64, &type_int64, 0 };
const decl_type* type_vec4b_el[] = { &type_int8,  &type_int8,  &type_int8,  &type_int8,  0 };
const decl_type* type_vec4s_el[] = { &type_int16, &type_int16, &type_int16, &type_int16, 0 };
const decl_type* type_vec4i_el[] = { &type_int32, &type_int32, &type_int32, &type_int32, 0 };
const decl_type* type_vec4l_el[] = { &type_int64, &type_int64, &type_int64, &type_int64, 0 };

const decl_type type_vec2b = { decl_struct, 16,  decl_pad_pow2, "vec2b", type_vec2b_el };
const decl_type type_vec2s = { decl_struct, 32,  decl_pad_pow2, "vec2s", type_vec2s_el };
const decl_type type_vec2i = { decl_struct, 64,  decl_pad_pow2, "vec2i", type_vec2i_el };
const decl_type type_vec2l = { decl_struct, 128, decl_pad_pow2, "vec2l", type_vec2l_el };
const decl_type type_vec3b = { decl_struct, 24,  decl_pad_pow2, "vec3b", type_vec3b_el };
const decl_type type_vec3s = { decl_struct, 48,  decl_pad_pow2, "vec3s", type_vec3s_el };
const decl_type type_vec3i = { decl_struct, 96,  decl_pad_pow2, "vec3i", type_vec3i_el };
const decl_type type_vec3l = { decl_struct, 192, decl_pad_pow2, "vec3l", type_vec3l_el };
const decl_type type_vec4b = { decl_struct, 32,  decl_pad_pow2, "vec4b", type_vec4b_el };
const decl_type type_vec4s = { decl_struct, 64,  decl_pad_pow2, "vec4s", type_vec4s_el };
const decl_type type_vec4i = { decl_struct, 128, decl_pad_pow2, "vec4i", type_vec4i_el };
const decl_type type_vec4l = { decl_struct, 256, decl_pad_pow2, "vec4l", type_vec4l_el };

const decl_type* type_vec2ub_el[] = { &type_uint8,  &type_uint8,  0 };
const decl_type* type_vec2us_el[] = { &type_uint16, &type_uint16, 0 };
const decl_type* type_vec2ui_el[] = { &type_uint32, &type_uint32, 0 };
const decl_type* type_vec2ul_el[] = { &type_uint64, &type_uint64, 0 };
const decl_type* type_vec3ub_el[] = { &type_uint8,  &type_uint8,  &type_uint8,  0 };
const decl_type* type_vec3us_el[] = { &type_uint16, &type_uint16, &type_uint16, 0 };
const decl_type* type_vec3ui_el[] = { &type_uint32, &type_uint32, &type_uint32, 0 };
const decl_type* type_vec3ul_el[] = { &type_uint64, &type_uint64, &type_uint64, 0 };
const decl_type* type_vec4ub_el[] = { &type_uint8,  &type_uint8,  &type_uint8,  &type_uint8,  0 };
const decl_type* type_vec4us_el[] = { &type_uint16, &type_uint16, &type_uint16, &type_uint16, 0 };
const decl_type* type_vec4ui_el[] = { &type_uint32, &type_uint32, &type_uint32, &type_uint32, 0 };
const decl_type* type_vec4ul_el[] = { &type_uint64, &type_uint64, &type_uint64, &type_uint64, 0 };

const decl_type type_vec2ub = { decl_struct, 16,  decl_pad_pow2, "vec2ub", type_vec2ub_el };
const decl_type type_vec2us = { decl_struct, 32,  decl_pad_pow2, "vec2us", type_vec2us_el };
const decl_type type_vec2ui = { decl_struct, 64,  decl_pad_pow2, "vec2ui", type_vec2ui_el };
const decl_type type_vec2ul = { decl_struct, 128, decl_pad_pow2, "vec2ul", type_vec2ul_el };
const decl_type type_vec3ub = { decl_struct, 24,  decl_pad_pow2, "vec3ub", type_vec3ub_el };
const decl_type type_vec3us = { decl_struct, 48,  decl_pad_pow2, "vec3us", type_vec3us_el };
const decl_type type_vec3ui = { decl_struct, 96,  decl_pad_pow2, "vec3ui", type_vec3ui_el };
const decl_type type_vec3ul = { decl_struct, 192, decl_pad_pow2, "vec3ul", type_vec3ul_el };
const decl_type type_vec4ub = { decl_struct, 32,  decl_pad_pow2, "vec4ub", type_vec4ub_el };
const decl_type type_vec4us = { decl_struct, 64,  decl_pad_pow2, "vec4us", type_vec4us_el };
const decl_type type_vec4ui = { decl_struct, 128, decl_pad_pow2, "vec4ui", type_vec4ui_el };
const decl_type type_vec4ul = { decl_struct, 256, decl_pad_pow2, "vec4ul", type_vec4ul_el };

const decl_type *all_types[] = {
    &type_cvoid,    &type_cbool,
    &type_uint1,    &type_int1,     &type_uint8,    &type_int8,
    &type_uint16,   &type_int16,    &type_uint32,   &type_int32,
    &type_uint64,   &type_int64,    &type_uint128,  &type_int128,
    &type_float16,  &type_float32,  &type_float64,  &type_float128,
    &type_cfloat16, &type_cfloat32, &type_cfloat64, &type_cfloat128,
#if 0
    &type_vec2h,    &type_vec2f,    &type_vec2d,    &type_vec2t,
    &type_vec3h,    &type_vec3f,    &type_vec3d,    &type_vec3t,
    &type_vec4h,    &type_vec4f,    &type_vec4d,    &type_vec4t,
    &type_vec2b,    &type_vec2s,    &type_vec2i,    &type_vec2l,
    &type_vec3b,    &type_vec3s,    &type_vec3i,    &type_vec3l,
    &type_vec4b,    &type_vec4s,    &type_vec4i,    &type_vec4l,
    &type_vec2ub,   &type_vec2us,   &type_vec2ui,   &type_vec2ul,
    &type_vec3ub,   &type_vec3us,   &type_vec3ui,   &type_vec3ul,
    &type_vec4ub,   &type_vec4us,   &type_vec4ui,   &type_vec4ul,
#endif
    0
};
