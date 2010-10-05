#!/usr/bin/env ruby
# List BFD Symbols
# Copyright 2010 Thoughtgang <http://www.thoughtgang.org>

require 'BFD'

def display_symbol(sym)
  puts "%s : 0x%X" % [sym.name, sym.value]
  puts "\t#{sym.binding}"
  puts "\t#{sym.section}"
  puts "\tFlags: #{sym.flags.join(',')}"
end

def list_syms(filename)
  Bfd::Target.new(filename) do |tgt|
    puts "#{tgt.id}: #{tgt.filename}"
    tgt.symbols.keys.sort.each { |name| display_symbol tgt.symbols[name] }
    puts
  end
end

if __FILE__ == $0
  raise "Usage: #{$0} FILE [FILE...]" if ARGV.length == 0
  ARGV.each { |f| list_syms(f) }
end
