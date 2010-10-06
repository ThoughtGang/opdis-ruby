#!/usr/bin/env ruby
# Copyright 2010 Thoughtgang <http://www.thoughtgang.org>
# Linear disassembly of a section in a BFD object file

require 'Opdis'

def disasm_file( file, arch, offset, length )
  if not length
    file.seek( 0, IO::SEEK_END )
    length = file.tell - offset
    file.rewind
  end

  Opdis::Disassembler.new( :arch => arch ) do |dis|
    opts = { :vma => offset, :buffer_vma => 0, :length=> length }
    dis.disassemble( file, opts ) do |i|
      puts "%08X\t%s" % [i.vma, i.to_s]
    end
  end
end

if __FILE__ == $0
  raise "Usage: #{$0} FILE ARCH [OFFSET] [LENGTH]" if ARGV.length == 0

  filename = ARGV.shift
  arch = ARGV.shift
  offset = ARGV.length > 0 ? ARGV.shift.to_i : 0
  length = ARGV.length > 0 ? ARGV.shift.to_i : nil

  File.open(filename, 'rb') { |f| disasm_file( f, arch, offset, length ) }
end
