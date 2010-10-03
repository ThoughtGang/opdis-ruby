# Makefile for Magic gem build

NAME		= 	Magic

RUBY_VERSION	=	1.9
RAKE		=	rake$(RUBY_VERSION)
RUBY		=	ruby$(RUBY_VERSION)

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
	rm -f *.gem
