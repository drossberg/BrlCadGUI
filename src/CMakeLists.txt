#########################################################################
#
#  Permission to use, copy, modify, and/or distribute this software for any
#  purpose with or without fee is hereby granted.
#
#  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
#  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
#  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
#  SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
#  RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
#  CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
#  CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#
#########################################################################

set(CMAKE_AUTOMOC ON)
find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(OpenGL REQUIRED)

INCLUDE_DIRECTORIES(
        ${BrlCadGUI_SOURCE_DIR}/include
        ${BrlCadGUI_BINARY_DIR}/include
        ${BRLCAD_MOOSE_INCLUDE_DIR}
)

SET(GuiSources
    main.cpp
    DisplayManager.cpp
    GeometryModel.cpp
    MainWindow.cpp
    PlotGeometry.cpp
)

IF(MSVC)
    ADD_DEFINITIONS("-DBRLCAD_MOOSE_EXPORT=__declspec(dllimport)")
ELSE(MSVC)
    ADD_DEFINITIONS("-DBRLCAD_MOOSE_EXPORT=")
ENDIF(MSVC)

ADD_EXECUTABLE(GUI WIN32 ${GuiSources})
TARGET_LINK_LIBRARIES(GUI ${BRLCAD_MOOSE_LIBRARY} Qt5::Widgets OpenGL::GL)
