cmake_minimum_required(VERSION 3.1)
project(libigl_tutorials)
message(STATUS "CMAKE_C_COMPILER: ${CMAKE_C_COMPILER}")
message(STATUS "CMAKE_CXX_COMPILER: ${CMAKE_CXX_COMPILER}")

### conditionally compile certain modules depending on libraries found on the system
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/../shared/cmake)
find_package(CGAL QUIET COMPONENTS Core)
find_package(MATLAB QUIET)
find_package(MOSEK)

### libIGL options: choose between header only and compiled static library
option(LIBIGL_USE_STATIC_LIBRARY "Use LibIGL as static library" ON)
option(LIBIGL_WITH_VIEWER      "Use OpenGL viewer"  ON)
option(LIBIGL_WITH_NANOGUI     "Use Nanogui menu"   OFF)
option(LIBIGL_WITH_EMBREE      "Use Embree"         ON)

### libIGL options: choose your dependencies (by default everything is OFF, in this example we need the viewer)
option(LIBIGL_WITH_CGAL             "Use CGAL"           "${CGAL_FOUND}")
option(LIBIGL_WITH_CORK             "Use CORK"           OFF)
option(LIBIGL_WITH_MATLAB           "Use Matlab"         "${MATLAB_FOUND}")
option(LIBIGL_WITH_MOSEK            "Use MOSEK"          "${MOSEK_FOUND}")
### End

### Adding libIGL: choose the path to your local copy libIGL
include(libigl)

### Output directories
if(MSVC)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR})
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR})
else()
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "../")
endif()

### Choose which chapters to compile
option(TUTORIALS_CHAPTER1 "Compile chapter 1" ON)
option(TUTORIALS_CHAPTER2 "Compile chapter 2" ON)
option(TUTORIALS_CHAPTER3 "Compile chapter 3" ON)
option(TUTORIALS_CHAPTER4 "Compile chapter 4" ON)
option(TUTORIALS_CHAPTER5 "Compile chapter 5" ON)
option(TUTORIALS_CHAPTER6 "Compile chapter 6" ON)
option(TUTORIALS_CHAPTER7 "Compile chapter 7" ON)

# Store location of tutorial/shared directory
set(TUTORIAL_SHARED_PATH ${CMAKE_CURRENT_SOURCE_DIR}/shared CACHE PATH "location of shared tutorial resources")
add_library(tutorials INTERFACE)
target_compile_definitions(tutorials INTERFACE "-DTUTORIAL_SHARED_PATH=\"${TUTORIAL_SHARED_PATH}\"")
target_include_directories(tutorials INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

# Chapter 1
if(TUTORIALS_CHAPTER1)
  add_subdirectory("101_FileIO")
  add_subdirectory("102_DrawMesh")
  add_subdirectory("103_Events")
  add_subdirectory("104_Colors")
  add_subdirectory("105_Overlays")
  add_subdirectory("106_ViewerMenu")
  add_subdirectory("107_OculusViewer")
endif()

# Chapter 2
if(TUTORIALS_CHAPTER2)
  add_subdirectory("201_Normals")
  add_subdirectory("202_GaussianCurvature")
  add_subdirectory("203_CurvatureDirections")
  add_subdirectory("204_Gradient")
  add_subdirectory("205_Laplacian")
endif()

# Chapter 3
if(TUTORIALS_CHAPTER3)
  add_subdirectory("301_Slice")
  add_subdirectory("302_Sort")
  add_subdirectory("303_LaplaceEquation")
  add_subdirectory("304_LinearEqualityConstraints")
  add_subdirectory("305_QuadraticProgramming")
  add_subdirectory("306_EigenDecomposition")
endif()

# Chapter 4
if(TUTORIALS_CHAPTER4)
  add_subdirectory("401_BiharmonicDeformation")
  add_subdirectory("402_PolyharmonicDeformation")
  add_subdirectory("403_BoundedBiharmonicWeights")
  add_subdirectory("404_DualQuaternionSkinning")
  add_subdirectory("405_AsRigidAsPossible")
  add_subdirectory("406_FastAutomaticSkinningTransformations")
  add_subdirectory("407_BiharmonicCoordinates")
endif()

# Chapter 5
if(TUTORIALS_CHAPTER5)
  add_subdirectory("501_HarmonicParam")
  add_subdirectory("502_LSCMParam")
  add_subdirectory("503_ARAPParam")
  if(LIBIGL_WITH_COMISO)
    add_subdirectory("504_NRosyDesign")
    add_subdirectory("505_MIQ")
    add_subdirectory("506_FrameField")
  endif()
  add_subdirectory("507_PolyVectorField")
  add_subdirectory("508_ConjugateField")
  add_subdirectory("509_Planarization")
  add_subdirectory("510_Integrable")
  add_subdirectory("511_PolyVectorFieldGeneral")
endif()

# Chapter 6
if(TUTORIALS_CHAPTER6)
  if(LIBIGL_WITH_XML)
    add_subdirectory("601_Serialization")
  endif()
  if(LIBIGL_WITH_MATLAB)
    add_subdirectory("602_Matlab")
  endif()
  if(LIBIGL_WITH_TRIANGLE)
    add_subdirectory("604_Triangle")
  endif()
  if(LIBIGL_WITH_TETGEN)
    add_subdirectory("605_Tetgen")
  endif()
  if(LIBIGL_WITH_EMBREE)
    add_subdirectory("606_AmbientOcclusion")
  endif()
  if(LIBIGL_WITH_PNG)
    add_subdirectory("607_ScreenCapture")
  endif()
  if(LIBIGL_WITH_LIM)
    add_subdirectory("608_LIM")
  endif()
  if(LIBIGL_WITH_CGAL)
    add_subdirectory("609_Boolean")
    add_subdirectory("610_CSGTree")
  endif()
endif()

# Chapter 7
if(TUTORIALS_CHAPTER7)
  add_subdirectory("701_Statistics")
  add_subdirectory("702_WindingNumber")
  add_subdirectory("703_Decimation")
  add_subdirectory("704_SignedDistance")
  add_subdirectory("705_MarchingCubes")
  if(LIBIGL_WITH_EMBREE)
    add_subdirectory("706_FacetOrientation")
  endif()
  add_subdirectory("707_SweptVolume")
  add_subdirectory("708_Picking")
  if(LIBIGL_WITH_COMISO)
    add_subdirectory("709_VectorFieldVisualizer")
  endif()
  add_subdirectory("710_SLIM")
  add_subdirectory("711_Subdivision")
  add_subdirectory("712_DataSmoothing")
endif()

