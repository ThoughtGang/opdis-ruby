#!/usr/bin/env ruby
# Opdis Example: BFD Symbol
# Disassembly of a BFD object file from named symbols
# Copyright 2010 Thoughtgang <http://www.thoughtgang.org>

require 'BFD'
require 'Opdis'

def disasm_symbol( sym )
  raise "Symbol not found in BFD" if not sym
  raise "Symbol #{sym.name} has no VMA!" if not sym.value

  puts "#{sym.name}:"
  Opdis::Disassembler.new() do |dis|
    # NOTE: This will print instructions as they are disassembled
    #       i.e. not necessarily in order
    dis.disasm_symbol( sym ) { |i| puts "\t#{i}" }
  end
end

if __FILE__ == $0
  raise "Usage: #{$0} FILE SYMBOL [SYMBOL...]" if ARGV.length == 0

  Bfd::Target.new(ARGV.shift) do |tgt|
    ARGV.each { |name| disasm_symbol( tgt.symbols[name] ) }
  end
end
