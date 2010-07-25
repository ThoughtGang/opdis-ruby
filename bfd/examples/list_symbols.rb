#!/usr/bin/env ruby1.9
# List BFD Symbols
# Copyright 2010 Thoughtgang <http://www.thoughtgang.org>

require 'BFD'

# Defined in /usr/include/bfd.h : typedef struct bfd_symbol
SYM_FLAGS={ 0x000001 => 'LOCAL',
            0x000002 => 'GLOBAL',
            0x000004 => 'DEBUGGING',
            0x000008 => 'FUNCTION',
            0x000020 => 'KEEP',
            0x000040 => 'KEEP_G',
            0x000080 => 'WEAK',
            0x000100 => 'SECTION_SYM',
            0x000200 => 'OLD_COMMON',
            0x000400 => 'NOT_AT_END',
            0x000800 => 'CONSTRUCTOR ',
            0x001000 => 'WARNING',
            0x002000 => 'INDIRECT',
            0x004000 => 'FILE',
            0x008000 => 'DYNAMIC',
            0x010000 => 'OBJECT',
            0x020000 => 'DEBUGGING_RELOC',
            0x040000 => 'THREAD_LOCAL',
            0x080000 => 'RELC',
            0x100000 => 'SRELC',
            0x200000 => 'SYNTHETIC',
            0x400000 => 'GNU_INDIRECT_FUNCTION',
            0x800000 => 'GNU_UNIQUE' }

def sym_flag_strings(flags)
  f = []
  SYM_FLAGS.each { |k,v| f << v if (flags & k > 0) }
  return f
end

def display_symbol(sym)
  puts "%s : 0x%X" % [sym.name, sym.value]
  puts "\t#{sym.binding}"
  puts "\t#{sym.section}"
  puts "\tFlags: #{sym_flag_strings(sym.flags).join(',')}"
end

def list_syms(filename)
  tgt = Bfd::Target.new(filename, {})
  puts "#{tgt.id}: #{tgt.filename}"
  tgt.symbols.keys.sort.each { |name| display_symbol tgt.symbols[name] }
  puts
end

if __FILE__ == $0
  raise "Usage: #{$0} FILE [FILE...]" if ARGV.length == 0
  ARGV.each { |f| list_syms(f) }
end
