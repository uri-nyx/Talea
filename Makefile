CC := cc
CFLAGS := -o talea -I include -L lib -l SDL2 -O2
SRC := src/
DOC := docs/
LIT := lit/
Literate-generated-toc := $(shell ls $(DOC)_book | grep contents\.html)

run: talea
	./talea
	rm talea

talea: talea.c
	$(CC) $(SRC)talea.c include/inprint/inprint2.c $(CFLAGS)

talea.c: rename
	lit -t -odir $(SRC) $(LIT)talea.lit

gh-page: talea.html
	mv $(DOC)_book $(DOC)book
	$(shell mv $(DOC)_book/*_contents.html $(DOC)_book/index.html)

talea.html:
	$(if $(filter $(DOC)book,$(wildcard *)), rm -r $(DOC)book)
	lit -w --md-compiler pandoc -odir $(DOC) $(LIT)talea.lit