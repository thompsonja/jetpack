GL_LIB_PATH = ../lib

INCLUDE_PATH = -I./ \
               -ICommon \
               -I$(GL_LIB_PATH)/FreeImage/Dist \
               -I$(GL_LIB_PATH)/glut/include \
               -I$(GL_LIB_PATH)/SDL2/include \
               -I$(GL_LIB_PATH)/SDL2_mixer-2.0.0/include

SOURCE_FILES = Audio.cpp \
               Billboard.cpp \
               Box.cpp \
               EnergyBar.cpp \
               Environment.cpp \
               Image.cpp \
               Light.cpp \
               main.cpp \
               Model.cpp \
               Renderer.cpp \
               SphereRing.cpp \
               User.cpp \

CXX = g++
CXXFLAGS = -std=c++0x -pg -Wall -g
LDFLAGS = -g
LDLIBS = -lGL -lGLU -lfreeimage -lglut -lSDL2 -lSDL2_mixer -lSDL2main

%.o : %.cpp
	@echo Compiling $<
	$(CXX) -c $(CXXFLAGS) $(INCLUDE_PATH) -o $@ $<

%.depend: %.cpp
	@echo Dependencies $<
	$(CXX) -MM $(CXXFLAGS) $(INCLUDE_PATH) -o $@ $<

#CPPFLAGS=-g $(shell root-config --cflags)
#LDFLAGS=-g $(shell root-config --ldflags)
#LDLIBS=$(shell root-config --libs)
OBJS=$(subst .cpp,.o,$(SOURCE_FILES))

OBJECT_FILES = $(SOURCE_FILES:%.cpp=%.o)
DEPEND_FILES = $(SOURCE_FILES:%.cpp=%.depend)

all: $(DEPEND_FILES) jetpack

jetpack: $(OBJECT_FILES)
	$(CXX) $(CXXFLAGS) $(INCLUDE_PATH) -o $@ $(OBJECT_FILES) $(LDLIBS)

#jetpack: $(OBJECT_FILES)
#	$(CXX) $(CXXFLAGS) $(INCLUDE_PATH) -o jetpack.o $(OBJS) $(LDLIBS) 

#jetpack.o: $(SOURCE_FILES)

clean:
	rm -f *.o
	rm -f *.depend
	rm -f *.out
