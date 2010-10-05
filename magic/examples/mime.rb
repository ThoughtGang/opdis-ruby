#!/usr/bin/env ruby
# Show file MIME-type for file
# Copyright 2010 Thoughtgang <http://www.thoughtgang.org>

require 'Magic'

def display_ident(filename)
  File.open( filename ) { |f| puts Magic::identify(f, {:mime => true}) }
end

if __FILE__ == $0
  raise "Usage: #{$0} FILE [FILE...]" if ARGV.length == 0
  ARGV.each { |f| display_ident(f) }
end
