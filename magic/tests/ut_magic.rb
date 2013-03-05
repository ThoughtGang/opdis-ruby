#!/usr/bin/env ruby
# Copyright 2010 Thoughtgang <http://www.thoughtgang.org>
# Unit test for Magic module

require 'test/unit'
require 'rubygems'
require 'Magic'

class TC_MagicIdent < Test::Unit::TestCase
  def test_buffer
    buf = '<html><head></head><body></body></html>'
    assert_equal( 'HTML document text', Magic.identify(buf).split("\n").first )
  end

  def test_unix_exec_file
    path = File::SEPARATOR + 'bin' + File::SEPARATOR + 'cat'
    return if not File.exist?(path)

    # verify that file is identified as an executable (we don't know what kind)
    ident = ''
    File.open(path, 'rb') do |f|
      ident = Magic::identify(f)
      assert( ident =~ /executable/ )
    end

    # compare against output of file(1) command
    cmd_ident = `file #{path}`.split(':')[1]
    cmd_ident = (cmd_ident) ? cmd_ident.strip : ''
    assert_equal( cmd_ident, ident)
  end

  def test_unix_data_file
    path = File::SEPARATOR + 'etc' + File::SEPARATOR + 'group'
    return if not File.exist?(path)

    File.open(path, 'rb') do |f|
      magic = Magic::identify(f)
      assert_equal( magic, 'ASCII text' ) 
    end
  end

end
