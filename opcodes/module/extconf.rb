#!/usrbin/ruby1.9
# Opcodes Ruby extension config file
# Copyright 2010 Thoughtgang <http://www.thoughtgang.org>
# Options:
#   --with-opcodes-dir=path
#   --with-opcodes-include=path
#   --with-opcodes-lib=path
#   --with-objdump=path
# see README for more info.

require 'mkmf'

have_library('opcodes', 'init_disassemble_info')
dir_config('opcodes')

# ----------------------------------------------------------------------
# Architectures supported by binutils
# These have to be specified on the command line, as binutils does not
# provide any clue as to which architectures it has been compiled for
# on the local machine.
# NOTE: These were compiled from the list of architectures in bfd.h .
	
  ARCH= %w[ m32c alpha arc arm avr bfin cr16 cris crx d10v d30v
	          dlx h8300 h8500 hppa i370 i386 i860 i960 ia64 ip2k fr30
	          lm32 m32r m68k m88k maxq mt microblaze msp430 ns32k mcore
	          mep mips mmix mn10200 mn10300 openrisc or32 pdp11 pj
	          powerpc rs6000 s390 score sh sparc spu tic30 tic4x tic54x
	          tic80 v850 w65 xstormy16 xc16x xtensa z80 z8k vax frv
	          moxie iq2000 m32c ]

# Define all architecture options
ARCH.each { |a| with_config( "ARCH_#{a.upcase}" ) }

# ----------------------------------------------------------------------
# Detect architectures supported locally

SEEN_ARCH = []
def handle_bfd_arch( line )
  arch = line.strip
  if ARCH.include?(arch) and not SEEN_ARCH.include?(arch)
    puts "Adding architecture '#{arch}'"
    SEEN_ARCH << arch
    $CPPFLAGS += " -DARCH_#{arch.upcase}"
  end
end

# allow user to override the objump binary using --with-objdump=path
objdump_bin = with_config('objdump', 'objdump')

# use objdump -i to get supported architectures
`#{objdump_bin} -i`.split("\n").each { |line| handle_bfd_arch(line) }

# default to i386 if objdump failed to run or no architectures were found
$CPPFLAGS += " -DARCH_I386" if SEEN_ARCH.length == 0

# ----------------------------------------------------------------------
# Makefile

if RUBY_VERSION =~ /1.8/ then
      $CPPFLAGS += " -DRUBY_18"
elsif RUBY_VERSION =~ /1.9/ then
      $CPPFLAGS += " -DRUBY_19"
end

create_makefile('OpcodesExt')
