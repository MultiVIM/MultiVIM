#!/usr/bin/env ksh

# MultiVIM
# Nightly: build assistant for MultiVIM.

root_dir=$(pwd)/../../
platform_name="`uname -s`-`uname -p`"
output_dir=$(pwd)

analyse=0
clean=1
debug=1
jobs=5
run_cmake=1
SCAN_BUILD=

while getopts aA:ij:o:rR: opt
do
    case "$opt" in
     a) analyse=1;;
     A) SCAN_BUILD="$OPTARG";;
     i) clean=0 run_cmake=0;;
     j) jobs="$OPTARG";;
     r) debug=0;;
     R) root_dir="$OPTARG";;
     o) output_dir="$OPTARG";;
     \?) echo >&2 \
         "Usage: $0 [-a] [-A <scan-build>] [-i] [-R root-dir]"
         exit 1;;
    esac
done
shift $((OPTIND-1))

src_root=${root_dir}/usr/src


DATE=date
EXPR=expr
PRINTF=printf
MAKE=make
RM=rm
MKDIR="mkdir -p"


if [ -e "./nightly.opt" ]
then
    source ./nightly.opt
fi

RELEASE_VERINFO="MultiVIM ${RELEASE}"
DEBUG_VERINFO="MultiVIM Development: `$DATE +%c`"

set_opts()
{
    cmake_opts+="-DCMAKE_INSTALL_PREFIX=${inst_dir}
        -DMV_VERINFO:STRING=\"${VERINFO}\""
}

if [ ${analyse} = "1" ]
then
    SCAN_BUILD= ${SCAN_BUILD:-scan-build}
fi

if [ ${debug} = "1" ]
then
    cmake_opts+="-DCMAKE_BUILD_TYPE=Debug "
    VERINFO="${DEBUG_VERINFO}"
else
    VERINFO="${RELEASE_VERINFO}"
fi

set_opts

# $1: start time
# $2: end time
calcelapsed()
{
    elapsed=$(($2 - $1))

    $PRINTF "==== Elapsed build time: %02d:%02d\n" \
        $((elapsed / 60 % 60)) $((elapsed % 60))
}

echo "==== Building Oopsilon at `$DATE` ===="
start_time=$SECONDS

failedbuild()
{
    echo "==== Failure building Oopsilon at `$DATE` ===="
    calcelapsed $start_time $SECONDS
    exit 1
}

passedbuild()
{
    echo "==== Finished building C/FL at `$DATE` ===="
    calcelapsed $start_time $SECONDS
    exit 0
}

trap 'failedbuild' ERR

cd ${output_dir}

flags="CC=clang CXX=clang++ OBJC=clang OBJCXX=clang++"

if [ ${run_cmake} = "1" ]
then
    eval env ${SCAN_BUILD} ${flags} cmake ${cmake_opts} ${src_root}
fi

env ${flags} ${SCAN_BUILD} $MAKE -j ${jobs}

passedbuild $start_time
