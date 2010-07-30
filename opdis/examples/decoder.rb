#!/usr/bin/env ruby1.9
# Opdis Example: Decoder
# Custom Instruction Decoder used in linear disassembly of array of bytes
# Copyright 2010 Thoughtgang <http://www.thoughtgang.org>

require 'BFD'
require 'Opdis'

# ----------------------------------------------------------------------
# Decoder that wraps Generic Decoder. This bypasses the X86Decoder processing.
class CustomDecoder < Opdis::InstructionDecoder

  def decode(insn, hash)
    # TODO
  end

end

# ----------------------------------------------------------------------
# print an instruction in the standard disasm listing format:
#       VMA 8_hex_bytes instruction
def print_insn(insn)
  hex_str = insn.bytes.collect { |b| "%02X" % b }.join(' ')
  puts "%08X %-23.23s %s" % [ insn.vma, hex_str, insn.ascii ]
end

# ----------------------------------------------------------------------
def disasm_bytes( bytes )
  # custom decoder to use in disassembler
  decoder = CustomDecoder.new

  Opdis::Disassembler.new( insn_decoder: decoder ) do |dis|

    dis.disasm_entry( bytes ) { |i| print_insn(i) }

  end
end

# ----------------------------------------------------------------------
if __FILE__ == $0
  raise "Usage: #{$0} ARCH BYTE [BYTE...]" if ARGV.length < 2

  arch = ARGV.shift
  disasm_bytes( arch, ARGV.collect { |b| b.hex } )
end
