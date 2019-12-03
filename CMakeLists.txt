cmake_minimum_required(VERSION 3.3)

project(ApexMax VERSION 1.6.1)

set(TARGETEX_LOCATION 3rd_party/ApexLib/3rd_party/PreCore/cmake)
include(${TARGETEX_LOCATION}/3dsmax.cmake)

set (ApexLibLibraryPath ../ApexLib_${CMAKE_GENERATOR_PLATFORM}_${CHAR_TYPE})

add_subdirectory(3rd_party/ApexLib ${ApexLibLibraryPath})

build_target(
	TYPE SHARED
	SOURCES
		src/ApexImp.cpp
		src/ApexMax.cpp
		src/ApexMat.cpp
		src/DllEntry.cpp
		src/ApexMax.def
		src/ApexImp.rc
		${MAX_EX_DIR}/win/About.rc
	LINKS
		gdiplus bmm core Morpher ApexLib flt mesh maxutil maxscrpt paramblk2 geom
	DEFINITIONS
		${MaxDefinitions}
	INCLUDES
		${MaxSDK}/include
		3rd_party/Xplorer/include
		3rd_party/ApexLib/include
		3rd_party/ApexLib/3rd_party/PreCore
	LINK_DIRS
		${MaxSDKLibrariesPath}
	AUTHOR "Lukas Cone"
	DESCR "Apex Engine 3DS Max Plugin"
	NAME "ApexMax"
	START_YEAR 2014
	PROPERTIES
		SUFFIX .dlu
		${MaxProperties}
)

build_morpher()