# The name of our project is "OF_LIB". CMakeLists files in this project can 
# refer to the root source directory of the project as ${OF_LIB_SOURCE_DIR} and 
# to the root binary directory of the project as ${OF_LIB_BINARY_DIR}. 
cmake_minimum_required (VERSION 2.4) 

if(WIN32)

	project (OF_LIB)

	if(COMMAND cmake_policy)
	  cmake_policy(SET CMP0003 NEW)
	endif(COMMAND cmake_policy)

	ADD_DEFINITIONS(-DPOCO_STATIC)

	include_directories (${OF_ABS_PATH}/openFrameworks)
	include_directories (${OF_ABS_PATH}/openFrameworks/graphics)
	include_directories (${OF_ABS_PATH}/openFrameworks/app)
	include_directories (${OF_ABS_PATH}/openFrameworks/sound)
	include_directories (${OF_ABS_PATH}/openFrameworks/utils)
	include_directories (${OF_ABS_PATH}/openFrameworks/communication)
	include_directories (${OF_ABS_PATH}/openFrameworks/video)
	include_directories (${OF_ABS_PATH}/openFrameworks/events)
	include_directories (${OF_ABS_PATH}/glut/include)
	include_directories (${OF_ABS_PATH}/rtAudio/include)
	include_directories (${OF_ABS_PATH}/quicktime/include)
	include_directories (${OF_ABS_PATH}/freetype/include)
	include_directories (${OF_ABS_PATH}/freetype/include/freetype2)
	include_directories (${OF_ABS_PATH}/freeImage/include)
	include_directories (${OF_ABS_PATH}/fmodex/include)
	include_directories (${OF_ABS_PATH}/videoInput/include)
	include_directories (${OF_ABS_PATH}/glee/include)
	include_directories (${OF_ABS_PATH}/glu/include)
	include_directories (${OF_ABS_PATH}/poco/include)

	add_library(OFLIB
		${OF_ABS_PATH}/openFrameworks/ofMain.h
		${OF_ABS_PATH}/openFrameworks/app/ofAppGlutWindow.cpp
		${OF_ABS_PATH}/openFrameworks/app/ofAppGlutWindow.h
		${OF_ABS_PATH}/openFrameworks/app/ofAppRunner.cpp
		${OF_ABS_PATH}/openFrameworks/app/ofAppRunner.h
		${OF_ABS_PATH}/openFrameworks/graphics/ofBitmapFont.cpp
		${OF_ABS_PATH}/openFrameworks/graphics/ofBitmapFont.h
		${OF_ABS_PATH}/openFrameworks/graphics/ofGraphics.cpp
		${OF_ABS_PATH}/openFrameworks/graphics/ofGraphics.h
		${OF_ABS_PATH}/openFrameworks/graphics/ofImage.cpp
		${OF_ABS_PATH}/openFrameworks/graphics/ofImage.h
		${OF_ABS_PATH}/openFrameworks/graphics/ofTexture.cpp
		${OF_ABS_PATH}/openFrameworks/graphics/ofTexture.h
		${OF_ABS_PATH}/openFrameworks/graphics/ofTrueTypefont.h
		${OF_ABS_PATH}/openFrameworks/sound/ofSoundPlayer.cpp
		${OF_ABS_PATH}/openFrameworks/sound/ofSoundPlayer.h
		${OF_ABS_PATH}/openFrameworks/sound/ofSoundStream.cpp
		${OF_ABS_PATH}/openFrameworks/sound/ofSoundStream.h
		${OF_ABS_PATH}/openFrameworks/utils/ofConstants.h
		${OF_ABS_PATH}/openFrameworks/utils/ofMath.cpp
		${OF_ABS_PATH}/openFrameworks/utils/ofMath.h
		${OF_ABS_PATH}/openFrameworks/utils/ofTypes.h
		${OF_ABS_PATH}/openFrameworks/utils/ofUtils.cpp
		${OF_ABS_PATH}/openFrameworks/utils/ofUtils.h
		${OF_ABS_PATH}/openFrameworks/video/ofQtUtils.cpp
		${OF_ABS_PATH}/openFrameworks/video/ofQtUtils.h
		${OF_ABS_PATH}/openFrameworks/video/ofVideoGrabber.cpp
		${OF_ABS_PATH}/openFrameworks/video/ofVideoGrabber.h
		${OF_ABS_PATH}/openFrameworks/video/ofVideoPlayer.cpp
		${OF_ABS_PATH}/openFrameworks/video/ofVideoPlayer.h
		${OF_ABS_PATH}/openFrameworks/communication/ofArduino.cpp
		${OF_ABS_PATH}/openFrameworks/communication/ofArduino.h
		${OF_ABS_PATH}/openFrameworks/communication/ofSerial.cpp
		${OF_ABS_PATH}/openFrameworks/communication/ofSerial.h
		${OF_ABS_PATH}/openFrameworks/events/ofEvents.h
		${OF_ABS_PATH}/openFrameworks/events/ofEventUtils.h
	)

	target_link_libraries(OFLIB OpenGL32.lib GLu32.lib kernel32.lib setupapi.lib glut32.lib videoInput.lib libfreetype.lib FreeImage.lib qtmlClient.lib dsound.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib GLee.lib fmodex_vc.lib glu32.lib)

	#target_link_libraries(OFLIB optimized openframeworksLib.lib)
	target_link_libraries(OFLIB optimized rtAudio.lib)
	target_link_libraries(OFLIB optimized PocoFoundationmt.lib)
	target_link_libraries(OFLIB optimized PocoNetmt.lib)
	target_link_libraries(OFLIB optimized PocoUtilmt.lib)
	target_link_libraries(OFLIB optimized PocoXMLmt.lib)

	#target_link_libraries(OFLIB debug openframeworksLibDebug.lib)
	target_link_libraries(OFLIB debug rtAudioD.lib)
	target_link_libraries(OFLIB debug PocoFoundationmtd.lib)
	target_link_libraries(OFLIB debug PocoNetmtd.lib)
	target_link_libraries(OFLIB debug PocoUtilmtd.lib)
	target_link_libraries(OFLIB debug PocoXMLmtd.lib)
