# Top-level Makefile for building Opdis gems.

DIRS	=	bfd	\
		opcodes	\
		opdis

# ----------------------------------------------------------------------
#  TARGETS

all: 
	echo Building modules
	for i in $(DIRS); do				\
		[ -d $$i ] && (cd $$i && make);		\
	done

gems:
	echo Building gems
	for i in $(DIRS); do				\
		[ -d $$i ] && (cd $$i && make gem);	\
	done

clean: 
	echo Cleaning build dirs
	for i in $(DIRS); do				\
		[ -d $$i ] && (cd $$i && make clean);		\
	done
