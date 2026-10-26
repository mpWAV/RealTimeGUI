cmake_minimum_required(VERSION 3.14)
project(RealTimeTestGUI)

#find_package(MKL CONFIG REQUIRED)

set(LINKLIBS)
set(COMPILE_OPTION)
set(SRC)
set(INCL)
set(SRC_Qt_MOC)
set(SRC_Qt)

include(${CMAKE_CURRENT_LIST_DIR}/lib/STFT/cpp/STFT_cpp.cmake)

include(${CMAKE_CURRENT_LIST_DIR}/lib/RtAudioWrapper/RtAudio.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/lib/WAV/WAV.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/lib/KJsonConfig/KJsonConfig.cmake)

macro(display_elements
	DISPLAY_MESSAGE
	LIST_TO_DISPLAY)
    message("[ ${DISPLAY_MESSAGE} ]")
    foreach(_DISPLAY_ELEMENT ${LIST_TO_DISPLAY})
		message(STATUS ${_DISPLAY_ELEMENT})
    endforeach()
endmacro()

file(MAKE_DIRECTORY ${PROJECT_SOURCE_DIR}/tmp)

## MOC(Meta Object Compiler) of QtFramework
#display_elements(MOC_TARGET ${SRC_Qt_MOC})
	## sources to be processed by MOC
	list(APPEND SRC_Qt_MOC
		${PROJECT_SOURCE_DIR}/src/app.h
		${PROJECT_SOURCE_DIR}/src/WidgetLog.h
		${PROJECT_SOURCE_DIR}/src/WidgetConfig.h
		${PROJECT_SOURCE_DIR}/src/processor.h
	)	
	
	## sources without MetaObject 
	list(APPEND SRC_Qt
		${PROJECT_SOURCE_DIR}/src/app.cpp
		${PROJECT_SOURCE_DIR}/src/WidgetConfig.cpp
		${PROJECT_SOURCE_DIR}/src/WidgetLog.cpp
		${PROJECT_SOURCE_DIR}/src/processor.cpp
	)
	
	foreach(i  ${SRC_Qt_MOC})
		string(REPLACE "/" ";" parse_i_1 ${i})
		LIST(GET parse_i_1 -1 parse_i_2)
		string(REPLACE "." ";" parse_i_3 ${parse_i_2})
		LIST(GET parse_i_3 0 fisrt_element)
		
		if(MSVC)
			execute_process(
			COMMAND moc ${i} -o ${PROJECT_SOURCE_DIR}/tmp/moc_${fisrt_element}.cpp
			WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/bin  
			 )
		endif()
		
		list(APPEND SRC_Qt
			tmp/moc_${fisrt_element}.cpp
		)
	endforeach()

################



list(APPEND SRC
	${CMAKE_CURRENT_LIST_DIR}/src/main.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/mpNC.h
)

list(APPEND INCL
	${CMAKE_CURRENT_LIST_DIR}/src
	${CMAKE_CURRENT_LIST_DIR}/tmp
	# QtFramework
	bin/include
	bin/include/QtCore
	bin/include/QtCore5Compat
	bin/include/QtGui
	bin/include/QtWidgets
	bin/include/QtOpenGL
	bin/include/QtOpenGLWidgets
)

if(NOT MSVC)
	list(APPEND LINKLIBS
	# <math.h>
	m
	# openMP
    -fopenmp
	)
else()
	list(APPEND LINKLIBS
	
	debug ${PROJECT_SOURCE_DIR}/bin/lib/Qt6Cored.lib optimized ${PROJECT_SOURCE_DIR}/bin/lib/Qt6Core.lib
	debug ${PROJECT_SOURCE_DIR}/bin/lib/Qt6Guid.lib optimized ${PROJECT_SOURCE_DIR}/bin/lib/Qt6Gui.lib
	debug ${PROJECT_SOURCE_DIR}/bin/lib/Qt6Widgetsd.lib optimized ${PROJECT_SOURCE_DIR}/bin/lib/Qt6Widgets.lib	
	debug ${PROJECT_SOURCE_DIR}/bin/lib/Qt6OpenGLWidgetsd.lib optimized ${PROJECT_SOURCE_DIR}/bin/lib/Qt6OpenGLWidgets.lib	

	)	
