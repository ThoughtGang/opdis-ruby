#!/usr/bin/env ruby
# Copyright 2010 Thoughtgang <http://www.thoughtgang.org>
# Unit test for Opdis module

require 'test/unit'
require 'rubygems'
require 'Opdis'

class TC_OpdisModule < Test::Unit::TestCase

  def hex_buf( arr )
    arr.collect{ |i| i.hex }.pack('C*')
  end

  def test_buffer_nop
    Opdis::Disassembler.new( :arch => 'x86' ) do |dis|
      ops = dis.disassemble( hex_buf(%w{ 90 90 90 }) )
      assert_equal( 3, ops.length )
      assert_equal( 'nop', ops[0].mnemonic )
    end
  end

  def test_buffer_int3
    Opdis::Disassembler.new( :arch => 'x86' ) do |dis|
      ops = dis.disassemble( hex_buf(%w{ CC CC CC }) )
      assert_equal( 3, ops.length )
      assert_equal( 'int3', ops[0].mnemonic )
    end
  end
end
