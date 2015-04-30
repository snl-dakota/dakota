
default:  all
	exit

include ../acro/packages/pico/Makefile

INCLUDE += -DDEBUGGING
#OPTIMIZATION=-g

#LDLIBS=-lutilib -lm

doclean:
	$(RM) -f *.exe *.o serialBB

all:
	$(MYMAKE) doclean
	$(MYMAKE) serialBB
