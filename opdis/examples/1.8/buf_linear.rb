#!/usr/bin/env ruby1.8
# Opdis Example: String
# Linear disassembly of string of bytes
# Copyright 2010 Thoughtgang <http://www.thoughtgang.org>

require 'Opdis'

def disasm_bytes( arch, bytes )

  Opdis::Disassembler.new( :arch => arch ) do |dis|
    dis.disassemble( bytes ) { |i| puts i }
  end

end

def hex_to_string( bytes )
  bytes.collect { |b| b.hex }.pack( 'C' * bytes.length )
end

if __FILE__ == $0
  raise "Usage: #{$0} ARCH BYTE [BYTE...]" if ARGV.length < 2

  arch = ARGV.shift
  disasm_bytes( arch, hex_to_string(ARGV) )
end
