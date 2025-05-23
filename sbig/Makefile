#
CCLINK = gcc -m32
CFLAGS = -m32 -Wno-deprecated -Wall -I/usr/X11R6/include/X11
LDOPTIONS = -g  -L/usr/X11R6/lib
LDLIBS = -lXext -lX11 -lflimage -L/usr/X11R6/lib -lforms -lXpm -L/usr/local/lib -lcfitsio -L/usr/local/sbig -lsbigcam -lsbigudrv -lm


sbigab: sbigab.o  $(DEPLIBS)
	$(RM) $@
	gcc -m32 -o $@ $(LDOPTIONS) sbigab.o $(LDLIBS)

sbig2: sbig2.o cc.o $(DEPLIBS)
	$(RM) $@
	gcc -m32 -o $@ $(LDOPTIONS) sbig2.o cc.o $(LDLIBS)

sbigT: sbigT.o cc.o $(DEPLIBS)
	$(RM) $@
	gcc -m32 -o $@ $(LDOPTIONS) sbigT.o cc.o $(LDLIBS)

sbigth: sbigth.o  $(DEPLIBS)
	$(RM) $@
	gcc -m32 -D_REENTRANT -lpthread -o $@ $(LDOPTIONS) sbigth.o $(LDLIBS)

.PHONY: all

all: sbig2 sbigab sbig_cmd sbigT

clean:
	rm *.o

real_clean:
	rm -f *.o
	rm -f sbig2 sbigab sbig_cmd sbigT

mrproper: real_clean

