#!/usr/bin/env ruby
# Copyright 2010 Thoughtgang <http://www.thoughtgang.org>
# Unit test for Git-DB Repo class

require 'test/unit'
require 'fileutils'

require 'git-db/repo'

class TC_GitRepoTest < Test::Unit::TestCase
  TMP = File.dirname(__FILE__) + File::SEPARATOR + 'tmp'

  attr_reader :repo

  def setup
    FileUtils.remove_dir(TMP) if File.exist?(TMP)
    Dir.mkdir(TMP)
  end

  def teardown
    FileUtils.remove_dir(TMP) if File.exist?(TMP)
  end

  def test_create
    path = TMP + File::SEPARATOR + 'test'
    @repo = GitDB::Repo.create(path)
    assert(File.exist?(path + File::SEPARATOR + '.git'), 
           "Repo #{path} not created!") 
  end
end