endif(WIN32)



# The name of our project is "OFSAMPLE". CMakeLists files in this project can 
# refer to the root source directory of the project as ${OFSAMPLE_SOURCE_DIR} and 
# to the root binary directory of the project as ${OFSAMPLE_BINARY_DIR}. 
cmake_minimum_required (VERSION 2.4) 

if(WIN32)

	project (OFSAMPLE)

	if(COMMAND cmake_policy)
	  cmake_policy(SET CMP0003 NEW)
	endif(COMMAND cmake_policy)

	ADD_DEFINITIONS(-DPOCO_STATIC)
	SET(CMAKE_EXE_LINKER_FLAGS /NODEFAULTLIB:/"atlthunk.lib, LIBC.lib, LIBCMT/")

	include_directories (${OF_ABS_PATH}/openFrameworks)
	include_directories (${OF_ABS_PATH}/openFrameworks/graphics)
	include_directories (${OF_ABS_PATH}/openFrameworks/app)
	include_directories (${OF_ABS_PATH}/openFrameworks/sound)
	include_directories (${OF_ABS_PATH}/openFrameworks/utils)
	include_directories (${OF_ABS_PATH}/openFrameworks/communication)
	include_directories (${OF_ABS_PATH}/openFrameworks/video)
	include_directories (${OF_ABS_PATH}/openFrameworks/events)
	include_directories (${OF_ABS_PATH}/glut/include)
	include_directories (${OF_ABS_PATH}/rtAudio/include)
	include_directories (${OF_ABS_PATH}/quicktime/include)
	include_directories (${OF_ABS_PATH}/freetype/include)
	include_directories (${OF_ABS_PATH}/freetype/include/freetype2)
	include_directories (${OF_ABS_PATH}/freeImage/include)
	include_directories (${OF_ABS_PATH}/fmodex/include)
	include_directories (${OF_ABS_PATH}/videoInput/include)
	include_directories (${OF_ABS_PATH}/glee/include)
	include_directories (${OF_ABS_PATH}/glu/include)
	include_directories (${OF_ABS_PATH}/poco/include)
	include_directories (${OF_ABS_PATH}/addons)

	link_directories (${OF_ABS_PATH}/glut/lib/vs2008)
	link_directories (${OF_ABS_PATH}/rtAudio/lib/vs2008)
	link_directories (${OF_ABS_PATH}/FreeImage/lib/vs2008)
	link_directories (${OF_ABS_PATH}/freetype/lib/vs2008)
	link_directories (${OF_ABS_PATH}/quicktime/lib/vs2008)
	link_directories (${OF_ABS_PATH}/fmodex/lib/vs2008)
	link_directories (${OF_ABS_PATH}/videoInput/lib/vs2008)
	link_directories (${OF_ABS_PATH}/glee/lib/vs2008)
	link_directories (${OF_ABS_PATH}/glu/lib/vs2008)
	link_directories (${OF_ABS_PATH}/poco/lib/vs2005)
	#link_directories (${OF_ABS_PATH}/openFrameworksCompiled/lib/vs2008)

	include_directories (${OFSAMPLE_SOURCE_DIR}/../include)
	link_directories (${OFSAMPLE_SOURCE_DIR}/../)

	add_executable(OFSAMPLE main.cpp testApp.cpp testApp.h GuiOf.h GuiOf.cpp)

	target_link_libraries(OFSAMPLE OFLIB GUI TOWER)

endif(WIN32)

