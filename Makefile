CPP := g++
OLEVEL := -O3
FLAGS = -std=c++11 -I . -fsigned-char $(OLEVEL) $(CXXFLAGS)

SRCS = ChordGen.cpp \
	Element.cpp \
	ErrorScreen.cpp \
	GameMenu.cpp \
	Group.cpp \
	Instructions.cpp \
	MenuButtons.cpp \
	MusicGenerator.cpp \
	NoteSet.cpp \
	Options.cpp \
	Particle.cpp \
	PixelCosmApp.cpp \
	PlayFunctions.cpp \
	SandParser.cpp \
	SaveHandler.cpp \
	TitleScreen.cpp \
	TrackScanner.cpp \
	Update.cpp \
	main.cpp

OBJS = $(SRCS:.cpp=.o)

sandbox : $(OBJS)
	$(CPP) -Wall -o sandbox $(OBJS) $(FLAGS) `sdl-config --libs` -lSDL_mixer -lSDL_ttf

%.o : %.cpp
	$(CPP) -Wall -o $*.o -c $*.cpp $(FLAGS) `sdl-config --cflags`

clean:
	rm -f sandbox *.o
	rm -rf usr
