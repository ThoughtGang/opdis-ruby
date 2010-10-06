#!/usr/bin/env ruby
# Opdis Example: X86Decoder
# Custom X86 Decoder used in linear disassembly of array of bytes
# Copyright 2010 Thoughtgang <http://www.thoughtgang.org>

require 'BFD'
require 'Opdis'

# ----------------------------------------------------------------------
# Decoder that wraps X86AttDecoder.
class CustomX86Decoder < Opdis::X86Decoder

  def decode(insn, hash)
    # Invoke built-in X86 AT&T instruction decoder
    super(insn, hash)
    # TODO: something interesting
  end

end

# ----------------------------------------------------------------------
# print an instruction in the standard disasm listing format:
#       VMA 8_hex_bytes instruction
def print_insn(insn)
  hex_str = insn.bytes.bytes.collect { |b| "%02X" % b }.join(' ')
  puts "%08X %-23.23s %s" % [ insn.vma, hex_str, insn.ascii ]
end

# ----------------------------------------------------------------------
def disasm_bytes( bytes )
  # custom decoder to use in disassembler
  decoder = CustomX86Decoder.new

  Opdis::Disassembler.new( :arch => 'x86', :insn_decoder => decoder ) do |dis|

    dis.disassemble( bytes ) { |i| print_insn(i) }

  end
end

# ----------------------------------------------------------------------
if __FILE__ == $0
  raise "Usage: #{$0} BYTE [BYTE...]" if ARGV.length < 2

  disasm_bytes( ARGV.collect { |b| b.hex } )
end
