cmake_minimum_required(VERSION 3.16)
project(FeatureShowcase CXX)

# NOTE: Remove /RTC1 option from default compiler options for Visual Studio
STRING(REGEX REPLACE "/RTC(su|[1su])" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")

set(PRODUCT_NAME FeatureShowcase)
set(POMDOG_DIR "../..")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED on)
set(CMAKE_CXX_EXTENSIONS off)
set(CMAKE_CONFIGURATION_TYPES Debug Release)

source_group(Source                         REGULAR_EXPRESSION "Source/*")
source_group(Source\\AnimationGraphTest     REGULAR_EXPRESSION "Source/AnimationGraphTest/*")
source_group(Source\\AudioClipTest          REGULAR_EXPRESSION "Source/AudioClipTest/*")
source_group(Source\\BasicEffectTest        REGULAR_EXPRESSION "Source/BasicEffectTest/*")
source_group(Source\\Beam2DTest             REGULAR_EXPRESSION "Source/Beam2DTest/*")
source_group(Source\\BillboardBatchTest     REGULAR_EXPRESSION "Source/BillboardBatchTest/*")
source_group(Source\\DistanceFieldFontTest  REGULAR_EXPRESSION "Source/DistanceFieldFontTest/*")
source_group(Source\\EditorGUITest          REGULAR_EXPRESSION "Source/EditorGUITest/*")
source_group(Source\\GamepadTest            REGULAR_EXPRESSION "Source/GamepadTest/*")
source_group(Source\\GIFDecodeTest          REGULAR_EXPRESSION "Source/GIFDecodeTest/*")
source_group(Source\\GUISplitterTest        REGULAR_EXPRESSION "Source/GUISplitterTest/*")
source_group(Source\\HardwareInstancingTest REGULAR_EXPRESSION "Source/HardwareInstancingTest/*")
source_group(Source\\HTTPClientTest         REGULAR_EXPRESSION "Source/HTTPClientTest/*")
source_group(Source\\ImageEffectsTest       REGULAR_EXPRESSION "Source/ImageEffectsTest/*")
source_group(Source\\LineBatchTest          REGULAR_EXPRESSION "Source/LineBatchTest/*")
source_group(Source\\Particle2DTest         REGULAR_EXPRESSION "Source/Particle2DTest/*")
source_group(Source\\Particle3DTest         REGULAR_EXPRESSION "Source/Particle3DTest/*")
source_group(Source\\ParticleClipLoaderTest REGULAR_EXPRESSION "Source/ParticleClipLoaderTest/*")
source_group(Source\\PolylineDrawingTest    REGULAR_EXPRESSION "Source/PolylineDrawingTest/*")
source_group(Source\\PrimitiveBatchTest     REGULAR_EXPRESSION "Source/PrimitiveBatchTest/*")
source_group(Source\\Skeletal2DTest         REGULAR_EXPRESSION "Source/Skeletal2DTest/*")
source_group(Source\\Skinning2DTest         REGULAR_EXPRESSION "Source/Skinning2DTest/*")
source_group(Source\\SpriteBatchTest        REGULAR_EXPRESSION "Source/SpriteBatchTest/*")
source_group(Source\\SpriteFontTest         REGULAR_EXPRESSION "Source/SpriteFontTest/*")
source_group(Source\\SpriteLineTest         REGULAR_EXPRESSION "Source/SpriteLineTest/*")
source_group(Source\\SVGDecodeTest          REGULAR_EXPRESSION "Source/SVGDecodeTest/*")
source_group(Source\\Texture2DLoaderTest    REGULAR_EXPRESSION "Source/Texture2DLoaderTest/*")
source_group(Source\\VoxelModelTest         REGULAR_EXPRESSION "Source/VoxelModelTest/*")
source_group(Platform                       REGULAR_EXPRESSION "Platform.*/*")

set(RESOURCE_FILES
  # `Content` directory
  Content

  # On Mac
  $<$<PLATFORM_ID:Darwin>:
    Platform.Cocoa/Assets.xcassets
    Platform.Cocoa/Base.lproj/MainMenu.xib
  >
)

