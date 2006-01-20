#!/bin/sh
# Sets up environment to build Avida using MSYS, the MinGW shell

# Set BUILD_ROOT to the directory you build the win32 libraries in
# These will probably need to be tweaked for your environment
export BUILD_ROOT=/c/cygwin/home/josh
export QTDIR=$BUILD_ROOT/src/qt-3
export LOCAL=/c/msys/1.0
export PATH=$PATH:$QTDIR/bin:$QTDIR/lib:$BUILD_ROOT/src/gccxml/build/bin:/c/Python24:$LOCAL/lib:$LOCAL/lib/AvidaCore

export PYTHONPATH="$LOCAL/lib;$LOCAL/lib/AvidaCore"

export GCCXML_FLAGS="-D__DBL_MIN_EXP__='(-1021)' -D__FLT_MIN__='1.17549435e-3
8F' -D__CHAR_BIT__='8' -D__WCHAR_MAX__='65535U' -D__DBL_DENORM_MIN__='4.9406564
584124654e-324' -D__FLT_EVAL_METHOD__='2' -D__DBL_MIN_10_EXP__='(-307)' -D__FIN
ITE_MATH_ONLY__='0' -D__GNUC_PATCHLEVEL__='2' -D_stdcall='__attribute__((__stdc
all__))' -D__SHRT_MAX__='32767' -D__LDBL_MAX__='1.18973149535723176502e+4932L' 
-D__LDBL_MAX_EXP__='16384' -D__SCHAR_MAX__='127' -D__USER_LABEL_PREFIX__='_' -D
__STDC_HOSTED__='1' -D__WIN32='1' -D__LDBL_HAS_INFINITY__='1' -D__DBL_DIG__='15
' -D__FLT_EPSILON__='1.19209290e-7F' -D__GXX_WEAK__='1' -D__tune_i686__='1' -Df
__LDBL_MIN__='3.36210314311209350626e-4932L' -D__DECIMAL_DIG__='21' -D__LDBL_HA
S_QUIET_NAN__='1' -D__GNUC__='3' -D_cdecl='__attribute__((__cdecl__))' -D__DBL_
MAX__='1.7976931348623157e+308' -D__WINNT='1' -D__DBL_HAS_INFINITY__='1' -D__WI
NNT__='1' -D_fastcall='__attribute__((__fastcall__))' -D__cplusplus='1' -D__USI
NG_SJLJ_EXCEPTIONS__='1' -D__DEPRECATED='1' -D__DBL_MAX_EXP__='1024' -D__WIN32_
_='1' -D__GNUG__='3' -D__LONG_LONG_MAX__='9223372036854775807LL' -D__GXX_ABI_VE
RSION='1002' -D__FLT_MIN_EXP__='(-125)' -D__DBL_MIN__='2.2250738585072014e-308'
 -D__FLT_MIN_10_EXP__='(-37)' -D__DBL_HAS_QUIET_NAN__='1' -D__REGISTER_PREFIX__
='' -D__cdecl='__attribute__((__cdecl__))' -D__NO_INLINE__='1' -D__i386='1' -D_
_FLT_MANT_DIG__='24' -D__VERSION__='3.4.2 (mingw-special)' -D_WIN32='1' -D_X86_
='1' -Di386='1' -D__i386__='1' -D__SIZE_TYPE__='unsigned int' -D__FLT_RADIX__='
2' -D__LDBL_EPSILON__='1.08420217248550443401e-19L' -D__MSVCRT__='1' -D__FLT_HA
S_QUIET_NAN__='1' -D__FLT_MAX_10_EXP__='38' -D__LONG_MAX__='2147483647L' -D__FL
T_HAS_INFINITY__='1' -D__stdcall='__attribute__((__stdcall__))' -D__EXCEPTIONS=
'1' -D__LDBL_MANT_DIG__='64' -D__WCHAR_TYPE__='short unsigned int' -D__FLT_DIG_
_='6' -D__INT_MAX__='2147483647' -DWIN32='1' -D__MINGW32__='1' -D__FLT_MAX_EXP_
_='128' -D__DBL_MANT_DIG__='53' -D__WINT_TYPE__='short unsigned int' -D__LDBL_M
IN_EXP__='(-16381)' -D__WCHAR_UNSIGNED__='1' -D__LDBL_MAX_10_EXP__='4932' -D__D
BL_EPSILON__='2.2204460492503131e-16' -D__tune_pentiumpro__='1' -D__fastcall='_
_attribute__((__fastcall__))' -DWINNT='1' -D__FLT_DENORM_MIN__='1.40129846e-45F
' -D__FLT_MAX__='3.40282347e+38F' -D__GNUC_MINOR__='4' -D__DBL_MAX_10_EXP__='30
8' -D__LDBL_DENORM_MIN__='3.64519953188247460253e-4951L' -D__PTRDIFF_TYPE__='in
t' -D__LDBL_MIN_10_EXP__='(-4931)' -D__LDBL_DIG__='18' -D__declspec(x)='__attri
bute__((x))' -iwrapper c:/cygwin/home/josh/src/gccxml/GCC_XML/Support/GCC/3.4 -
I/mingw/lib/gcc/mingw32/3.4.2/../../../../include/c++ -I/mingw/include -I/mingw
/include/c++/3.4.2 -I/mingw/include/c++/3.4.2/mingw32 -I/mingw/lib/gcc/mingw32/
3.4.2/include -include c:/cygwin/home/josh/src/gccxml/GCC_XML/Support/GCC/3.4/g
ccxml_builtins.h"
