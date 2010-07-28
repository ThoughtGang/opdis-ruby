#!/usr/bin/env ruby1.9
# List BFD Sections
# Copyright 2010 Thoughtgang <http://www.thoughtgang.org>

require 'BFD'

def display_section(sec)
  puts "#{sec.id}: #{sec.name}"
  puts "\tFlags: #{sec.flags.join(',')}"
  puts "\tFile Pos: 0x%X VMA: 0x%X LMA: 0x%X" % [sec.file_pos, sec.vma, sec.lma]
  puts "\tSize: #{sec.size} Alignment: #{2**sec.alignment_power}"
end

def list_secs(filename)
  tgt = Bfd::Target.new(filename)
  puts "#{tgt.id}: #{tgt.filename}"
  tgt.sections.values.sort_by { |s| s.index }.each { |s| display_section s }
  puts
end

if __FILE__ == $0
  raise "Usage: #{$0} FILE [FILE...]" if ARGV.length == 0
  ARGV.each { |f| list_secs(f) }
end
