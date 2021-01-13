cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_SKIP_RULE_DEPENDENCY TRUE)

enable_language(CXX)

message("Prepare OpenAMF project")

if(AMF_CORE_STATIC)
  if(AMF_CORE_SHARED)
    message(SEND_ERROR "OpenAMF could not be built as a shared and a static library simultaneously!")
    return()
  endif()

  message("OpenAMF will be compiled as a static library")
elseif(AMF_CORE_SHARED)
  message("OpenAMF will be compiled as a shared library and a static loader helper library will be generated")
else()
  message("OpenAMF will be compiled as a static loader helper library only")
endif()

# detects demanded name (previosly set in setup_amf.cmake)
get_property(AMF_NAME GLOBAL PROPERTY AMF_NAME)
get_property(AMF_LOADER_NAME GLOBAL PROPERTY AMF_LOADER_NAME)

if(NOT AMF_NAME)
  if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
    set_property(GLOBAL PROPERTY AMF_NAME amfrt64)
  else()
    set_property(GLOBAL PROPERTY AMF_NAME amfrt32)
  endif()
  get_property(AMF_NAME GLOBAL PROPERTY AMF_NAME)
endif()

#static library to load AMF
if(NOT AMF_CORE_STATIC AND NOT AMF_LOADER_NAME)
  set_property(GLOBAL PROPERTY AMF_LOADER_NAME ${AMF_NAME}_loader)
  get_property(AMF_LOADER_NAME GLOBAL PROPERTY AMF_LOADER_NAME)
endif()

if(NOT AMF_NAME)
  if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
    set_property(GLOBAL PROPERTY AMF_NAME amfrt64)
  else()
    set_property(GLOBAL PROPERTY AMF_NAME amfrt32)
  endif()

  #static library to load AMF
  if(NOT AMF_CORE_STATIC)
    set_property(GLOBAL PROPERTY AMF_LOADER_NAME ${AMF_NAME}_loader)
  endif()

  set_property(GLOBAL PROPERTY AMF_NAME amf)
  #get_property(AMF_NAME GLOBAL PROPERTY AMF_NAME)

  message(WARNING "OpenAMF will be built with the default library name: " ${AMF_NAME})
endif()

if(NOT AMF_CORE_STATIC AND NOT AMF_LOADER_NAME)
  set_property(GLOBAL PROPERTY AMF_LOADER_NAME ${AMF_NAME}_loader)

  message(WARNING "Default loader name for OpenAMF will be used: " ${AMF_LOADER_NAME})
endif()

# project name
project(${AMF_NAME})

include_directories(../../..)

if(APPLE)
  ADD_DEFINITIONS(-DCL_TARGET_OPENCL_VERSION=120)
endif()

if(OpenCL_INCLUDE_DIR AND (OpenCL_LIBRARIES OR AMF_CORE_STATIC))
  message("${AMF_NAME}> OpenCL paths was set externally")

  #test the ${OpenCL_INCLUDE_DIR} is not an absolute path
  if(NOT IS_ABSOLUTE ${OpenCL_INCLUDE_DIR})
    #test amf is included from other project
    if(NOT ${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR})
      get_filename_component(Path_To_OpenCL "${CMAKE_SOURCE_DIR}/${OpenCL_INCLUDE_DIR}" ABSOLUTE)
    #assume that amf is buit from itself
    else()
      get_filename_component(Path_To_OpenCL "${CMAKE_CURRENT_SOURCE_DIR}/${OpenCL_INCLUDE_DIR}" ABSOLUTE)
    endif()
  else()
    get_filename_component(Path_To_OpenCL ${OpenCL_INCLUDE_DIR} ABSOLUTE)
  endif()

  message("${AMF_NAME}> OpenCL_INCLUDE_DIRS: " ${Path_To_OpenCL})
  if(NOT AMF_CORE_STATIC AND OpenCL_LIBRARIES)
    message("${AMF_NAME}> OpenCL_LIBRARIES: " ${OpenCL_LIBRARIES})
  endif()

  include_directories(${Path_To_OpenCL})
