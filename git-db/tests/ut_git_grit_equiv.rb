#!/usr/bin/env ruby
# Copyright 2011 Thoughtgang <http://www.thoughtgang.org>
# Unit tests to ensure that assumptions about Grit behaving like Git still hold.
# Note: This is used to verify that Grit can be made to act like the Git
#       command line, and to determine what Grit methods correspond to 
#       Git commands.

require 'test/unit'
require 'fileutils'

require 'rubygems'
require 'grit'
require 'grit/git-ruby'

# NOTES:
#   `git read-tree --prefix=path #{sha}` will read SHA object into git at the
#   specified path. git-write-tree can then be used to create the tree object
#   for the entire index. NOTE: `export GIT_INDEX_FILE=/tmp/index` can be used
#   to specify an index that is NOT the staging index. IMPORTANT!
# git-update-index --force-remove 
# git-update-index --add --cacheinfo 100644 `echo '#{data}' | git-hash-object --stdin` #{path}
# echo '#{msg}' | git commit-tree #{sha}
# git-ls-files --stage #{path}
# git rev-parse --show-toplevel
# git rev-parse --show-prefix : path of curr dir from top_level. cdup = inverse.

# Grit:Git methods:
# object_exists? sha
# git_dir, work_tree
# fs_exist? path [on fs, in git dir]. fs_read|write|delete|move|mkdir|chmod
# commit_from_sha sha
# raw_git(command, index_filename) : note this is exec_in_git!
# get_object_by_sha1 sha
# object_exists? sha
# cat_file_type sha
# cat_file_size sha
# cat_file sha
# list_tree sha => ['blob'|'tree']['NAME'] = { :mode, :sha }
# ls_tree sha, paths, recursive = false -> cat-file output
# ls_tree_path(sha, path, append=nil) -> array of tree entries
# get_subtree(commit_sha, path) -> tree sha (or parent of, or /)
 
class TC_GitGritEquivalenceTest < Test::Unit::TestCase
  TMP = File.dirname(__FILE__) + File::SEPARATOR + 'tmp'
  GIT_REPO = 'git_repo'
  GRIT_REPO = 'grit_repo'

  attr_reader :repo # grit repo object

  def exec_in_dir(dir, &block)
    curr = Dir.getwd
    Dir.chdir dir 
    result = yield
    Dir.chdir curr
    result
  end

  def git_exec(&block)
    exec_in_dir(TMP + File::SEPARATOR + GIT_REPO, &block)
  end

  # this allows mimics Grit's use of anonymous (non-stage) indexes
  def git_index_exec(filename, &block)
    old_index = ENV[GIT_INDEX_FILE]
    ENV[GIT_INDEX_FILE] = TMP + File::SEPARATOR + filename
    rv = git_exec(&block)
    ENV[GIT_INDEX_FILE] = old_index
    rv
  end

  # Create a Git repo and a Grit repo in a temporary directory
  def setup
    FileUtils.remove_dir(TMP) if File.exist?(TMP)
    Dir.mkdir(TMP)

    # init git repo
    exec_in_dir(TMP) { `git init #{GIT_REPO}` }
    @git_path = TMP + File::SEPARATOR + GIT_REPO

    # init grit repo
    @grit_path = TMP + File::SEPARATOR + GRIT_REPO
    @repo = Grit::Repo.init(@grit_path)
  end

  # Delete all repos
  def teardown
    FileUtils.remove_dir(TMP) if File.exist?(TMP)
  end

  # Test that initializing a repository produces the same results
  def test_init
    sep = File::SEPARATOR
    assert( File.exist?(TMP + sep + GIT_REPO + sep + '.git'),
            'git repo was not created!')
    assert( File.exist?(TMP + sep + GRIT_REPO + sep + '.git'),
            'Grit repo was not created!')
  end

  # Test that generating an object SHA produces the same results
  def test_object_sha
    data = '01020304050607080900!@#$%^&*()_+='
    git_sha = git_exec { `echo -n '#{data}' | git hash-object --stdin` }.chomp
    grit_sha = Grit::GitRuby::Internal::LooseStorage.calculate_sha(data, 'blob')
    assert_equal( git_sha, grit_sha, 'Git and Grit SHA differ')
  end

  # Test that adding a Blob object produces the same results
  def test_add_blob_to_object_database
    data = '7654321098!@$#%^&*()_-=+'
    git_sha = git_exec {`echo -n '#{data}' | git hash-object -w --stdin`}.chomp
    blob_data = git_exec { `git cat-file -p #{git_sha}` }
    assert_equal(data, blob_data, 'Git cat-file does not match input')


    grit_sha = @repo.git.put_raw_object(data, 'blob')
    assert(@repo.git.object_exists?(grit_sha), 'Grit fails object_exists? call')
    assert_equal(data, @repo.git.ruby_git.cat_file(grit_sha),
                 'Grit cat_file does not match input')

    assert_equal( git_sha, grit_sha, 'Git and Grit SHA differ')

    sep = File::SEPARATOR
    path = sep + '.git' + sep + 'objects' + sep + git_sha[0,2] + sep + 
           git_sha[2..-1]
    assert(File.exist?(@git_path + path), 'Git did not add blob to object DB')
    assert(File.exist?(@grit_path + path), 'Grit did not add blob to object DB')
  end

  # Test that adding a Tree object produces the same results
  def test_add_path_to_object_database
    bdata = "123454321"
    bsha = git_exec {`echo -n '#{bdata}' | git hash-object -w --stdin`}.chomp
    grit_bsha = @repo.git.put_raw_object(bdata, 'blob')

    tdata1 = "100644 blob #{bsha}\ttest_blob"
    git_sha = git_exec {`echo -n '#{tdata1}' | git mktree`}.chomp

    # Note the different format needed for Grit!
    tdata1 = "100644 test_blob\0#{[bsha].pack('H*')}"
    grit_sha = @repo.git.put_raw_object(tdata1, 'tree')
    assert(@repo.git.object_exists?(grit_sha), 'Grit fails object_exists? call')
    assert_not_nil( @repo.tree(grit_sha), 'Grit did not create valid tree')

    assert_equal( git_sha, grit_sha, 'Git and Grit SHA differ')
  end

  # Test that adding a Blob to the (staging) index produces the same results
  def test_add_blob_to_index
    data = '....****....'
    fname = 'test_add_blob'

    # Add BLOB to Git object database
    git_sha = git_exec {`echo -n '#{data}' | git hash-object -w --stdin`}.chomp
    # Add BLOB to Git index
    git_exec {`git update-index --add --cacheinfo 100644 #{git_sha} #{fname}`}
    # Get SHA for filename in index
    mode, sha, junk = git_exec { `git ls-files -s #{fname}` }.split(/\s/)
    assert_equal(git_sha, sha, 'SHA in staging does not match input SHA')

    idx = @repo.index
    # Add BLOB to Grit index
    idx.add( fname, data)
    # Add BLOB to Grit object database and re-read
    idx.read_tree(idx.write_tree(idx.tree))
    # Get SHA for filename in index
    blob = idx.current_tree/fname
    assert_equal(data, blob.data, 'Grit index data does not match input data')

    assert_equal( git_sha, blob.id, 'Git and Grit SHA differ')
  end

  def test_add_path_to_index
  end

  def test_remove_file_from_object_database
  end

  def test_remove_path_from_object_database
  end

  def test_remove_file_from_index
  end

  def test_remove_path_from_index
  end

end

