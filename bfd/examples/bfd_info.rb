#!/usr/bin/env ruby
# Show BFD Info
# Copyright 2010 Thoughtgang <http://www.thoughtgang.org>

require 'BFD'

def display_info(filename)
  Bfd::Target.new(filename) do |tgt|
    puts "#{tgt.id}: #{tgt.filename}"

    puts "#{tgt.flavour} #{tgt.format}: #{tgt.format_flags.join(',')}"

    puts "#{tgt.type}: #{tgt.type_flags.join(',')}"

    puts "#{tgt.sections.length} Sections #{tgt.symbols.length} Symbols"

    puts "Info:"
    info = tgt.arch_info
    info[:endian] = tgt.endian
    info[:entry] = "0x%X" % (tgt.start_address) if tgt.start_address
    info.keys.sort.each {|k| puts "\t#{k}: #{info[k]}" } 

    puts
  end
end

if __FILE__ == $0
  raise "Usage: #{$0} FILE [FILE...]" if ARGV.length == 0
  ARGV.each { |f| display_info(f) }
end
