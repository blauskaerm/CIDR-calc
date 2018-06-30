CC:=gcc
APP:=cidrCalc

all: bin $(APP).o link

$(APP).o:
	$(CC) -c cidrCalc.c

link:
	$(CC) -o bin/$(APP) *.o

bin:
	@if [ ! -d bin ]; \
	then \
		mkdir bin; \
	fi

clean:
	rm -rf bin
	rm -rf *.o

.PHONY: clean
