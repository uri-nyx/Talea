export LDFLAGS := -rc 524288

export CRT0 := $(SDK)/crt0.s
export NAIMEN := $(abspath .)

ALL_DIRS := $(patsubst %/,%,$(wildcard */))

LIBS := $(filter lib%, $(ALL_DIRS))
EXCLUDES := tps_a tps_b SoftFloat-2c $(LIBS)
APPS := $(filter-out $(EXCLUDES), $(ALL_DIRS))

.PHONY: all libs apps img clean

all: libs apps img

libs:
	@for dir in $(LIBS); do $(MAKE) -C $$dir; done

apps:
	@for dir in $(APPS); do $(MAKE) -C $$dir; done

img:
	@echo "Populating TPS A..."
	cd tps_a && mcopy -m -o -i ../../build/akai.img -s * ::/ && cd ..
	@echo "Populating TPS B..."
	cd tps_b && mcopy -m -o -i ../../build/test.img -s * ::/ && cd ..

clean:
	@for dir in $(ALL_DIRS); do $(MAKE) -C $$dir clean; done
