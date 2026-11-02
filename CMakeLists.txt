cmake_minimum_required(VERSION 3.14)
project(RealTimeTestGUI)

#find_package(MKL CONFIG REQUIRED)

set(LINKLIBS)
set(COMPILE_OPTION)
set(SRC)
set(INCL)
set(SRC_Qt_MOC)
set(SRC_Qt)

# ---------------------------------------------------------------------------
#  Qt / mpNC 위치 선택
#
#  기본값은 기존 구성인 x64 + Qt6 (bin/ 의 QtBin 서브모듈). 아무 것도 넘기지
#  않으면 예전과 똑같이 동작한다.
#
#  win32 는 Qt6 로 빌드할 수 없다. QtBin 의 Qt 는 x64 전용이고, Qt 6 는 32비트
#  Windows 공식 바이너리를 배포하지 않는다. 그래서 win32 는 Qt 5.15 의 32비트
#  키트를 따로 받아 QT_ROOT 로 지정한다. 이 프로젝트가 쓰는 Qt API 는 전부
#  고전 QtWidgets 클래스라서 Qt5 에서도 소스 수정 없이 컴파일된다.
#
#    x64 + Qt6 (기본)
#      cmake -S . -B build -A x64
#
#    win32 + Qt5
#      cmake -S . -B build32 -A Win32 -DQT_MAJOR=5
#            -DQT_ROOT=<Qt5 키트>/5.15.2/msvc2019
#            -DMPNC_DIR=<win32 패키지>/bin
#
#  키트 배치가 버전마다 다르다. Qt6(QtBin)은 DLL 이 루트에 있고 플랫폼
#  플러그인이 platforms/ 에 있다. Qt5 공식 키트는 DLL 이 bin/ 에, 플러그인이
#  plugins/platforms/ 에 있다.
# ---------------------------------------------------------------------------
set(QT_MAJOR "6" CACHE STRING "Qt major version : 5 or 6")
set(QT_ROOT "${PROJECT_SOURCE_DIR}/bin" CACHE PATH "Qt kit root directory")
set(MPNC_DIR "${PROJECT_SOURCE_DIR}/bin" CACHE PATH "IR_Link_mpNC / onnxruntime directory")

set(QT_INC_DIR ${QT_ROOT}/include)
set(QT_LIB_DIR ${QT_ROOT}/lib)
if(QT_MAJOR STREQUAL "5")
	set(QT_DLL_DIR    ${QT_ROOT}/bin)
	set(QT_PLUGIN_DIR ${QT_ROOT}/plugins/platforms)
	set(QT_MOC        ${QT_ROOT}/bin/moc.exe)
	# Qt5 는 QOpenGLWidget 이 QtWidgets 안에 있어서 별도 모듈이 없다.
	set(QT_MODULES Core Gui Widgets)
else()
	set(QT_DLL_DIR    ${QT_ROOT})
	set(QT_PLUGIN_DIR ${QT_ROOT}/platforms)
	set(QT_MOC        ${QT_ROOT}/moc.exe)
	set(QT_MODULES Core Gui Widgets OpenGLWidgets)
endif()

# exe 옆에 복사할 Qt 런타임. 링크 대상과 다르다 - OpenGLWidgets 는 링크만 되고
# 실제로 임포트되지 않아서 복사하지 않는다.
set(QT_RUNTIME_MODULES Core Gui Widgets)

message(STATUS "QT_MAJOR = ${QT_MAJOR}")
message(STATUS "QT_ROOT  = ${QT_ROOT}")
message(STATUS "MPNC_DIR = ${MPNC_DIR}")

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

# moc 산출물은 Qt 버전별로 나눈다. 한 폴더를 공유하면 win32(Qt5) 를 구성한
# 뒤에 x64(Qt6) 를 다시 빌드할 때 Qt5 moc 산출물이 Qt6 헤더로 컴파일되면서
# 깨진다. 둘 다 .gitignore 의 tmp 아래라 추적되지 않는다.
set(MOC_DIR ${PROJECT_SOURCE_DIR}/tmp/qt${QT_MAJOR})
file(MAKE_DIRECTORY ${MOC_DIR})

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
			# moc 는 Qt 버전마다 다른 것을 써야 한다. 예전에는 이름만 'moc' 로
			# 불러서 실패해도 조용히 넘어갔고, tmp/ 에 남아 있던 이전 버전의
			# moc 산출물로 빌드가 계속됐다. 절대 경로로 부르고 실패를 에러로
			# 만든다.
			execute_process(
			COMMAND ${QT_MOC} ${i} -o ${MOC_DIR}/moc_${fisrt_element}.cpp
			COMMAND_ERROR_IS_FATAL ANY
			 )
		endif()

		list(APPEND SRC_Qt
			${MOC_DIR}/moc_${fisrt_element}.cpp
		)
	endforeach()

################



list(APPEND SRC
	${CMAKE_CURRENT_LIST_DIR}/src/main.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/mpNC.h
)

list(APPEND INCL
	${CMAKE_CURRENT_LIST_DIR}/src
	${MOC_DIR}
	# QtFramework
	${QT_INC_DIR}
	${QT_INC_DIR}/QtCore
	${QT_INC_DIR}/QtGui
	${QT_INC_DIR}/QtWidgets
)

if(NOT QT_MAJOR STREQUAL "5")
	# Qt6 전용 모듈. Qt5 에는 이 디렉터리가 없다.
	list(APPEND INCL
		${QT_INC_DIR}/QtCore5Compat
		${QT_INC_DIR}/QtOpenGL
		${QT_INC_DIR}/QtOpenGLWidgets
	)
endif()

if(NOT MSVC)
	list(APPEND LINKLIBS
	# <math.h>
	m
	# openMP
    -fopenmp
	)
else()
	foreach(m ${QT_MODULES})
		list(APPEND LINKLIBS
			debug     ${QT_LIB_DIR}/Qt${QT_MAJOR}${m}d.lib
			optimized ${QT_LIB_DIR}/Qt${QT_MAJOR}${m}.lib
		)
	endforeach()
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
	${MPNC_DIR}/IR_Link_mpNC.lib
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
#  Windows 는 DLL 을 exe 디렉터리에서 먼저 찾는다. 원본은 QT_DLL_DIR / MPNC_DIR
#  에 있으므로 복사하지 않으면 "DLL 을 찾을 수 없습니다" 가 뜬다.
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

set(QT_RUNTIME_DLLS)
foreach(m ${QT_RUNTIME_MODULES})
	list(APPEND QT_RUNTIME_DLLS ${QT_DLL_DIR}/Qt${QT_MAJOR}${m}${QTD}.dll)
endforeach()

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
		${QT_RUNTIME_DLLS}
		${MPNC_DIR}/IR_Link_mpNC.dll
		${MPNC_DIR}/onnxruntime.dll
		${MPNC_DIR}/onnxruntime_providers_shared.dll
		$<TARGET_FILE_DIR:${PROJECT_NAME}>
	COMMAND ${CMAKE_COMMAND} -E make_directory
		$<TARGET_FILE_DIR:${PROJECT_NAME}>/platforms
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
		${QT_PLUGIN_DIR}/qwindows${QTD}.dll
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
