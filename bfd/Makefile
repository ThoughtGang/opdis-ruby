# Makefile for BFD gem build

NAME		= 	BFD
GEMSPEC		=	$(NAME).gemspec

ifeq "$(RUBY_VERSION)" ""
RUBY_VERSION	=	1.9
endif
export RUBY_VERSION

GEM		=	gem$(RUBY_VERSION)
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
	$(GEM) build $(GEMSPEC)

clean: 
	echo Cleaning $(NAME) build dir
	[ -e module/Makefile ] && (cd module && make clean)
	[ -e module/Makefile ] && (cd module && make distclean)
	rm -f *.gem
