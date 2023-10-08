cmake_minimum_required(VERSION 3.5)

project(wxm_m5_flasher VERSION 0.1.2 LANGUAGES CXX)

add_definitions(-DPROJECT_VERSION="${PROJECT_VERSION}")

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
find_package(Qt6 REQUIRED COMPONENTS SerialPort)

set(PROJECT_SOURCES
	main.cpp
	mainwindow.cpp
	mainwindow.h
	mainwindow.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
	qt_add_executable(Esp32Flasher
		MANUAL_FINALIZATION
		${PROJECT_SOURCES}
		const.h esptool.cpp esptool.h esptool_state.h struct.h
		resources.qrc
		firmware.h firmware.cpp
		log.h log.cpp

	)
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET Esp32Flasher APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
	if(ANDROID)
		add_library(Esp32Flasher SHARED
			${PROJECT_SOURCES}
		)
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
	else()
		add_executable(Esp32Flasher
			${PROJECT_SOURCES}
		)
	endif()
endif()

target_link_libraries(Esp32Flasher PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(Esp32Flasher PRIVATE Qt6::SerialPort)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.Esp32Flasher)
endif()
set_target_properties(Esp32Flasher PROPERTIES
	${BUNDLE_ID_OPTION}
	MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
	MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
	MACOSX_BUNDLE TRUE
	WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS Esp32Flasher
	BUNDLE DESTINATION .
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Download esptool from github
set(esptool_dest "${CMAKE_BINARY_DIR}/bin")
if(UNIX)
	message("Downloading esptool for Linux")
	set(esptool_url "https://github.com/espressif/esptool/releases/download/v4.6.2/esptool-v4.6.2-linux-amd64.zip")
	get_filename_component(esptool_filename ${esptool_url} NAME)
	set(esptool_downloaded ${esptool_dest}/${esptool_filename})
	file(DOWNLOAD ${esptool_url} ${esptool_downloaded} SHOW_PROGRESS)
	execute_process(COMMAND unzip -joq ${esptool_downloaded} -d ${esptool_dest})
	execute_process(COMMAND chmod +x ${esptool_dest}/esptool)

	# Copy esptool to app root
#    file(COPY ${esptool_dest}/esptool DESTINATION ./)
elseif(WIN32)
	message("Downloading esptool for Windows")

	set(esptool_url "https://github.com/espressif/esptool/releases/download/v4.6.2/esptool-v4.6.2-win64.zip")
	get_filename_component(esptool_filename ${esptool_url} NAME_WLE)
	set(esptool_downloaded ${esptool_dest}/${esptool_filename}.zip)
	file(DOWNLOAD ${esptool_url} ${esptool_downloaded} SHOW_PROGRESS)
	execute_process(COMMAND powershell Expand-Archive -Path ${esptool_downloaded} -DestinationPath ${esptool_dest} -Force)

	# Copy esptool to app root
	file(COPY ${esptool_dest}/${esptool_filename}/esptool.exe DESTINATION ./)
endif()

# Copy all binaries from fw to same dir as esptool
# To be downloaded and extracted automatically when fw repo becomes public
file(GLOB FW_BINARIES "fw/*")
file(COPY ${FW_BINARIES} DESTINATION ${esptool_dest})

# Copy fw dir to build dir
#file(COPY fw DESTINATION ${esptool_dest})

if(QT_VERSION_MAJOR EQUAL 6)
	qt_finalize_executable(Esp32Flasher)
endif()
