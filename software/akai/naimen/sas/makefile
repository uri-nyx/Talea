CPPFLAGS := -I$(SDK) -I$(NAIMEN)/libc/include
LIBS := -l $(NAIMEN)/libc/libcmin.a

all: sas

sas: sas.c lexer.c parser.c
	$(CC) $(CPPFLAGS) -c $(CRT0) sas.c lexer.c parser.c
	$(LD) $(LIBS) $(LDFLAGS) -o sas crt0.o sas.o lexer.o parser.o
	cp sas ../tps_a/bin

clean:
	rm crt0.o sas.o lexer.o parser.o sas
