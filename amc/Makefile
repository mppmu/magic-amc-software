CCLINK = gcc -m32
CFLAGS = -m32 -Wno-deprecated -Wall
LDFLAGS = -g  -L/usr/X11R6/lib

LDLIBS = -lXext -lX11 -lflimage -L/usr/X11R6/lib -lforms -lXpm  -lcfitsio -lsbigcam -lsbigudrv -lm

ab.o: ab.c AMCpanel.h AMCpower.h AMCconst.h

ha_m1.o: ha_m1.c *.h *.inc


# amcab.o: amcab.c AMCpanel.h AMCpower.h AMCgui.h CC.h

# amcabX.o: amcabX.c AMCpanel.h AMCpower.h AMCgui.h CC.h





amcabX: amcabX.o ab.o      ha_m1.o ha_m2ab.o $(DEPLIBS)
	$(RM) $@
	gcc -m32 -pthread -o $@ $(LDFLAGS) amcabX.o ab.o      ha_m2ab.o   $(LDLIBS)

amcab: amcab.o ab.o      ha_m1.o ha_m2ab.o $(DEPLIBS)
	$(RM) $@
	gcc -m32 -pthread -o $@ $(LDFLAGS) amcab.o ab.o      ha_m2ab.o   $(LDLIBS)


amc: amcab.o ab.o      ha_m1.o ha_m2ab.o $(DEPLIBS)
	$(RM) $@
	g++ -g -m32 -pthread -o $@ $(LDFLAGS) amcab.o ab.o      ha_m1.o   $(LDLIBS)





.DEFAULT_GOAL := all

.PHONY: all

all: amc amcab

clean:
	rm *.o

real_clean:
	rm -f *.o
	rm -f amc amcab amcabX

mrproper: real_clean

