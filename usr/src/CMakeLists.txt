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
find_package(CURL REQUIRED)
pkg_check_modules(GTKMM REQUIRED IMPORTED_TARGET gtkmm-3.0)

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

add_subdirectory(${MV_CLOSED}/litehtml closed/litehtml)
add_subdirectory(${MV_CLOSED}/lemon closed/lemon)
add_subdirectory(${MV_CLOSED}/libtcc-cmake closed/libtcc-cmake)

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

SET(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -O0")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -O0")
SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -O0")

set(CMAKE_OBJC_FLAGS "${CMAKE_OBJC_FLAGS}  -fexceptions -fobjc-exceptions \
    -funwind-tables -fconstant-string-class=OFConstantString -Xclang \
    -fno-constant-cfstrings -Xclang -fobjc-runtime=objfw -Xclang -fblocks \
    -Wno-nullability-completeness")
set(CMAKE_OBJCXX_FLAGS "${CMAKE_OBJCXX_FLAGS} ${CMAKE_OBJC_FLAGS}")
set(CMAKE_OBJC_LINK_FLAGS "${CMAKE_OBJC_LINK_FLAGS} -L/usr/local/lib -lobjfw \
    -lobjfwrt  -lm -ldl -lpthread -fexceptions ")
set(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} ${CMAKE_OBJC_LINK_FLAGS}")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-attribute-packed-for-bitfield")

FlexCXXComp(Compiler/Lexer.l)
LemonCXXComp(Compiler/Parser.y)

add_executable(MultiVIM
    GUI/Controls/MVMDIWorkspace.cxx GUI/Controls/MVMDIWindow.cxx
    GUI/Presenters/MVAppPresenter.mm GUI/Presenters/MVGridPresenter.mm
    GUI/Views/MVMainWindowView.mm GUI/Views/MVTextView.mm GUI/Views/MVWebView.cxx

    LGPL/EFLTK.cxx
    LGPL/Controls/Flw_Split.cxx LGPL/Controls/Fl_Scroll_Tabs.cxx
    LGPL/Controls/Fl_Drag_Tabs.cxx

	LGPL/Controls/Fl_Html_Formatter.cxx LGPL/Controls/Fl_Html_Object.cxx
	LGPL/Controls/Fl_Html_Parser.cxx LGPL/Controls/Fl_Html_View.cxx

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

    "BSD/WebView/http_loader.cxx"
    "WebView/container_linux.cxx" "WebView/MVWebView.cxx"

    ${CMAKE_CURRENT_BINARY_DIR}/Lexer.l.cxx
    ${CMAKE_CURRENT_BINARY_DIR}/Parser.tab.cxx


    #OldMultiVIM.mm
    #testmainhtmlview.cxx
    MultiVIM.cxx

    NeoVIMClient.mm)


target_include_directories(MultiVIM PRIVATE ${PROJECT_SOURCE_DIR} ${CMAKE_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/../closed)
target_include_directories(MultiVIM SYSTEM BEFORE PRIVATE
    ${CMAKE_SOURCE_DIR}/../closed/fltk
    ${CMAKE_BINARY_DIR}/closed/fltk)
target_link_libraries(MultiVIM PkgConfig::Cairo BoehmGC::BoehmGC PkgConfig::GTKMM
    CURL::libcurl PocoFoundation libtcc
    PkgConfig::Pango fltk fltk_cairo fltk_images stdc++fs litehtml)

set_target_properties(MultiVIM PROPERTIES
    CXX_STANDARD 17)

#
# JIT prelude
#

add_executable(ResComp ${MV_TOOLS_DIR}/ResComp.c)

set (JIT_PRELUDE_SRC ${MV_SRC}/JIT/Prelude.h)
set (JIT_PRELUDE_RES ${CMAKE_BINARY_DIR}/JITPrelude.rh)
add_custom_command(OUTPUT ${JIT_PRELUDE_RES}
    COMMAND $<TARGET_FILE:ResComp>
    ARGS JITPrelude ${JIT_PRELUDE_SRC}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    DEPENDS ${JIT_PRELUDE_SRC})
add_custom_target(jit_prelude ALL DEPENDS ${JIT_PRELUDE_RES})

#
# Resources
#

set (HTMLDIR ${CMAKE_SOURCE_DIR}/Resources/HTML)
set (STYLEDIR ${HTMLDIR}/Less)
set (STYLECSS ${HTMLDIR}/Style.css)
set (STYLELESS ${STYLEDIR}/Style.less)
add_custom_command(OUTPUT ${STYLECSS}
    COMMAND lessc ${STYLELESS} > ${STYLECSS} 
    DEPENDS ${STYLELESS} ${STYLEDIR}/buttons.less ${STYLEDIR}/dropdowns.less
      ${STYLEDIR}/button-groups.less
      ${STYLEDIR}/forms.less ${STYLEDIR}/patterns.less
      ${STYLEDIR}/preboot.less ${STYLEDIR}/reset.less
      ${STYLEDIR}/scaffolding.less ${STYLEDIR}/tables.less
      ${STYLEDIR}/type.less)
add_custom_target(css ALL DEPENDS ${STYLECSS})