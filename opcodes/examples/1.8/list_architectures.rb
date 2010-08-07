#!/usr/bin/env ruby1.8
# List available architectures
# Copyright 2010 Thoughtgang <http://www.thoughtgang.org>

require 'Opcodes'

if __FILE__ == $0
  puts "Supported architectures:"
  Opcodes::Disassembler.architectures.each { |a| puts "\t#{a}" }
end
