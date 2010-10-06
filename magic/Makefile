# Makefile for Magic gem build

NAME		= 	Magic

RUBY		=	ruby$(RUBY_VERSION)
RAKE		=	rake$(RUBY_VERSION)

# ----------------------------------------------------------------------
#  TARGETS

all: 
	echo Building $(NAME) extension
	[ -e module/Makefile ] || (cd module && $(RUBY) extconf.rb)
	(cd module && make)


gem:
	echo Building $(NAME) gem
	[ -e module/Makefile ] && (cd module && make clean)
	$(RAKE) gem

test:
	$(RAKE) test

doc:
	$(RAKE) rdoc

clean: 
	echo Cleaning $(NAME) build dir
	[ -e module/Makefile ] && (cd module && make clean)
	[ -e module/Makefile ] && (cd module && make distclean)
	rm -f *.gem
