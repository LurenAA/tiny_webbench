out = webBench
middleware = webBench.o config.o

goal: ${middleware}
	g++ -o ${out} ${middleware}

webBench.o: webBench.c config.h
	g++ webBench.c -c -o webBench.o -g

config.o: config.h
	g++ config.c -g -c -o config.o

.PHONY: c, a

c: 
	rm ${middleware}

a: 
	rm ${middleware} ${out}