endif()


if(NOT MSVC)
	list(APPEND COMPILE_OPTION
	# openMP
	#	  -fopenmp
	# debugging
	  -g
	# -O3 : maximize performace
	# -O2 : maximize performance as long as not increasing size of executable
	# -O1 : minimize compile time and others are secondary
	# -Os : minimize size of executable
	  -O3
	)
	
else()
list(APPEND COMPILE_OPTION
# Visual Studio uses '/' for compile option

	  /openmp
	  # /MP - compiled in mult-process : faster build time
	  /MP
	)
endif()

display_elements(COMPILE_OPTION "${COMPILE_OPTION}")
display_elements(LINKLIBS "${LINKLIBS}")

add_executable(${PROJECT_NAME} 
	${SRC}
	${SRC_Qt_MOC}
	${SRC_Qt}
	)


target_include_directories(${PROJECT_NAME}	PRIVATE	${INCL}  )
target_link_libraries(${PROJECT_NAME} ${LINKLIBS}
	${CMAKE_CURRENT_LIST_DIR}/bin/IR_Link_mpNC.lib 
)
if(MSVC)
	target_compile_options(${PROJECT_NAME} PRIVATE ${COMPILE_OPTION})
else()
	target_compile_options(${PROJECT_NAME} PRIVATE ${COMPILE_OPTION})
endif()
target_compile_options(${PROJECT_NAME} PRIVATE ${COMPILE_OPTION} )

set_target_properties(${PROJECT_NAME} PROPERTIES  CXX_STANDARD 17)

# ---------------------------------------------------------------------------
#  실행에 필요한 것을 exe 옆(bin/Release, bin/Debug)에 배치한다.
#
#  Windows 는 DLL 을 exe 디렉터리에서 먼저 찾는다. 원본은 bin/ 에 있으므로
#  복사하지 않으면 "DLL 을 찾을 수 없습니다" 가 뜬다.
#
#  Qt DLL 은 Debug 에서 이름 뒤에 d 가 붙는다 (Qt6Cored.dll). 구성에 따라
#  고르기 위해 제네릭 표현식을 쓴다.
#
#  config/io.json 은 app.cpp 가 ../config/io.json 으로 읽으므로 exe 디렉터리의
#  한 단계 위(bin/config/)에 둔다. 마스터는 RealTimeGUI/config/ 이고 여기로
#  복사되는 것이므로, 장치 번호를 바꿀 때는 마스터를 고칠 것 - bin/config/ 를
#  고치면 다음 빌드에 덮어써진다.
# ---------------------------------------------------------------------------
set(QTD "$<$<CONFIG:Debug>:d>")
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
		${CMAKE_CURRENT_LIST_DIR}/bin/Qt6Core${QTD}.dll
		${CMAKE_CURRENT_LIST_DIR}/bin/Qt6Gui${QTD}.dll
		${CMAKE_CURRENT_LIST_DIR}/bin/Qt6Widgets${QTD}.dll
		${CMAKE_CURRENT_LIST_DIR}/bin/IR_Link_mpNC.dll
		${CMAKE_CURRENT_LIST_DIR}/bin/onnxruntime.dll
		${CMAKE_CURRENT_LIST_DIR}/bin/onnxruntime_providers_shared.dll
		$<TARGET_FILE_DIR:${PROJECT_NAME}>
	COMMAND ${CMAKE_COMMAND} -E make_directory
		$<TARGET_FILE_DIR:${PROJECT_NAME}>/platforms
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
		${CMAKE_CURRENT_LIST_DIR}/bin/platforms/qwindows${QTD}.dll
		$<TARGET_FILE_DIR:${PROJECT_NAME}>/platforms
	COMMAND ${CMAKE_COMMAND} -E make_directory
		${CMAKE_BINARY_DIR}/config
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
		${CMAKE_CURRENT_LIST_DIR}/config/io.json
		${CMAKE_BINARY_DIR}/config
	COMMENT "Copying Qt / mpNC / onnxruntime DLLs and config next to the exe"
	)

# F5 로 실행할 때도 ../config 가 맞도록 작업 디렉터리를 exe 쪽으로 맞춘다.
set_target_properties(${PROJECT_NAME} PROPERTIES
	VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIG>
	)


