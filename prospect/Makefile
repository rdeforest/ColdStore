all: prospect

prospect: prospect.c
	$(CC) $< -o prospect


# don't ask
mangle: mangle.cc
	eg++ -frtti -fno-implicit-templates -c mangle.cc -o mangle.o
	 # objdump --syms mangle.o
	nm mangle.o

.PHONY: clean distclean

clean:
	rm -f *.o *~

distclean: clean
	rm -f prospect