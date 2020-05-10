set(FLEX_EXECUTABLE flex)

function (FlexComp path)
GET_FILENAME_COMPONENT (file ${path} NAME_WE)
add_custom_command(
OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${file}.l.h
	${CMAKE_CURRENT_BINARY_DIR}/${file}.l.c
COMMAND ${FLEX_EXECUTABLE}
ARGS -o${CMAKE_CURRENT_BINARY_DIR}/${file}.l.c
    --header-file=${CMAKE_CURRENT_BINARY_DIR}/${file}.l.h
    ${CMAKE_CURRENT_SOURCE_DIR}/${path}
DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${path}
)
endfunction (FlexComp)

function (FlexCXXComp path)
GET_FILENAME_COMPONENT (file ${path} NAME_WE)
add_custom_command(
OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${file}.l.h
	${CMAKE_CURRENT_BINARY_DIR}/${file}.l.cxx
COMMAND ${FLEX_EXECUTABLE}
ARGS -o${CMAKE_CURRENT_BINARY_DIR}/${file}.l.cxx
    --header-file=${CMAKE_CURRENT_BINARY_DIR}/${file}.l.h
    ${CMAKE_CURRENT_SOURCE_DIR}/${path}
DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${path}
)
endfunction (FlexCXXComp)

function (LemonComp path)
GET_FILENAME_COMPONENT (file ${path} NAME_WE)
add_custom_command(
OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${file}.tab.h
    ${CMAKE_CURRENT_BINARY_DIR}/${file}.tab.c
	${CMAKE_CURRENT_BINARY_DIR}/${file}.out
COMMAND $<TARGET_FILE:lemon>
ARGS ${CMAKE_CURRENT_SOURCE_DIR}/${path} 
	-d. -p -T${CMAKE_SOURCE_DIR}/../closed/lemon/lempar.tpl
DEPENDS $<TARGET_FILE:lemon>
MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/${path}
WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
endfunction (LemonComp)

function (LemonCXXComp path)
GET_FILENAME_COMPONENT (file ${path} NAME_WE)
add_custom_command(
OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${file}.tab.h
    ${CMAKE_CURRENT_BINARY_DIR}/${file}.tab.cxx
	${CMAKE_CURRENT_BINARY_DIR}/${file}.out
COMMAND $<TARGET_FILE:lemonxx>
ARGS ${CMAKE_CURRENT_SOURCE_DIR}/${path} 
	-d. -p -T${CMAKE_SOURCE_DIR}/../closed/lemon/lemxx.tpl
DEPENDS $<TARGET_FILE:lemon>
MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/${path}
WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
endfunction (LemonCXXComp)