else()
  find_package(OpenCL REQUIRED)

  if(NOT OpenCL_FOUND)
    message("")
    message(SEND_ERROR "${AMF_NAME}> Error: Failed to find OpenCL!")
    return()
  else()
    message("${AMF_NAME}> OpenCL found: " ${OpenCL_FOUND})
    message("${AMF_NAME}> OpenCL_INCLUDE_DIRS: " ${OpenCL_INCLUDE_DIRS})
    message("${AMF_NAME}> OpenCL_LIBRARIES: " ${OpenCL_LIBRARIES})
    message("${AMF_NAME}> OpenCL_VERSION_STRING: " ${OpenCL_VERSION_STRING})
    message("${AMF_NAME}> OpenCL_VERSION_MAJOR: " ${OpenCL_VERSION_MAJOR})
    message("${AMF_NAME}> OpenCL_VERSION_MINOR: " ${OpenCL_VERSION_MINOR})

    message("${AMF_NAME}> OpenCL_INCLUDE_DIR: ${OpenCL_INCLUDE_DIR}")
    message("${AMF_NAME}> OpenCL_LIBRARY: ${OpenCL_LIBRARY}")

    include_directories(${OpenCL_INCLUDE_DIR})
  endif()
endif()

# sources
set(
  LOADER_SOURCES

  ../../common/AMFFactory.cpp
  ../../common/AMFSTL.cpp
  ../../common/CurrentTimeImpl.cpp
  ../../common/DataStreamFactory.cpp
  ../../common/DataStreamFile.cpp
  ../../common/DataStreamMemory.cpp
  ../../common/IOCapsImpl.cpp
  ../../common/PropertyStorageExImpl.cpp
  ../../common/Thread.cpp
  ../../common/TraceAdapter.cpp
)

if(WIN32)
  list(APPEND LOADER_SOURCES ../../common/Windows/ThreadWindows.cpp)
else()
  list(APPEND LOADER_SOURCES ../../common/Unix/ThreadUnix.cpp)
endif()

set(
  LIBRARY_SOURCES

  ../../src/core/BufferImpl.cpp
  ../../src/core/ComputeOCL.cpp
  ../../src/core/ContextImpl.cpp
  ../../src/core/DataImpl.cpp
  ../../src/core/DebugImpl.cpp
  ../../src/core/DeviceHostImpl.cpp
  ../../src/core/DeviceImpl.cpp
  ../../src/core/DeviceOCLImpl.cpp
  ../../src/core/FactoryImpl.cpp
  ../../src/core/PlaneImpl.cpp
  ../../src/core/ProgramsImpl.cpp
  ../../src/core/SurfaceImpl.cpp
  ../../src/core/TraceImpl.cpp
  ../../src/core/AMFFileTraceWriter.cpp
  ../../src/core/AMFConsoleTraceWriter.cpp
)

if(APPLE AND ENABLE_METAL)
  list(APPEND LIBRARY_SOURCES ../../src/core/AMFDeviceMetalImpl.cpp)
  list(APPEND LIBRARY_SOURCES ../../src/core/AMFComputeKernelMetal.cpp)
  list(APPEND LIBRARY_SOURCES ../../src/core/AMFComputeFactoryMetal.cpp)
endif()

#headers
set(
  LOADER_HEADERS

  ../../common/AMFMath.h
  ../../common/AMFFactory.h
  ../../common/AMFSTL.h
  ../../common/ByteArray.h
  ../../common/CurrentTime.h
  ../../common/CurrentTimeImpl.h
  ../../common/DataStream.h
  ../../common/DataStreamFile.h
  ../../common/DataStreamMemory.h
  ../../common/IOCapsImpl.h
  ../../common/InterfaceImpl.h
  ../../common/ObservableImpl.h
  ../../common/PropertyStorageExImpl.h
  ../../common/PropertyStorageImpl.h
  ../../common/Thread.h
  ../../common/TraceAdapter.h

  ../../include/components/Ambisonic2SRenderer.h
  ../../include/components/AudioCapture.h
  ../../include/components/ColorSpace.h
  ../../include/components/Component.h
  ../../include/components/ComponentCaps.h
  ../../include/components/DisplayCapture.h
  ../../include/components/FFMPEGAudioConverter.h
  ../../include/components/FFMPEGAudioDecoder.h
  ../../include/components/FFMPEGAudioEncoder.h
  ../../include/components/FFMPEGComponents.h
  ../../include/components/FFMPEGFileDemuxer.h
  ../../include/components/FFMPEGFileMuxer.h
  ../../include/components/FFMPEGVideoDecoder.h
  ../../include/components/MediaSource.h
  ../../include/components/VideoCapture.h
  ../../include/components/VideoConverter.h
  ../../include/components/VideoDecoderUVD.h
  ../../include/components/VideoEncoderHEVC.h
  ../../include/components/VideoEncoderVCE.h
  ../../include/components/VideoStitch.h
  ../../include/components/ZCamLiveStream.h

  ../../include/core/AudioBuffer.h
  ../../include/core/Buffer.h
  ../../include/core/Compute.h
  ../../include/core/ComputeFactory.h
  ../../include/core/Context.h
  ../../include/core/Data.h
  ../../include/core/Debug.h
  ../../include/core/Dump.h
  ../../include/core/Factory.h
  ../../include/core/Interface.h
  ../../include/core/Plane.h
  ../../include/core/Platform.h
  ../../include/core/PropertyStorage.h
  ../../include/core/PropertyStorageEx.h
  ../../include/core/Result.h
  ../../include/core/Surface.h
  ../../include/core/Trace.h
  ../../include/core/Variant.h
  ../../include/core/Version.h
  )

  if(AMF_CORE_STATIC OR AMF_CORE_SHARED)
    set(
      LIBRARY_HEADERS

      ../../src/core/BufferImpl.h
      ../../src/core/ComputeOCL.h
      ../../src/core/ContextImpl.h
      ../../src/core/DataImpl.h
      ../../src/core/DebugImpl.h
      ../../src/core/Device.h
      ../../src/core/DeviceHostImpl.h
      ../../src/core/DeviceImpl.h
      ../../src/core/DeviceOCLImpl.h
      ../../src/core/FactoryImpl.h
      ../../src/core/PlaneImpl.h
      ../../src/core/ProgramsImpl.h
      ../../src/core/SurfaceImpl.h
      ../../src/core/TraceImpl.h
      ../../src/core/AMFFileTraceWriter.h
      ../../src/core/AMFConsoleTraceWriter.h
      )

  if(APPLE AND ENABLE_METAL)
    list(APPEND LIBRARY_HEADERS ../../src/core/AMFDeviceMetalImpl.h)
    list(APPEND LIBRARY_HEADERS ../../src/core/AMFComputeKernelMetal.h)
    list(APPEND LIBRARY_HEADERS ../../src/core/AMFComputeFactoryMetal.h)
  endif()