add_executable(${PRODUCT_NAME} WIN32
  Source/FeatureShowcaseGame.cpp
  Source/FeatureShowcaseGame.hpp
  Source/AnimationGraphTest/AnimationGraphTest.cpp
  Source/AnimationGraphTest/AnimationGraphTest.hpp
  Source/AudioClipTest/AudioClipTest.cpp
  Source/AudioClipTest/AudioClipTest.hpp
  Source/BasicEffectTest/BasicEffectTest.cpp
  Source/BasicEffectTest/BasicEffectTest.hpp
  Source/Beam2DTest/Beam2DTest.cpp
  Source/Beam2DTest/Beam2DTest.hpp
  Source/BillboardBatchTest/BillboardBatchTest.cpp
  Source/BillboardBatchTest/BillboardBatchTest.hpp
  Source/DistanceFieldFontTest/DistanceFieldFontTest.cpp
  Source/DistanceFieldFontTest/DistanceFieldFontTest.hpp
  Source/EditorGUITest/EditorGUITest.cpp
  Source/EditorGUITest/EditorGUITest.hpp
  Source/GamepadTest/GamepadTest.cpp
  Source/GamepadTest/GamepadTest.hpp
  Source/GIFDecodeTest/GIFDecodeTest.cpp
  Source/GIFDecodeTest/GIFDecodeTest.hpp
  Source/GUISplitterTest/GUISplitterTest.cpp
  Source/GUISplitterTest/GUISplitterTest.hpp
  Source/HardwareInstancingTest/HardwareInstancingTest.cpp
  Source/HardwareInstancingTest/HardwareInstancingTest.hpp
  Source/HTTPClientTest/HTTPClientTest.cpp
  Source/HTTPClientTest/HTTPClientTest.hpp
  Source/ImageEffectsTest/ImageEffectsTest.cpp
  Source/ImageEffectsTest/ImageEffectsTest.hpp
  Source/LineBatchTest/LineBatchTest.cpp
  Source/LineBatchTest/LineBatchTest.hpp
  Source/Particle2DTest/Particle2DTest.cpp
  Source/Particle2DTest/Particle2DTest.hpp
  Source/Particle3DTest/Particle3DTest.cpp
  Source/Particle3DTest/Particle3DTest.hpp
  Source/ParticleClipLoaderTest/ParticleClipLoaderTest.cpp
  Source/ParticleClipLoaderTest/ParticleClipLoaderTest.hpp
  Source/PolylineDrawingTest/PolylineDrawingTest.cpp
  Source/PolylineDrawingTest/PolylineDrawingTest.hpp
  Source/PrimitiveBatchTest/PrimitiveBatchTest.cpp
  Source/PrimitiveBatchTest/PrimitiveBatchTest.hpp
  Source/Skeletal2DTest/Skeletal2DTest.cpp
  Source/Skeletal2DTest/Skeletal2DTest.hpp
  Source/Skinning2DTest/Skinning2DTest.cpp
  Source/Skinning2DTest/Skinning2DTest.hpp
  Source/SpriteBatchTest/SpriteBatchTest.cpp
  Source/SpriteBatchTest/SpriteBatchTest.hpp
  Source/SpriteFontTest/SpriteFontTest.cpp
  Source/SpriteFontTest/SpriteFontTest.hpp
  Source/SpriteLineTest/SpriteLineTest.cpp
  Source/SpriteLineTest/SpriteLineTest.hpp
  Source/SVGDecodeTest/SVGDecodeTest.cpp
  Source/SVGDecodeTest/SVGDecodeTest.hpp
  Source/Texture2DLoaderTest/Texture2DLoaderTest.cpp
  Source/Texture2DLoaderTest/Texture2DLoaderTest.hpp
  Source/VoxelModelTest/VoxelModelTest.cpp
  Source/VoxelModelTest/VoxelModelTest.hpp
  ${RESOURCE_FILES}

  # On Mac
  $<$<PLATFORM_ID:Darwin>:
    Platform.Cocoa/main.mm
    Platform.Cocoa/AppDelegate.h
    Platform.Cocoa/AppDelegate.mm
  >

  # On Linux
  $<$<PLATFORM_ID:Linux>:
    Platform.X11/main.cpp
  >

  # On Windows
  $<$<PLATFORM_ID:Windows>:
    Platform.Win32/main.cpp
    Platform.Win32/Resource.hpp
    Platform.Win32/game.rc
  >
)

target_include_directories(${PRODUCT_NAME} PRIVATE
  ${POMDOG_DIR}/include
)

target_compile_definitions(${PRODUCT_NAME} PRIVATE
  $<$<CONFIG:DEBUG>:DEBUG>
  $<$<CONFIG:RELEASE>:NDEBUG>

  # On Windows
  $<$<PLATFORM_ID:Windows>:
    WIN32_LEAN_AND_MEAN
    NOMINMAX
  >
)

