Few months ago, I found an [article][3] that explains how to use [GoogleTest and GoogleMock][4] --- as an external dependency --- in a [CMake][5] project. Since the approach is amazingly straightforward, I have managed to mimic the paradigm and use *SFML* --- as an external dependency --- in a C++ graphical app.

## The Idea : build the dependency targets at *configuration time*

The main idea is to compile the external project at *configure time*. This fully integrates it to the build and gives access to all its *targets*.

The original [article][3] recommends two sets of definitions to achieve that with [googletest][4]  :
- a *CMakeLists.txt.in* file, which holds the external project references (namely, the official github location) 
- a *CMakeLists.txt* file, which defines the targets of your application or your library.

With SFML as the external project, all I had to do was basically to fill the configuration files with the right url, and define my own target on top of the dependency build. Here is a short presentation of the *P.O.C*.

## The Workspace

A global `CMakeLists.txt` is located at the root of the folder --- for lisibility purposes --- as you can see below. It sets the targets folders for the project. The dependency and the graphical app targets are defined in a sub-project.

```shell
project/
├── App
│   ├── CMakeLists.txt
│   ├── CMakeLists.txt.in
│   ├── include
│   │   └── App.h
│   └── src
│       ├── App.cpp
│       └── main.cpp
├── CMakeLists.txt
├── bin
├── lib
└── build
```
> **Note**: there is a global CMakeLists.txt file to set the targets folders locations. The graphical app target and the SFML dependency configuration are defined in a sub-folder.

### The Global *CMakeList.txt*

```cmake
# CMakeList.txt : Upper level configuration file
cmake_minimum_required (VERSION 3.10)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY
    ${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}/)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY
    ${CMAKE_SOURCE_DIR}/lib/${CMAKE_BUILD_TYPE}/)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY 
    ${CMAKE_SOURCE_DIR}/lib/${CMAKE_BUILD_TYPE}/)

project (SFMLCMAKE C CXX)
add_subdirectory ("App")
```
> **Note**: the dependency and the graphical app targets are defined in a sub-project called App --- see the next paragraphs for further details.

### Dependency Location : CMakeList.txt.in

*CMakeList.txt.in* holds the *SFML* official location data. This file is used during the configuration of graphical app targets.

```cmake
cmake_minimum_required (VERSION 3.8)
project(sfml-download NONE)
include(ExternalProject)
ExternalProject_Add(sfml
  GIT_REPOSITORY    https://github.com/SFML/SFML.git
  GIT_TAG           master
  SOURCE_DIR        "${CMAKE_SOURCE_DIR}/build/sfml-src"
  BINARY_DIR        "${CMAKE_SOURCE_DIR}/build/sfml-build"
  CONFIGURE_COMMAND ""
  BUILD_COMMAND     ""
  INSTALL_COMMAND   ""
  TEST_COMMAND      ""
)
```
> **Note**: the SFML official repository is pulled at configuration time.

### *CMakeLists.txt* for the targets definition

This is where the magic is done. This file basically sets the dependency targets to be built at configuration time. Our graphical app target is defined at the end.

```cmake
cmake_minimum_required (VERSION 3.8)
project (app C CXX)
# build SFML targets ------------
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt.in
  ${CMAKE_SOURCE_DIR}/build/sfml-download/CMakeLists.txt
)
execute_process(
  COMMAND ${CMAKE_COMMAND} -G ${CMAKE_GENERATOR} .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/build/sfml-download
)
if(result)
  message(FATAL_ERROR "CMake step for sfml failed: ${result}")
endif()
execute_process(
  COMMAND ${CMAKE_COMMAND} --build .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/build/sfml-download
)
if(result)
  message(FATAL_ERROR "Build step for sfml failed: ${result}")
endif()
add_subdirectory(
  ${CMAKE_SOURCE_DIR}/build/sfml-src
  ${CMAKE_SOURCE_DIR}/build/sfml-build
)
#----------------------------------
set(SFML_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/build/sfml-build/include/)
set(APP_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include/)
file(GLOB_RECURSE APP_SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
include_directories(${SFML_INCLUDE_DIR} ${APP_INCLUDE_DIR})
# app target
add_executable (app ${APP_SRC_FILES}) 
if(WIN32 OR WIN64)
  target_link_libraries(app sfml-window sfml-system sfml-graphics)
else()
  target_link_libraries(app sfml-window sfml-system sfml-graphics pthread X11)
endif()
source_group("src" FILES ${APP_SRC_FILES})
source_group("include" FILES ${APP_INCLUDE_DIR}/*.h)
```
> **Note**: Lines 4 --- 27 set the dependency build --- you can see how the *CMakeList.txt.in* file is consumed at line 5. The graphical app target definition begins at line 34 (the configuration is cross-platform -- linux and windows).

### C++ Code

The main file *App/src/main.cpp*  basically launches two threads : one for the logic of the application --- the main thread --- and the other, for the display routines (the SFML window should be initialized in the main thread).

#### main.cpp file

```c++
#include "App.h"
#include <thread>
#include <SFML/Graphics.hpp>
#ifdef __linux__
#include <X11/Xlib.h>
#endif

int main()
{
#ifdef __linux__
  // init X threads
  XInitThreads();
#endif
  sf::ContextSettings settings;
  settings.antialiasingLevel = 10;	
  const unsigned int width = (App::DEFAULT_WIDTH*App::DEFAULT_RESX);
  const unsigned int height = (App::DEFAULT_HEIGHT*App::DEFAULT_RESY);
  sf::RenderWindow window (
    sf::VideoMode(width, height),
    "SFML & CMAKE",
    sf::Style::Titlebar | sf::Style::Close,
    settings
  );
  window.clear(sf::Color::Cyan);
  window.setFramerateLimit(120);
  window.setActive(false);
  // App definition
  App app;
  app.setWindow(&window);
  std::thread rendering_thread(&App::display, &app);
  app.run();
  rendering_thread.join();
  return 0;
}
```
> **Note**: The main.cpp file launches two threads : one for the logic of the application and another for the display. The source code is cross-platform (Linux and Windows)


## Configuration and build

If you fork the full source code from [here][1] (to get `App.h` and `App.cpp`), you should be able to type the following lines in a terminal at the root of the project folder.

On windows
```shell
  # on windows
  cmake  -G "Visual Studio 15 $(Version)" -S . -B ./build -DCMAKE_BUILD_TYPE=Debug ..
  cmake  --build ./build --config Debug --target app
```
On Linux
```shell
  # on linux
  mkdir build  
  cd build
  cmake -G "Unix Makefiles" .. -DCMAKE_BUILD_TYPE=Debug
  cmake --build ./ --target app --config Debug 
```
> **Note**:  For linux user, you might need to check [this][2] first.

## Run
You should be able to launch the executable located in the `bin` folder and see a nice (and clickable) cyan window.

```shell
./bin/Debug/app
```

![screenshot](/images/sfml-window.gif)
> **Note**: On ubuntu 18.08 with gcc 7.5

Smile! Now you are ready to take your graphical app wherever you want. Enjoy and feel free to send me your feedbacks!


[1]: https://github.com/kanmeugne/sfmlcmake
[2]: https://www.sfml-dev.org/tutorials/2.5/compile-with-cmake.php
[3]: https://crascit.com/2015/07/25/cmake-gtest/
[4]: https://github.com/google/googletest
[5]: https://cmake.org/
[6]: https://www.sfml-dev.org/documentation/2.5.1/
