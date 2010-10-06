#!/usr/bin/env ruby
# Opdis Example: Array
# Linear disassembly of array of bytes
# Copyright 2010 Thoughtgang <http://www.thoughtgang.org>

require 'Opdis'

def disasm_bytes( arch, bytes )
  Opdis::Disassembler.new( :arch => arch ) do |dis|
    dis.disassemble( bytes ) { |i| puts i }
  end
end

def hex_to_array( bytes )
  bytes.collect { |b| b.hex }
end

if __FILE__ == $0
  raise "Usage: #{$0} ARCH BYTE [BYTE...]" if ARGV.length < 2

  arch = ARGV.shift
  disasm_bytes( arch, hex_to_array(ARGV) )
end
