.PHONY: unittest

vpath %.h .:Unity/:../src
vpath %.c .:Unity/:../src

unittest: map.c unity.c main.c 
	@echo "test program for map compent"
	@rm *.o || true
	gcc -g3 -O0 -W -Wall -Wno-int-to-pointer-cast -Wno-pointer-to-int-cast -Wno-unused-variable -I. -I../src $^ -o $@
