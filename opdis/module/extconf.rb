#!/usr/bin/env ruby1.9
# NOTE: use --with-opdis=path_to_opdis_source to set opdis location

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
# Makefile
create_makefile('OpdisExt')
