
PROCESSOR_H = ../../../processors/SIRIUS-LCC.h
SOFTFLOAT_MACROS = ../softfloat-macros

OBJ = .o
EXE =
INCLUDES = -I. -I..
COMPILE_C = $(CC) -zI $(INCLUDES) -I- -c -o $@ 
LINK = $(LD) $(LDFLAGS) -o $@

#------------------------------------------------------------------------------
# Probably okay below here.
#------------------------------------------------------------------------------

ALL: softfloat$(OBJ) 

milieu.h: $(PROCESSOR_H)
	touch milieu.h

softfloat$(OBJ): milieu.h softfloat.h softfloat-specialize $(SOFTFLOAT_MACROS) ../softfloat.c
	$(COMPILE_C) ../softfloat.c
