SHELL := /bin/bash
#MAKEFLAGS += --silent
N := 7 8 9
I := 01 02 04 08 16

all: mainp
main:
	mpicxx -Wall main.cpp -o main -std=c++11
mainp: main.cpp
	mpixlC -Wall main.cpp -o main -std=c++11
submit: mainp
	for n in $(N); do \
		for i in $(I); do \
			mpisubmit.pl -p $$i -w 00:30 --stdout out_$$n-$$i --stderr err_$$n-$$i ./main $$n 0 ; \
		done \
	done
clean:
	rm main out_* err_*
