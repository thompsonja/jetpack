GL_LIB_PATH = ../

INCLUDE_PATH = ./ \
               Common \
               $(GL_LIB_PATH)/FreeImage/Dist \
               $(GL_LIB_PATH)/glut/include \
               $(GL_LIB_PATH)/SDL2/include \
               $(GL_LIB_PATH)/SDL2_mixer-2.0.0/include

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

CPPFLAGS=-g $(shell root-config --cflags)
LDFLAGS=-g $(shell root-config --ldflags)
LDLIBS=$(shell root-config --libs)
OBJS=$(subst .cpp,.o,$(SOURCE_FILES))

all: jetpack

jetpack: $(OBJS)
	g++ $(LDFLAGS) -o jetpack.o $(OBJS) $(LDLIBS)

jetpack.o: $(SOURCE_FILES)

clean:
	$(FM) $(OBJS)