CC := cc
CFLAGS := -o talea -I include -L lib -l SDL2 -O2
SRC := src/
DOC := doc/
LIT := lit/

run: talea
	./talea
	rm talea

talea: talea.c
	$(CC) $(SRC)talea.c include/inprint/inprint2.c $(CFLAGS)

talea.c: talea.html
	lit -t --compiler -odir $(SRC) $(LIT)talea.lit

talea.html:
	lit -w --md-compiler pandoc -odir $(DOC) $(LIT)talea.lit