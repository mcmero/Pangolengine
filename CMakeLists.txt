cmake_minimum_required(VERSION 3.5)

# Set the output directory for built objects
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

# Prevent installing to system directories
set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}" CACHE INTERNAL "")

# Declare the project
project(pangolengine)

# Set C++ version
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# On Web targets, we need CMake to generate a HTML webpage
if(CMAKE_SYSTEM_NAME MATCHES Emscripten)
    set(CMAKE_EXECUTABLE_SUFFIX ".html" CACHE INTERNAL "")
endif()

#==============================================================================
# SDL3 Dependencies
#==============================================================================

# Configure SDL by calling its CMake file
# We use EXCLUDE_FROM_ALL so that its install targets and configs don't
# pollute upwards into our configuration
add_subdirectory(SDL EXCLUDE_FROM_ALL)

# Add SDL_ttf
set(SDLTTF_VENDORED ON) # Tell SDL_ttf to build its own dependencies
add_subdirectory(SDL_ttf EXCLUDE_FROM_ALL)

# SDL_mixer (used for playing audio)
set(SDLMIXER_MIDI_NATIVE OFF)     # Disable formats we don't use to make the build faster and smaller
set(SDLMIXER_GME OFF)
set(SDLMIXER_WAVPACK OFF)
set(SDLMIXER_MOD OFF)
set(SDLMIXER_OPUS OFF)
set(SDLMIXER_VENDORED ON)   # Tell SDL_mixer to build its own dependencies
add_subdirectory(SDL_mixer EXCLUDE_FROM_ALL)

# Add SDL_image submodule and link it
set(SDLIMAGE_VENDORED ON)
set(SDLIMAGE_AVIF OFF)    # Disable formats we don't use to make the build faster and smaller
set(SDLIMAGE_BMP OFF)
set(SDLIMAGE_JPEG OFF)
set(SDLIMAGE_WEBP OFF)
add_subdirectory(SDL_image EXCLUDE_FROM_ALL)

#==============================================================================
# Pangolengine Library
#==============================================================================

add_library(pangolengine_lib STATIC
  src/Engine.cpp
  src/Camera.cpp
  src/TextureManager.cpp
  src/MapLoader.cpp
  src/Vector2D.cpp
  src/Collision.cpp
  src/Parsers/Tokeniser.cpp
  src/Parsers/JsonParser.cpp
  src/Parsers/TsxParser.cpp
  # Header-only files (for IDE support)
  src/Engine.h
  src/IGame.h
  src/Constants.h
  src/Camera.h
  src/TextureManager.h
  src/MapLoader.h
  src/Vector2D.h
  src/Collision.h
  src/Components/Components.h
  src/Components/ECS.h
  src/UI/UIManager.h
  src/UI/Grid.h
  src/UI/IUIComponent.h
  src/UI/IUIManager.h
  src/UI/UIComponents.h
)

# Make engine headers available to users of the library
target_include_directories(pangolengine_lib PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link SDL to the engine library
target_link_libraries(pangolengine_lib PUBLIC
  SDL3::SDL3
  SDL3_ttf::SDL3_ttf
  SDL3_mixer::SDL3_mixer
  SDL3_image::SDL3_image
)

# Set C++ standard and compile definitions
target_compile_features(pangolengine_lib PUBLIC cxx_std_20)
target_compile_definitions(pangolengine_lib PUBLIC SDL_MAIN_USE_CALLBACKS)

#==============================================================================
# Demo Executable (Optional)
#==============================================================================

option(BUILD_DEMO "Build the demo game" ON)

if(BUILD_DEMO)
  # Determine executable name based on platform
  if(ANDROID)
    # The SDL java code is hardcoded to load libmain.so on android
    set(DEMO_EXECUTABLE_NAME main)
    add_library(${DEMO_EXECUTABLE_NAME} SHARED)
  else()
    set(DEMO_EXECUTABLE_NAME pangolengine_demo)
    add_executable(${DEMO_EXECUTABLE_NAME})
  endif()

  # Add demo sources
  target_sources(${DEMO_EXECUTABLE_NAME} PRIVATE
    examples/demo/main.cpp
    examples/demo/DemoGame.cpp
    examples/demo/DemoGame.h
    src/iosLaunchScreen.storyboard
  )

  # To get an app icon on Apple platforms, add it to your executable
  if(APPLE)
    target_sources(${DEMO_EXECUTABLE_NAME} PRIVATE "src/logo.png")
  endif()

  # Link the demo against the engine library
  target_link_libraries(${DEMO_EXECUTABLE_NAME} PRIVATE pangolengine_lib)

  # Set C++ standard
  target_compile_features(${DEMO_EXECUTABLE_NAME} PUBLIC cxx_std_20)

  # Copy demo assets to the build directory
  set(DEMO_ASSETS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/examples/demo/assets")
  add_custom_command(
    TARGET ${DEMO_EXECUTABLE_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${DEMO_ASSETS_DIR} $<TARGET_FILE_DIR:${DEMO_EXECUTABLE_NAME}>/assets
    COMMENT "Copying demo assets..."
  )

  # Set platform-specific properties
  set_target_properties(${DEMO_EXECUTABLE_NAME} PROPERTIES
    # On macOS, make a proper .app bundle instead of a bare executable
    MACOSX_BUNDLE TRUE
    
    # Set the Info.plist file for Apple Mobile platforms. Without this file, your app will not launch
    # What is iosLaunchScreen. storyboard? This file describes what Apple's mobile platforms
    # should show the user while the application is starting up. If you don't include one,
    # then you get placed in a compatibility mode that does not allow HighDPI.
    # This file is referenced inside Info.plist.in, where it is marked as the launch screen file.
    # It is also ignored on non-Apple platforms.
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/src/Info.plist.in"

    # In Xcode, create a Scheme in the schemes dropdown for the app
    XCODE_GENERATE_SCHEME TRUE
    
    # Identification for Xcode
    XCODE_ATTRIBUTE_BUNDLE_IDENTIFIER "com.mcmero.pangolengine.demo"
    XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "com.mcmero.pangolengine.demo"
    XCODE_ATTRIBUTE_CURRENTYEAR "${CURRENTYEAR}"
  )

  # On Visual Studio, set the demo as the default project
  set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT "${DEMO_EXECUTABLE_NAME}")

  # On macOS Platforms, ensure that the bundle is valid for distribution by calling fixup_bundle
  # Note that fixup_bundle does not work on iOS, so you will want to use static libraries
  # or manually copy dylibs and set rpaths
  if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
    INSTALL(CODE 
      "include(BundleUtilities)
      fixup_bundle(\"$<TARGET_BUNDLE_DIR:${DEMO_EXECUTABLE_NAME}>\" \"\" \"\")
      " 
      COMPONENT Runtime
    )
  endif()
  
  install(TARGETS ${DEMO_EXECUTABLE_NAME} BUNDLE DESTINATION ./install)
endif()
