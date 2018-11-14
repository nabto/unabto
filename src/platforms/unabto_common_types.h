/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#ifndef _UNABTO_COMMON_TYPES_H_
#define _UNABTO_COMMON_TYPES_H_

// Define the compiler specific symbol for program memory variables. Default is nothing as all data is placed in RAM on larger platforms.
#ifndef __ROM
#define __ROM const
#endif

// Select whether or not to place certain variables in program memory. Default is to place everything in RAM as this works on all larger platforms.
#ifndef __USE_ROM
#define __USE_ROM 0
#endif

typedef __ROM char* text;

// Define default formatter for strings.
#ifndef PRItext
#define PRItext         "s"
#endif


#ifndef __LENGTH_MODIFIER_8
#define __LENGTH_MODIFIER_8 ""
#endif

#ifndef __LENGTH_MODIFIER_16
#define __LENGTH_MODIFIER_16 ""
#endif

#ifndef __LENGTH_MODIFIER_32
#define __LENGTH_MODIFIER_32 ""
#endif

#ifndef __LENGTH_MODIFIER_64
#define __LENGTH_MODIFIER_64 "l"
#endif

#ifndef __DECIMAL_MODIFIER
#define __DECIMAL_MODIFIER "d"
#endif

#ifndef __INTEGER_MODIFIER
#define __INTEGER_MODIFIER "i"
#endif

#ifndef __OCTAL_MODIFIER
#define __OCTAL_MODIFIER "o"
#endif

#ifndef __UNSIGNED_MODIFIER
#define __UNSIGNED_MODIFIER "u"
#endif

#ifndef __LOWER_CASE_HEX_MODIFIER
#define __LOWER_CASE_HEX_MODIFIER "x"
#endif

#ifndef __UPPER_CASE_HEX_MODIFIER
#define __UPPER_CASE_HEX_MODIFIER "X"
#endif

#ifndef PRId8
#define PRId8         __LENGTH_MODIFIER_8 __DECIMAL_MODIFIER
#endif
#ifndef PRIi8
#define PRIi8         __LENGTH_MODIFIER_8 __INTEGER_MODIFIER
#endif
#ifndef PRIo8
#define PRIo8         __LENGTH_MODIFIER_8 __OCTAL_MODIFIER
#endif
#ifndef PRIu8
#define PRIu8         __LENGTH_MODIFIER_8 __UNSIGNED_MODIFIER
#endif
#ifndef PRIx8
#define PRIx8         __LENGTH_MODIFIER_8 __LOWER_CASE_HEX_MODIFIER
#endif
#ifndef PRIX8
#define PRIX8         __LENGTH_MODIFIER_8 __UPPER_CASE_HEX_MODIFIER
#endif
#ifndef PRId16
#define PRId16        __LENGTH_MODIFIER_16 __DECIMAL_MODIFIER
#endif
#ifndef PRIi16
#define PRIi16        __LENGTH_MODIFIER_16 __INTEGER_MODIFIER
#endif
#ifndef PRIo16
#define PRIo16        __LENGTH_MODIFIER_16 __OCTAL_MODIFIER
#endif
#ifndef PRIu16
#define PRIu16        __LENGTH_MODIFIER_16 __UNSIGNED_MODIFIER
#endif
#ifndef PRIx16
#define PRIx16        __LENGTH_MODIFIER_16 __LOWER_CASE_HEX_MODIFIER
#endif
#ifndef PRIX16
#define PRIX16        __LENGTH_MODIFIER_16 __UPPER_CASE_HEX_MODIFIER
#endif

#ifndef PRId32
#define PRId32        __LENGTH_MODIFIER_32 __DECIMAL_MODIFIER
#endif
#ifndef PRIi32
#define PRIi32        __LENGTH_MODIFIER_32 __INTEGER_MODIFIER
#endif
#ifndef PRIo32
#define PRIo32        __LENGTH_MODIFIER_32 __OCTAL_MODIFIER
#endif
#ifndef PRIu32
#define PRIu32        __LENGTH_MODIFIER_32 __UNSIGNED_MODIFIER
#endif
#ifndef PRIx32
#define PRIx32        __LENGTH_MODIFIER_32 __LOWER_CASE_HEX_MODIFIER
#endif
#ifndef PRIX32
#define PRIX32        __LENGTH_MODIFIER_32 __UPPER_CASE_HEX_MODIFIER
#endif

#ifndef PRId64
#define PRId64        __LENGTH_MODIFIER_64 __DECIMAL_MODIFIER
#endif
#ifndef PRIi64
#define PRIi64        __LENGTH_MODIFIER_64 __INTEGER_MODIFIER
#endif
#ifndef PRIo64
#define PRIo64        __LENGTH_MODIFIER_64 __OCTAL_MODIFIER
#endif
#ifndef PRIu64
#define PRIu64        __LENGTH_MODIFIER_64 __UNSIGNED_MODIFIER
#endif
#ifndef PRIx64
#define PRIx64        __LENGTH_MODIFIER_64 __LOWER_CASE_HEX_MODIFIER
#endif
#ifndef PRIX64
#define PRIX64        __LENGTH_MODIFIER_64 __UPPER_CASE_HEX_MODIFIER
#endif


// define default (nabto) version of pointer specific size (any *)
#ifndef PRIptr
//For platforms without %p support, use PRIxPTR (uintptr_t) instead
#define PRIptr "p"
#endif

// define default (nabto) version of difference between pointer specific size (ptrdiff_t)
#ifndef PRIptrdiff
#define PRIptrdiff "td"
#endif

// define default (nabto) version of size_t
#ifndef PRIsize
#define PRIsize "zu"
#endif

// define default (nabto) version of PRI_index
#ifndef PRI_index
#define PRI_index PRIptrdiff
#endif

// define default version of MAKE_IP_PRINTABLE (suitable for little endian platforms)
//#ifndef MAKE_IP_PRINTABLE
//#define MAKE_IP_PRINTABLE(ip) (uint8_t)(ip >> 24), (uint8_t)(ip >> 16), (uint8_t)(ip >> 8), (uint8_t)(ip)
//#endif

#ifndef MAKE_NSI_PRINTABLE
#define MAKE_NSI_PRINTABLE(cp, sp, cont) (uint32_t)(cp), (uint32_t)(sp), (uint32_t)(cont)
#endif

//#ifndef PRIip
//#define PRIip         "%" PRIu8 ".%" PRIu8 ".%" PRIu8 ".%" PRIu8
//#endif

#ifndef PRInsi
#define PRInsi        "(%" PRIu32 ".%" PRIu32 ".%" PRIu32 ")"
#endif

#ifndef MAKE_EP_PRINTABLE
#define MAKE_EP_PRINTABLE(ep) nabto_ip_to_string(&(ep).addr), (ep).port
#endif

#ifndef PRIep
#define PRIep         "%s:%" PRIu16
#endif

#ifndef MAKE_FP_PRINTABLE
#define MAKE_FP_PRINTABLE(val) (val).data[0],(val).data[1],(val).data[2],(val).data[3]
#endif

#ifndef PRIfp
#define PRIfp         "%x:%x:%x:%x..."
#endif

#endif
