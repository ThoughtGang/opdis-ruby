#!/usr/bin/env ruby1.9
# Opdis Example: BFD Entry Point
# Control-flow disassembly of a BFD object file from its entry point
# Copyright 2010 Thoughtgang <http://www.thoughtgang.org>

require 'BFD'
require 'Opdis'

def disasm_entry( tgt )
  Opdis::Disassembler.new() do |dis|
    # NOTE: This will print instructions as they are disassembled
    #       (i.e. not in order)
    dis.disasm_entry( tgt ) { |i| puts i }
  end
end

if __FILE__ == $0
  raise "Usage: #{$0} FILE [FILE...]" if ARGV.length == 0

  ARGV.each do |filename| 
    Bfd::Target.new(filename) { |f| disasm_entry( f ) }
  end

end
