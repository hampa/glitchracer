all = gr 

CC = g++
SOURCES=glitchracer.cpp sdl.cpp drawstuff.cpp util.cpp font.cpp povray.cpp texture.cpp ode_extra.cpp gr_car.cpp font.cpp tb_math.cpp gr_car.h blood.cpp blood.h gr_passenger.cpp gr_passenger.h gr_geom.h

CFLAGS = --float-store -fno-rtti -I/usr/local/include/SDL -L/usr/local/lib
DFLAGS = -DWITH_COPY_PROTECTION -DOSX
UNI_CFLAGS = --float-store -fno-rtti -I/System/Library/Frameworks/GLUT.framework/Headers -I /System/Library/Frameworks/OpenGL.framework/Headers/ -I /Library/Frameworks/SDL.framework/Headers/  -I/usr/local/include/SDL/ 
UNI_FLAGS = --float-store -fno-rtti -I/System/Library/Frameworks/GLUT.framework/Headers -I /System/Library/Frameworks/OpenGL.framework/Headers/ -I../include -I../include/SDL -F../Frameworks/ -I../include/ode/
LINUX_CFLAGS = -L../lib/linux/
LINUX_CFLAGS = -L../lib/linux/
LINUX_CFLAGS = -L../lib/linux_us/
LINUX_IFLAGS = -I../include/ -I/usr/include/GL -I/usr/include/SDL 
LINUX_DFLAGS = -DWITH_COPY_PROTECTION -DLINUX 
OSX_CFLAGS = -L../lib/osx
FRAMEWORK = -framework Cocoa -framework OpenGL -framework GLUT 
FRAMEWORK_SDL = -framework SDL -framework SDL_mixer
UNI_FRAMEWORK = -framework Cocoa -framework OpenGL -framework GLUT 
LDFLAGS = -lode -lSDLMain -lSDL -lSDL_mixer
UNI_LDFLAGS = -lode -lSDLMain -lSDL -lSDL_mixer
PPC_LDFLAGS = -lode -lSDLMain 
LINUX_LDFLAGS = -lode -lSDL -lSDL_mixer -lglut

gr : $(SOURCES) 
	$(CC) --save-temps -DWITH_CYLINDER -DdCYLINDER_ENABLED -DDEBUG $(DFLAGS) $(UNI_FLAGS)  $(UNI_FRAMEWORK) $(FRAMEWORK_SDL) $(OSX_CFLAGS) -o $@ $^ -ggdb -lode-0.8-rel -lSDLMain && ./gr 
	#$(CC) --save-temps -DWITH_CAPSULE -DDEBUG $(DFLAGS) $(UNI_FLAGS)  $(UNI_FRAMEWORK) $(FRAMEWORK_SDL) $(OSX_CFLAGS) -o $@ $^ -ggdb -lode -lSDLMain && ./gr 
	#$(CC) --save-temps -DDEBUG $(DFLAGS) $(UNI_FLAGS)  $(UNI_FRAMEWORK) $(FRAMEWORK_SDL) $(OSX_CFLAGS) -o $@ $^ -ggdb -lode -lSDLMain && ./gr 

test_buggy : test_buggy.cpp sdl.cpp drawstuff.cpp util.cpp font.cpp povray.cpp texture.cpp ode_extra.cpp
	$(CC) --save-temps -DDEBUG $(DFLAGS) $(UNI_FLAGS)  $(UNI_FRAMEWORK) $(FRAMEWORK_SDL) $(OSX_CFLAGS) -o $@ $^ -ggdb -lode -lSDLMain && ./test_buggy

bs : main.cpp
	$(CC) --save-temps -DWITH_CAPSULE -DdCYLINDER_ENABLED -DDEBUG $(DFLAGS) $(UNI_FLAGS)  $(UNI_FRAMEWORK) $(FRAMEWORK_SDL) $(OSX_CFLAGS) -o $@ $^ -ggdb -lode-float -lSDLMain -llua && ./bs
	

clean : 
	rm -f *.o *.obj *.s *.ii *.gch *.exe 

tag :
	test -e /opt/local/bin/ctags && /opt/local/bin/ctags *.cpp *.h || ctags *.cpp *.h
