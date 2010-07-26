#!/usr/bin/env ruby1.9
# Disassemble BFD Section
# Copyright 2010 Thoughtgang <http://www.thoughtgang.org>

require 'BFD'
require 'Opcodes'

def print_insn( insn )
  puts "%08X\t%s" % [ insn[:vma], insn[:insn].join(' ') ]
end

def disasm_sections(filename, sections)
  # BFD for target file
  tgt = Bfd::Target.new(filename, {})
  puts "#{tgt.id}: #{tgt.filename}"

  # Disassembler for BFD
  disasm = Opcodes::Disassembler.new( bfd: tgt )

  # Disassemble until end of buffer is reached
  sections.each do |name| 
    sec = tgt.sections[name]
    raise "Section #{name} not in #{tgt.filename}" if not sec

    disasm.disasm(sec, {}).each { |i| print_insn i }
  end
end

if __FILE__ == $0
  raise "Usage: #{$0} FILE SECTION [SECTION...]" if ARGV.length < 2
  filename = ARGV.shift

  disasm_sections(filename, ARGV)
end
