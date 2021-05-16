sim: sim.c
	gcc sim.c -o sim.o -lm

run:
	make run1 && make run2 && make run3

run1: sim.c
	make && ./sim.o -f inputs/trace1.trc -s 1024 -b 16 -a 8 -r RR

run2: sim.c
	make && ./sim.o -f inputs/trace1.trc -s 1024 -b 4 -a 1 -r RR

run3: sim.c
	make && ./sim.o -f inputs/trace1.trc -s 2048 -b 8 -a 2 -r RR

run4: sim.c
	make && ./sim.o -f inputs/trace1.trc -s 2048 -b 8 -a 4 -r RR

run5: sim.c
	make && ./sim.o -f inputs/trace1.trc -s 4096 -b 8 -a 1 -r RND

run6: sim.c
	make && ./sim.o -f inputs/trace1.trc -s 2048 -b 4 -a 4 -r RR

run7: sim.c
	make && ./sim.o -f inputs/trace1.trc -s 1024 -b 4 -a 4 -r RR

run8: sim.c
	make && ./sim.o -f inputs/trace1.trc -s 1024 -b 4 -a 1 -r RR

clean:
	rm *.o