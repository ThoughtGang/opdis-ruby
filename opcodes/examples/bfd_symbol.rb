#!/usr/bin/env ruby1.9
# Disassemble BFD Symbol
# Copyright 2010 Thoughtgang <http://www.thoughtgang.org>

require 'BFD'
require 'Opcodes'

def print_insn( insn )
  puts "%08X\t%s" % [ insn[:vma], insn[:insn].join(' ') ]
end

def disasm_symbols(filename, symbols)
  # BFD for target file
  tgt = Bfd::Target.new(filename)
  puts "#{tgt.id}: #{tgt.filename}"

  # Disassembler for BFD
  disasm = Opcodes::Disassembler.new( bfd: tgt )

  # Disassemble until end of buffer is reached
  symbols.each do |name| 
    sym = tgt.symbols[name]
    raise "Symbol #{name} not in #{tgt.filename}" if not sym

    disasm.disasm(sym).each { |i| print_insn i }
  end
end

if __FILE__ == $0
  raise "Usage: #{$0} FILE SYMBOL [SYMBOL...]" if ARGV.length < 2
  filename = ARGV.shift

  disasm_symbols(filename, ARGV)
end
