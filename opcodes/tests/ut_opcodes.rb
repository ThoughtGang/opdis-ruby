#!/usr/bin/env ruby
# Copyright 2010 Thoughtgang <http://www.thoughtgang.org>
# Unit test for Opcodes module

require 'test/unit'
require 'rubygems'
require 'Opcodes'

def hex_buf( arr )
  arr.collect{ |i| i.hex }.pack('C*')
end

class TC_BfdModule < Test::Unit::TestCase

  def test_nop
    Opcodes::Disassembler.new do |dis|
      buf = hex_buf( %w{ 90 90 90 } )
      ops = dis.disasm( buf )
      assert_equal( 3, ops.length )

      insn = dis.disasm_insn( buf )
      assert_equal( 'nop', insn.mnemonic )
    end
  end

  def test_int3
    Opcodes::Disassembler.new do |dis|
      ops = dis.disasm( hex_buf( %w{ CC CC CC } ) )
      assert_equal( 3, ops.length )
    end
  end

end

