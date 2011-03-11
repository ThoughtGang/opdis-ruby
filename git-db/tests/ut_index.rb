#!/usr/bin/env ruby
# Copyright 2011 Thoughtgang <http://www.thoughtgang.org>
# Unit test for Git-DB Index class

require 'test/unit'
require 'fileutils'

require 'git-db/repo'
require 'git-db/index'

# TODO: write
#       add_db
#       add_fs
#       add
#       add true
#       [add_fs_item]
#       [make_parent_dirs]
#       StageIndex
#         commit
#         write
#         [read_sha1]
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

  def test_index
    name, data = 'test-file-1', '..!!..!!..'

    idx = @repo.index
    #idx.add(name, data)
    #idx.write
  end

  def test_staging
    file, data = 'misc/stuff/stage-test-1', 'ststststst'
    idx = GitDB::StageIndex.new(@repo)
    idx.add(file, data)
    idx.write
    @repo.exec_in_git_dir {
      puts "git-ls-files --stage:"
      puts `git ls-files --stage`
    }
    # puts @repo.list_tree('misc').inspect
    # puts @repo.list_files('misc/stuff').inspect
  end

  def test_dummy
    true
  end
end

