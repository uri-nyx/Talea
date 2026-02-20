CPPFLAGS := -zI -I.

.PHONY:		all clean

SRCS = softfloat.c
OBJS = $(SRCS:.c=.o)
LIB = libf.a

all: $(LIB)

%.o:%.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(LIB):$(OBJS)
	$(AR) -rs $@ $(OBJS)


clean:
		rm -f *~ $(LIB) $(OBJS)
