#!/usr/bin/env ruby1.9
# Copyright 2010 Thoughtgang <http://www.thoughtgang.org>
# Print available libopcodes options to STDOUT

require 'Opdis'

if __FILE__ == $0

  Opdis::Disassembler.options.each { |line| puts line }

end
