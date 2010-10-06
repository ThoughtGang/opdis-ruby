#!/usr/bin/env ruby
# Opdis Example: BFD Section
# Linear disassembly of a section in a BFD object file
# Copyright 2010 Thoughtgang <http://www.thoughtgang.org>

require 'BFD'
require 'Opdis'

def disasm_section( sec )
  puts "Section not found in BFD" if not sec

  puts "#{sec.name}:"
  Opdis::Disassembler.new() do |dis|
    dis.disasm_section( sec ) { |i| puts "\t#{i}" }
  end
end

if __FILE__ == $0
  raise "Usage: #{$0} FILE SECTION [SECTION...]" if ARGV.length == 0

  Bfd::Target.new(ARGV.shift) do |tgt| 
    ARGV.each { |name| disasm_section( tgt.sections[name] ) }
  end
end