target_compile_options(${PRODUCT_NAME} PRIVATE
  # Clang
  $<$<OR:$<CXX_COMPILER_ID:Clang>,$<STREQUAL:${CMAKE_GENERATOR},Xcode>>:
    -Wcomma
    -Wextra-semi
    -Wimplicit-fallthrough
    -Wmost
    -Wmove
    -Wnewline-eof
  >
  # GCC
  $<$<CXX_COMPILER_ID:GNU>:
    -pedantic
    -Wimplicit-fallthrough=3
  >
  # Clang and GCC
  $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>,$<STREQUAL:${CMAKE_GENERATOR},Xcode>>:
    -Werror
    $<$<CONFIG:DEBUG>:-g;-O0>
    $<$<CONFIG:RELEASE>:-O3>
  >
  # MSVC
  $<$<CXX_COMPILER_ID:MSVC>:
    /W4
    $<$<CONFIG:DEBUG>:/Od;/MTd>
    $<$<CONFIG:RELEASE>:/O2;/Ob2;/MT>
  >
)

if(WIN32)
  # Replace with target_link_options() when upgrading CMake 3.12
  target_link_libraries(${PRODUCT_NAME}
    debug -INCREMENTAL
    debug -DEBUG
    optimized -INCREMENTAL:NO
    optimized -OPT:REF
  )
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  # Copying 'Content' directory into shippable directory after building executable
  add_custom_command(
    COMMENT "Copying contents to ${CMAKE_BINARY_DIR}/Content"
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Content
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
      ${CMAKE_CURRENT_SOURCE_DIR}/Content
      ${CMAKE_CURRENT_BINARY_DIR}/Content
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Content
  )
endif()

if(APPLE)
  set_source_files_properties(
    ${RESOURCE_FILES}
    PROPERTIES
    MACOSX_PACKAGE_LOCATION Resources
  )

  set_target_properties(
    ${PRODUCT_NAME} PROPERTIES
    MACOSX_BUNDLE TRUE
    RESOURCE "${RESOURCE_FILES}"
  )

  target_link_libraries(${PRODUCT_NAME}
    "-framework AudioToolBox"
    "-framework Cocoa"
    "-framework IOKit"
    "-framework Metal"
    "-framework MetalKit"
    "-framework OpenAL"
    "-framework OpenGL"
    "-framework QuartzCore"
  )
endif()

