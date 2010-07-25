#!/usr/bin/env ruby1.9
# Show BFD Info
# Copyright 2010 Thoughtgang <http://www.thoughtgang.org>

require 'BFD'

# Defined in /usr/include/bfd.h : enum bfd_flavour
FLAVOURS=%w{ unknown aout coff ecoff xcoff elf ieee nlm oasys tekhex srec
             verilog ihex som os0k versados msdos ovax evax mmo mach_o
             pef pef_xlib sym }
             
# Defined in /usr/include/bfd.h : enum bfd_endian
ENDIAN=%w{ big little unknown }

# Defined in /usr/include/bfd.h : struct bfd 
FORMAT_FLAGS={ 0x0001 => 'HAS_RELOC',
               0x0002 => 'EXEC_P',
               0x0004 => 'LINEN',
               0x0008 => 'DEBUG',
               0x0010 => 'SYMS',
               0x0020 => 'LOCALS',
               0x0040 => 'DYNAMIC',
               0x0080 => 'WP_TEXT',
               0x0100 => 'D_PAGED',
               0x0200 => 'IS_RELAXABLE',
               0x0400 => 'TRADITIONAL_FORMAT',
               0x0800 => 'IN_MEMORY',
               0x1000 => 'HAS_LOAD_PAGE',
               0x2000 => 'LINKER_CREATED',
               0x4000 => 'DETERMINISTIC_OUTPUT' }
def format_flag_strings(flags)
  f = []
  FORMAT_FLAGS.each { |k,v| f << v if (flags & k > 0) }
  return f
end

def display_info(filename)
  tgt = Bfd::Target.new(filename, {})
  puts "#{tgt.id}: #{tgt.filename}"

  flags = format_flag_strings(tgt.format_flags)
  puts "#{FLAVOURS[tgt.flavour]} #{tgt.format}: #{flags.join(',')}"

  flags = format_flag_strings(tgt.type_flags)
  puts "#{tgt.type}: #{flags.join(',')}"

  puts "#{tgt.sections.length} Sections #{tgt.symbols.length} Symbols"

  puts "Info:"
  info = tgt.arch_info
  info[:endian] = ENDIAN[tgt.endian]
  info[:entry] = "0x%X" % (tgt.start_address) if tgt.start_address
  info.keys.sort.each {|k| puts "\t#{k}: #{info[k]}" } 

  puts
end

if __FILE__ == $0
  raise "Usage: #{$0} FILE [FILE...]" if ARGV.length == 0
  ARGV.each { |f| display_info(f) }
end
