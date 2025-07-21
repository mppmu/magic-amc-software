#
CCLINK = gcc -m32
LDOPTIONS = -g  -L/usr/X11R6/lib

LDLIBS = -lXext -lX11 -lflimage -L/usr/X11R6/lib -lforms -lXpm  -lcfitsio -lsbigcam -lsbigudrv

ab.o: ab.c AMCpanel.h AMCpower.h

ha_m2.o: ha_m2.c *.h *.inc

ha_m2ab.o: ha_m2ab.c AMCpanel.h AMCpower.h

amcab.o: amcab.c AMCpanel.h AMCpower.h AMCgui.h CC.h

amcabX.o: amcabX.c AMCpanel.h AMCpower.h AMCgui.h CC.h

amcabX: amcabX.o ab.o      ha_m2ab.o $(DEPLIBS)
	$(RM) $@
	gcc -m32 -pthread -o $@ $(LDOPTIONS) amcabX.o ab.o      ha_m2ab.o   $(LDLIBS)

amcab: amcab.o ab.o      ha_m2ab.o $(DEPLIBS)
	$(RM) $@
	gcc -m32 -pthread -o $@ $(LDOPTIONS) amcab.o ab.o      ha_m2ab.o   $(LDLIBS)

amcap: amcap.o ap.o      ha_m2ab.o $(DEPLIBS)
	$(RM) $@
	gcc -m32 -pthread -o $@ $(LDOPTIONS) amcab.o ab.o      ha_m2ab.o   $(LDLIBS)

amc: amcab.o ab.o      ha_m2.o  $(DEPLIBS)
	$(RM) $@
	g++ -g -m32 -pthread -o $@ $(LDOPTIONS) amcab.o ab.o      ha_m2.o   $(LDLIBS)

amp: amcap.o ap.o      ha_m2.o  $(DEPLIBS)
	$(RM) $@
	g++ -g -m32 -pthread -o $@ $(LDOPTIONS) amcap.o ap.o      ha_m2.o   $(LDLIBS)