set_target_properties(${PRODUCT_NAME} PROPERTIES
  # Change working directory in Visual Studio
  VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"

  # Xcode
  XCODE_ATTRIBUTE_GCC_OPTIMIZATION_LEVEL[variant=Debug] "0" # -O0
  XCODE_ATTRIBUTE_GCC_OPTIMIZATION_LEVEL[variant=Release] "3" # -O3
  XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH[variant=Debug] "YES"
  XCODE_ATTRIBUTE_COMBINE_HIDPI_IMAGES "YES"

  XCODE_ATTRIBUTE_GCC_VERSION "com.apple.compilers.llvm.clang.1_0"
  XCODE_ATTRIBUTE_GCC_C_LANGUAGE_STANDARD "c11"
  XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++17"
  XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++"
  XCODE_ATTRIBUTE_MACOSX_DEPLOYMENT_TARGET "10.12"

  MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_LIST_DIR}/Platform.Cocoa/Info.plist
  XCODE_ATTRIBUTE_LD_RUNPATH_SEARCH_PATHS "$(inherited) @loader_path @executable_path/../Frameworks"

  # Warnings (Clang)
  XCODE_ATTRIBUTE_CLANG_WARN_BOOL_CONVERSION "YES"
  XCODE_ATTRIBUTE_CLANG_WARN_CONSTANT_CONVERSION "YES"
  XCODE_ATTRIBUTE_CLANG_WARN_EMPTY_BODY "YES"
  XCODE_ATTRIBUTE_CLANG_WARN_ENUM_CONVERSION "YES"
  XCODE_ATTRIBUTE_CLANG_WARN_INT_CONVERSION "YES"
  XCODE_ATTRIBUTE_CLANG_WARN_UNGUARDED_AVAILABILITY "YES_AGGRESSIVE"
  XCODE_ATTRIBUTE_CLANG_WARN_UNREACHABLE_CODE "YES"

  # Warnings (GCC and Clang)
  XCODE_ATTRIBUTE_GCC_WARN_64_TO_32_BIT_CONVERSION "YES"
  XCODE_ATTRIBUTE_GCC_WARN_ABOUT_DEPRECATED_FUNCTIONS "YES"
  XCODE_ATTRIBUTE_GCC_WARN_ABOUT_MISSING_FIELD_INITIALIZERS "YES"
  XCODE_ATTRIBUTE_GCC_WARN_ABOUT_MISSING_NEWLINE "YES"
  XCODE_ATTRIBUTE_GCC_WARN_ABOUT_RETURN_TYPE "YES_ERROR"
  XCODE_ATTRIBUTE_GCC_WARN_CHECK_SWITCH_STATEMENTS "YES"
  XCODE_ATTRIBUTE_GCC_WARN_HIDDEN_VIRTUAL_FUNCTIONS "YES"
  XCODE_ATTRIBUTE_GCC_WARN_MISSING_PARENTHESES "YES"
  XCODE_ATTRIBUTE_GCC_WARN_NON_VIRTUAL_DESTRUCTOR "YES"
  XCODE_ATTRIBUTE_GCC_WARN_SHADOW "YES"
  XCODE_ATTRIBUTE_GCC_WARN_SIGN_COMPARE "YES"
  XCODE_ATTRIBUTE_GCC_WARN_TYPECHECK_CALLS_TO_PRINTF "YES"
  XCODE_ATTRIBUTE_GCC_WARN_UNINITIALIZED_AUTOS "YES_AGGRESSIVE"
  XCODE_ATTRIBUTE_GCC_WARN_UNKNOWN_PRAGMAS "YES"
  XCODE_ATTRIBUTE_GCC_WARN_UNUSED_FUNCTION "YES"
  XCODE_ATTRIBUTE_GCC_WARN_UNUSED_LABEL "YES"
  XCODE_ATTRIBUTE_GCC_WARN_UNUSED_VALUE "YES"
  XCODE_ATTRIBUTE_GCC_WARN_UNUSED_VARIABLE "YES"

  # Warnings - Objective-C
  XCODE_ATTRIBUTE_CLANG_WARN_DIRECT_OBJC_ISA_USAGE "YES_ERROR"
  XCODE_ATTRIBUTE_CLANG_WARN__DUPLICATE_METHOD_MATCH "YES"
  XCODE_ATTRIBUTE_GCC_WARN_ALLOW_INCOMPLETE_PROTOCOL "YES"
  XCODE_ATTRIBUTE_GCC_WARN_UNDECLARED_SELECTOR "YES"
  XCODE_ATTRIBUTE_CLANG_WARN_DEPRECATED_OBJC_IMPLEMENTATIONS "YES"
  XCODE_ATTRIBUTE_CLANG_WARN_OBJC_IMPLICIT_RETAIN_SELF "YES"
  XCODE_ATTRIBUTE_CLANG_WARN_OBJC_LITERAL_CONVERSION "YES"
  XCODE_ATTRIBUTE_CLANG_WARN_OBJC_ROOT_CLASS "YES_ERROR"

  # Warning Policies
  XCODE_ATTRIBUTE_GCC_TREAT_WARNINGS_AS_ERRORS "YES"

  # Metal Compiler
  XCODE_ATTRIBUTE_MTL_ENABLE_DEBUG_INFO[variant=Debug] "INCLUDE_SOURCE"
  XCODE_ATTRIBUTE_MTL_ENABLE_DEBUG_INFO[variant=Release] "NO"
  XCODE_ATTRIBUTE_MTL_FAST_MATH "YES"

  # Static Analyzer
  XCODE_ATTRIBUTE_CLANG_ANALYZER_LOCALIZABILITY_NONLOCALIZED "YES"

  # Symbols
  XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC "YES"
  XCODE_ATTRIBUTE_GCC_INLINES_ARE_PRIVATE_EXTERN "YES" # -fvisibility-inlines-hidden
  XCODE_ATTRIBUTE_GCC_SYMBOLS_PRIVATE_EXTERN "YES" # -fvisibility=hidden
)

if(NOT POMDOG_BUILD_TARGET_ALL)
  add_subdirectory(${POMDOG_DIR}/build/pomdog ${CMAKE_CURRENT_BINARY_DIR}/pomdog_build)
endif()
target_link_libraries(${PRODUCT_NAME} pomdog_static)
