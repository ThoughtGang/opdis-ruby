#!/usr/bin/env ruby1.9
# List BFD Sections
# Copyright 2010 Thoughtgang <http://www.thoughtgang.org>

require 'BFD'

# Defined in /usr/include/bfd.h : typedef struct bfd_section
SEC_FLAGS={ 0x00000001 => 'ALLOC',
            0x00000002 => 'LOAD',
            0x00000004 => 'RELOC',
            0x00000008 => 'READONLY',
            0x00000010 => 'CODE',
            0x00000020 => 'DATA',
            0x00000040 => 'ROM',
            0x00000080 => 'CONSTRUCTOR',
            0x00000100 => 'HAS_CONTENTS',
            0x00000200 => 'NEVER_LOAD',
            0x00000400 => 'THREAD_LOCAL',
            0x00000800 => 'HAS_GOT_REF',
            0x00001000 => 'IS_COMMON',
            0x00002000 => 'DEBUGGING',
            0x00004000 => 'IN_MEMORY',
            0x00008000 => 'EXCLUDE',
            0x00010000 => 'SORT_ENTRIES',
            0x00020000 => 'LINK_ONCE',
            0x00040000 => 'LINK_DUPLICATES_ONE_ONLY',
            0x00080000 => 'LINK_DUPLICATES_SAME_SIZE',
            0x00100000 => 'LINKER_CREATED',
            0x00200000 => 'KEEP',
            0x00400000 => 'SMALL_DATA',
            0x00800000 => 'MERGE',
            0x01000000 => 'STRINGS',
            0x02000000 => 'GROUP',
            0x04000000 => 'COFF_SHARED_LIBRARY',
            0x08000000 => 'COFF_SHARED',
            0x10000000 => 'TIC54X_BLOCK',
            0x20000000 => 'TIC54X_CLINK',
            0x40000000 => 'COFF_NOREAD' }

def sec_flag_strings(flags)
  f = []
  SEC_FLAGS.each { |k,v| f << v if (flags & k > 0) }
  return f
end

def display_section(sec)
  puts "#{sec.id}: #{sec.name}"
  puts "\tFlags: #{sec_flag_strings(sec.flags).join(',')}"
  puts "\tFile Pos: 0x%X VMA: 0x%X LMA: 0x%X" % [sec.file_pos, sec.vma, sec.lma]
  puts "\tSize: #{sec.size} Raw: #{sec.raw_size}"
  # TODO: alignment_power
end

def list_secs(filename)
  tgt = Bfd::Target.new(filename, {})
  puts "#{tgt.id}: #{tgt.filename}"
  tgt.sections.values.sort_by { |s| s.index }.each { |s| display_section s }
  puts
end

if __FILE__ == $0
  raise "Usage: #{$0} FILE [FILE...]" if ARGV.length == 0
  ARGV.each { |f| list_secs(f) }
end
