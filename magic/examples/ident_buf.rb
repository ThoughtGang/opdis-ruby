#!/usr/bin/env ruby
# Show ident string for bytes
# Copyright 2010 Thoughtgang <http://www.thoughtgang.org>

require 'Magic'

def hex_to_string( bytes )
  bytes.collect { |b| b.hex }.pack( 'C' * bytes.length )
end

if __FILE__ == $0
  raise "Usage: #{$0} BYTE [BYTE...]" if ARGV.length == 0
  puts Magic::identify( hex_to_string(ARGV) )
end
