cmake_minimum_required(VERSION 3.16)
cmake_policy(SET CMP0072 NEW)
project("MultiVIM" LANGUAGES C CXX OBJC OBJCXX)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMake/")

include (CMake/LemFlex.cmake)

#
# Definitions
#
set(MV_USR ${CMAKE_SOURCE_DIR}/..)
set(MV_CLOSED ${MV_USR}/closed)
set(MV_SRC ${CMAKE_SOURCE_DIR})
set(MV_TOOLS_DIR ${MV_SRC}/Tools)
set(MV_SMALLTALK_DIR ${MV_SRC}/Smalltalk)

#
# Dependencies
#
find_package(PkgConfig REQUIRED)
find_package(BoehmGC REQUIRED)

pkg_check_modules(Cairo REQUIRED IMPORTED_TARGET cairo)
pkg_check_modules(Pango REQUIRED IMPORTED_TARGET pangocairo)

#
# FLTK subbuild
#
set(OPTION_CAIRO ON CACHE BOOL "Need Cairo")
set(OPTION_CAIROEXT ON CACHE BOOL "Need Cairoext")
set(OPTION_BUILD_EXAMPLES OFF CACHE BOOL "Need not examples")
add_subdirectory(${MV_CLOSED}/fltk closed/fltk)

#
# Other subbuilds
#

add_subdirectory(${MV_CLOSED}/lemon closed/lemon)

#
# Misc targets
#
add_custom_target(format
  COMMAND
    ${MV_TOOLS_DIR}/format.ksh ${MV_SRC}
  COMMENT
		"Run Clang-Format.")

#
# Global flags
#

add_compile_definitions(FLTK_HAVE_CAIRO)

set(CMAKE_OBJC_FLAGS "${CMAKE_OBJC_FLAGS} -fobjc-arc -fobjc-arc-exceptions -fexceptions -fobjc-exceptions \
    -funwind-tables -fconstant-string-class=OFConstantString -Xclang \
    -fno-constant-cfstrings -Xclang -fobjc-runtime=objfw -Xclang -fblocks \
    -Wno-nullability-completeness")
set(CMAKE_OBJCXX_FLAGS "${CMAKE_OBJCXX_FLAGS} ${CMAKE_OBJC_FLAGS}")
set(CMAKE_OBJC_LINK_FLAGS "${CMAKE_OBJC_LINK_FLAGS} -L/usr/local/lib -lobjfw \
    -lobjfwrt  -lm -ldl -lpthread -fexceptions -fobjc-arc -fobjc-arc-exceptions")
set(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} ${CMAKE_OBJC_LINK_FLAGS}")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-attribute-packed-for-bitfield")

FlexCXXComp(Compiler/Lexer.l)
LemonCXXComp(Compiler/Parser.y)

add_executable(MultiVIM
    GUI/Controls/MVMDIWorkspace.cxx GUI/Controls/MVMDIWindow.mm
    GUI/Presenters/MVAppPresenter.mm GUI/Presenters/MVGridPresenter.mm
    GUI/Views/MVMainWindowView.mm GUI/Views/MVTextView.mm

    LGPL/EFLTK.cxx
    LGPL/Controls/Flw_Split.cxx LGPL/Controls/Fl_Scroll_Tabs.cxx
    LGPL/Controls/Fl_Drag_Tabs.cxx

    Lowlevel/MVLog.m
    "ObjFW Categories/OFMutableData+MV.m"

    "Compiler/AST/Generation.cxx" "Compiler/Compiler.cxx"
    "Compiler/AST/AST.cxx" "Compiler/AST/Printing.cxx" "Compiler/AST/Synth.cxx"
    "Compiler/CodeGen.cxx"

    "ObjectMemory/ObjectMemory.cxx"

    "Oops/Oops.cxx" "Oops/BlockOop.cxx" "Oops/ContextOop.cxx"
    "Oops/DictionaryOop.cxx" "Oops/MethodOop.cxx" "Oops/ProcessOop.cxx"
    "Oops/Printing.cxx"

    "VM/Interpreter.cxx" "VM/Primitive.cxx"

    "OldVM/VM.cxx" "OldVM/Memory.cxx" "OldVM/Compiler.cxx"
    ${CMAKE_CURRENT_BINARY_DIR}/Lexer.l.cxx
    ${CMAKE_CURRENT_BINARY_DIR}/Parser.tab.cxx


    MultiVIM.cxx OldMultiVIM.mm NeoVIMClient.mm)

target_include_directories(MultiVIM PRIVATE ${PROJECT_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/../closed)
target_include_directories(MultiVIM SYSTEM BEFORE PRIVATE
    ${CMAKE_SOURCE_DIR}/../closed/fltk
    ${CMAKE_BINARY_DIR}/closed/fltk)
target_link_libraries(MultiVIM BoehmGC::BoehmGC PkgConfig::Cairo
    PkgConfig::Pango fltk fltk_cairo fltk_images stdc++fs)

set_target_properties(MultiVIM PROPERTIES
    CXX_STANDARD 17)

#
# Smalltalk
#

add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/systemImage
    COMMAND $<TARGET_FILE:MultiVIM> -c ${MV_SMALLTALK_DIR}/Base/Image.st
    DEPENDS ${MV_SMALLTALK_DIR}/Base/Image.st)
add_custom_target(systemImage ALL DEPENDS ${MV_SMALLTALK_DIR}/Base/Image.st)
