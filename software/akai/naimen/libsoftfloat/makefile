CPPFLAGS := -zI -I.

.PHONY:		all clean

DIR = SoftFloat-2c/softfloat/bits32/SIRIUS-Akai-LCC/
OBJS = $(DIR)/softfloat.o
LIB = libf.a

all: $(LIB)

$(OBJS):
	make -C $(DIR) ALL

$(LIB):$(OBJS)
	$(AR) -rs $@ $(OBJS)

clean:
	rm -f *~ $(LIB) $(OBJS)
