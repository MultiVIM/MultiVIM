/*
 * The prelude is read before every piece of inline C. It defines the inline C
 * code's interface with the VM.
 */

#include <stddef.h>
#include <stdint.h>

#ifdef __MultiVIM_JIT
#define PACKSTRUCT __attribute__ ((PACKSTRUCT))
#else
#include "Lowlevel/MVBeginPackStruct.h"
#endif

typedef enum
{
    VT_kOop = 0,
    VT_kInt = 1,
    VT_kFloat = 2,
} VT_OopKind;

typedef enum
{
    VT_Mem_kOop = 0,
    VT_Mem_kByte = 1,
} VT_MemKind;

typedef struct PACKSTRUCT VT_MemOopDesc
{
    struct VT_MemOopDesc * _isa PACKSTRUCT;
    size_t _size : 62 PACKSTRUCT;
    VT_MemKind _kind : 2 PACKSTRUCT;
    /* Space for the object's fields. MUST BE WORD-ALIGNED! */
    union
    {
        uint8_t bytes[0] PACKSTRUCT;
        struct VT_MemOopDesc * oops[0] PACKSTRUCT;
    } _vonNeumannSpace PACKSTRUCT;
} VT_MemOopDesc;

#ifndef __MultiVIM_JIT
#include "Lowlevel/MVEndPackStruct.h"
#endif

typedef VT_MemOopDesc * VT_Oop;

#define VT_tagBits 2
#define VT_tagMask 3 /* lowest 2 bits set both */

/*typedef union PACKSTRUCT VT_Oop
{
    struct
    {
        intptr_t num : 62 PACKSTRUCT;
        uintptr_t tag : _tagBits PACKSTRUCT;
    } i PACKSTRUCT;

    struct
    {
        intptr_t flo : 32 PACKSTRUCT;
        intptr_t padding : 30 PACKSTRUCT;
        uintptr_t tag : _tagBits PACKSTRUCT;
    } f PACKSTRUCT;

    intptr_t ival PACKSTRUCT;
} VT_Oop;*/

#define VT_nilObj ((VT_Oop)0)
#define VT_tag(x) (((intptr_t)x) & VT_tagMask)
#define VT_isOop(x) (!VT_tag (x))
#define VT_isSmallInteger(x) (VT_tag (x) == VT_kInt)
#define VT_isFloat(x) (VT_tag (x) == VT_kFloat)
#define VT_isa(x) (VT_isSmallInteger (x) ? 0 :)
#define Oop_intValue(x) (((intptr_t)x) >> VT_tagBits)
#define Oop_floatValue(x) ((float)0)
#define Oop_fromInt(iVal) ((VT_Oop) (((iVal) << 2) | VT_kInt))
