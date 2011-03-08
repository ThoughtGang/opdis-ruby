#!/usr/bin/env ruby
# Copyright 2011 Thoughtgang <http://www.thoughtgang.org>
# Unit test for Git-DB Index class

require 'test/unit'
require 'fileutils'

require 'git-db/repo'
require 'git-db/index'

class TC_GitIndexTest < Test::Unit::TestCase
  TMP = File.dirname(__FILE__) + File::SEPARATOR + 'tmp'

  attr_reader :repo

  def setup
    FileUtils.remove_dir(TMP) if File.exist?(TMP)
    Dir.mkdir(TMP)
    path = TMP + File::SEPARATOR + 'index_test'
    @repo = GitDB::Repo.create(path)
  end

  def teardown
    FileUtils.remove_dir(TMP) if File.exist?(TMP)
  end

  def test_dummy
    true
  end
end

