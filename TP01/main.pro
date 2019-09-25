GLM_PATH  = ../../ext/glm-0.9.4.1

TEMPLATE  = app
TARGET    = tp01

LIBS     += -lGLEW -lGL -lGLU -lm
INCLUDEPATH  += $${GLEW_PATH}/include  $${GLM_PATH}

SOURCES   = main.cpp viewer.cpp
HEADERS   = viewer.h

CONFIG   += qt opengl warn_on thread uic4 release
QT       *= xml opengl core

