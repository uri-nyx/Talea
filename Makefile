CC := cc
SRC := src/
DOC := docs/
LIT := lit/
INCLUDE := $(SRC)include/
CFLAGS := $(INCLUDE)inprint/inprint2.c -o talea -I $(INCLUDE) -L $(SRC)lib -l SDL2 -Wall -Wextra -std=c11 -O2

run: talea
	./talea
	rm talea

talea: talea.c
	$(CC) $(SRC)talea.c $(CFLAGS)

talea.c:
	lit -t -odir $(SRC) $(LIT)talea.lit

gh-page: talea.html
	mv $(DOC)_book $(DOC)book
	$(shell mv $(DOC)_book/*_contents.html $(DOC)_book/index.html)

talea.html:
	if [ -d $(DOC)book ]; then rm -r $(DOC)book; fi
	lit -w --md-compiler pandoc -odir $(DOC) $(LIT)talea.lit