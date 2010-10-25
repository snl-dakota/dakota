# for use on ithilien

#E = /usr/local/matlab/7.4b/x86_64
E = /usr/local/matlab/7.6
I = $E/extern/include
#B = $E/bin/glnx86
B = $E/bin/glnxa64
S = ../sys.x86_64.Linux
L = $S/amplsolver.a -L$B -leng -lmx
W = -Wl,-rpath,$B -Wl,-rpath-link,$B
CFLAGS = -g
CC = cc

dakmatlab: dakmatlab.o
	$(CC) -o dakmatlab $W dakmatlab.o $L

dakmatlab.o: dakmatlab.c
	$(CC) -c $(CFLAGS) -I$S -I$I dakmatlab.c

dakampl: dakampl.c
	$(CC) -o dakampl -I$S dakampl.c $S/amplsolver.a -lm -ldl
