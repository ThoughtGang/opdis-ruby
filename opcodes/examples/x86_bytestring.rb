#!/usr/bin/env ruby1.9
# Disassemble x86 Bytes
# Copyright 2010 Thoughtgang <http://www.thoughtgang.org>

require 'Opcodes'

# print a disassembled instruction in standard disasm listing format
def print_insn( insn, buf )
  # Unpack instruction bytes from binary string in buf
  bytes = buf.unpack('C' * buf.length)[insn[:vma]...(insn[:vma]+insn[:size])]

  # Convert bytes to hex strings
  hex_str = bytes.collect { |b| "%02X" % b }.join(' ')

  # Format as hexump, showing up to 8 bytes (8*2 hex + 8-1 spaces)
  puts "%08X %-23.23s %s" % [insn[:vma], hex_str, insn[:insn].join(' ')]
end

# disassemble binary string using x86 architecture
def disasm_bytes( bytes )
  # Disassembler for x86
  disasm = Opcodes::Disassembler.new( arch: 'x86' )

  # disassemble until end of buffer is reached
  pos = 0
  while ( pos < bytes.length )
    insn = disasm.disasm_insn( bytes, vma: pos )
    print_insn insn, bytes
    pos += 1
  end

end

# Convert array of hex byte strings to a binary string
def hex_to_bytes( bytes )
  bytes.collect { |b| b.hex }.pack( 'C' * bytes.length )
end

if __FILE__ == $0
  raise "Usage: #{$0} BYTES" if ARGV.length == 0

  disasm_bytes( hex_to_bytes(ARGV) )
end
