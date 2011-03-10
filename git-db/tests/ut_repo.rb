#!/usr/bin/env ruby
# Copyright 2010 Thoughtgang <http://www.thoughtgang.org>
# Unit test for Git-DB Repo class

require 'test/unit'
require 'fileutils'

require 'git-db/repo'

# TODO:
# index
# staging
# stage
# stage_and_commit
# exec_in_git_dir
# exec_git_command
# object_data
# list_tree
# list_subtrees
# list_files
# tree_contents
# [object_blob]
# [tree_sha1]
# add
# add(true)
# add_files
# top_level
# - clean_tag
# - next_branch_tag
# - branch
# - create_branch
# - set_branch
# - merge_branch
# - tag_object
#

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