endif()

#generate projects
if(AMF_CORE_STATIC)
  message("${AMF_NAME} will be built as a static library")

  #declare static library creation
  add_library(
    ${AMF_NAME}
    STATIC

    ${LOADER_SOURCES}
    ${LIBRARY_SOURCES}

    ${LOADER_HEADERS}
    ${LIBRARY_HEADERS}
    )
else()
  #shared library must be linked with OpenCL, so, test it first
  if(NOT OpenCL_LIBRARIES)
    message(SEND_ERROR "Error: OpenCL_LIBRARIES is not set!")
  endif()

  message("${AMF_NAME} will be built as a shared library")
  message("To use ${AMF_NAME} as a shared library a special static helper will be generated - ${AMF_LOADER_NAME}, link applications against it")

  if(AMF_CORE_SHARED)
    #declare shared library creation
    add_library(
      ${AMF_NAME}
      SHARED

      ${LOADER_SOURCES}
      ${LIBRARY_SOURCES}

      ${LOADER_HEADERS}
      ${LIBRARY_HEADERS}
      )

    #link_directories(${OpenCL_LIBRARIES})
    TARGET_LINK_LIBRARIES(${AMF_NAME} ${OpenCL_LIBRARIES})

    set_property(TARGET ${AMF_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)
  endif()

  #declare creation of the static loader helper for amf
  add_library(
    ${AMF_LOADER_NAME}
    STATIC

    ${LOADER_SOURCES}
    ${LOADER_HEADERS}
    )
endif()

if(APPLE AND ENABLE_METAL)
  add_subdirectory(../../src/core/Metal AMF_Metal)
endif()

if (APPLE AND ENABLE_METAL)
  ADD_DEFINITIONS(-DENABLE_METAL=1)
  target_link_libraries(${AMF_NAME} AMF_Metal)
endif()

if(NOT WIN32)
  if(AMF_CORE_SHARED)
    target_compile_options(${AMF_NAME} PUBLIC -mavx2)
    target_compile_options(${AMF_NAME} PUBLIC -mfma)
    target_compile_options(${AMF_NAME} PUBLIC -msse4.2)

    if(CMAKE_BUILD_TYPE MATCHES "Debug" OR CMAKE_BUILD_TYPE MATCHES "RelWithDebInfo")
      target_compile_options(${AMF_NAME} PUBLIC -g)
    endif()

    if (APPLE OR XCODE AND ENABLE_METAL)
      message("Link ${AMF_NAME} with Metal")
      set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} \
        -framework AppKit \
        -framework Metal \
        -framework MetalKit"
      )
    endif()

    if(NOT APPLE AND NOT XCODE)
      target_compile_options(${AMF_NAME} PUBLIC -Wpsabi)
    endif()
  endif()
endif()