# creates universal binaries of ppc and i386 libs
all : ode sdl

ode : libode.a
	ranlib $^

sdl : libSDLmain.a
	ranlib $^

libode.a : libode_i386.a libode_ppc.a
	lipo -create $^ -output $@ 

libSDLmain.a : libSDLmain_i386.a libSDLmain_ppc.a
	lipo -create $^ -output $@ 

clean :
	rm libode.a libSDLmain.a

