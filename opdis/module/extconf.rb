#!/usr/bin/env ruby1.9
# Opdis Ruby extension config file
# Copyright 2010 Thoughtgang <http://www.thoughtgang.org>
# Options:
#   --with-bfd-dir=path_to_binutils_install_base (/usr)
#   --with-bfd-include=path_to_bfd.h (/usr/include)
#   --with-bfd-lib=path_to_libbfd.so (/usr/lib)
#   --with-opcodes-dir=path_to_binutils_install_base (/usr)
#   --with-opcodes-include=path_to_dis-asm.h (/usr/include)
#   --with-opcodes-lib=path_to_libopcodes.so (/usr/lib)
#   --with-opdis-dir=path_to_opdis_install_base (/usr/local)
#   --with-opdis-include=path_to_opdis_include_dir (/usr/local/include)
#   --with-opdis-lib=path_to_libopdis.so (/usr/local/lib)
#   --with-opdis=path_to_opdis_source_tree
#   --with-objdump=path_to_objdump_binary (objdump)
# See README for more info.

require 'mkmf'

def require_header(name)
  have_header(name) or raise "Missing header file #{name}"
end

def require_library(name, func) 
  have_library(name, func) or raise "Missing library #{name}"
end

def require_opdis_header(name, opdis_base)
  return require_header(name) if not opdis_base

  dirs = [opdis_base, opdis_base + "/opdis", opdis_base + "/include"]
  find_header(name, *dirs) or raise "#{name} not found in #{dirs}"
end

def require_opdis_library(name, func, opdis_base)
  return require_library(name, func) if not opdis_base

  dirs = [opdis_base, opdis_base + "/lib", opdis_base + '/dist',
          opdis_base + '/dist/.libs']
  find_library(name, func, *dirs) or raise "#{name} not found in #{dirs}"
end

# ----------------------------------------------------------------------
# BFD

# allow user to specify specific binutils distro
dir_config('binutils')

require_header('bfd.h')
require_library('bfd', 'bfd_init')

# ----------------------------------------------------------------------
# OPCODES

# allow user to override libopcodes
dir_config('opcodes')

require_header('dis-asm.h')
require_library('opcodes', 'init_disassemble_info')

# ----------------------------------------------------------------------
# OPDIS

dir_config('opdis')

# allow pointing to source code repo
opdis_base=with_config('opdis')

require_opdis_header('opdis/opdis.h', opdis_base)
require_opdis_header('opdis/model.h', opdis_base)
require_opdis_header('opdis/metadata.h', opdis_base)
require_opdis_library('opdis', 'opdis_init', opdis_base)

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
# Compatibility flags

if RUBY_VERSION =~ /1.8/ then
        $CPPFLAGS += " -DRUBY_18"
elsif RUBY_VERSION =~ /1.9/ then
        $CPPFLAGS += " -DRUBY_19"
end

# ----------------------------------------------------------------------
# Makefile

create_makefile('OpdisExt')
