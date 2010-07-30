#!/usr/bin/env ruby1.9
# Opdis Example: Visited Address Tracker
# Custom Visited Address Tracker used in control-flow disassembly of 
# BFD entry point
# Copyright 2010 Thoughtgang <http://www.thoughtgang.org>

require 'BFD'
require 'Opdis'

# ----------------------------------------------------------------------
class CustomTracker < Opdis::VisitedAddressTracker

  attr_reader :visited_addresses

  def initialize()
    visited_addresses = {}
  end

  def visited?( insn )
    return visited_addresses.fetch(insn.vma, false)
  end

  def visit( insn )
    visited_addresses[insn.vma] = true
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
def disasm_entry( tgt )
  # custom resolver to use in disassembler
  tracker = CustomTracker.new

  Opdis::Disassembler.new( addr_tracker: tracker ) do |dis|

    dis.disasm_entry( tgt ) do |insn|
      # Store instruction as having been visited
      tracker.visit(insn)

    # Print instructions in order of VMA
    end.values.sort_by( |i| i.vma ).each { |i| print_insn(i) }
  end
end

# ----------------------------------------------------------------------
if __FILE__ == $0
  raise "Usage: #{$0} FILE [FILE...]" if ARGV.length == 0

  ARGV.each do |filename| 
    Bfd::Target.new(filename) { |f| disasm_entry( f ) }
  end

end
