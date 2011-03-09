#!/usr/bin/env ruby
# :title: Git-DB::Index
=begin rdoc
Wrapper for Grit::Index

Copyright 2010 Thoughtgang <http://www.thoughtgang.org>
=end

require 'rubygems'
require 'grit'

require 'git-db/shared'

module GitDB

# =============================================================================
=begin rdoc
A Git Index.
=end
  class Index < Grit::Index

# TODO: obsolete
=begin rdoc
Read a tree into the index, yield the index to block, write the index to the
object DB, then read the object DB into the GIT staging index.

This is required in order to have the Grit index match the Git staging index.

Note: this is expensive, so as much work should be packed into the supplied 
block as possible.
    def modify_tree(treeish=@repo.current_branch)
      read_tree(treeish)  # read tree from Git staging
      yield self
      write               # write tree to Git staging
    end
=end

=begin rdoc
Write index to the object db, then read the object DB into the GIT staging
index. Returns SHA of new tree.
=end
    def write
      sha = write_tree(self.tree, self.current_tree)
      @repo.exec_in_git_dir { `git read-tree #{sha}` }
      sha
    end

=begin rdoc
Add a DB entry at the virtual path 'path' with contents 'contents'
=end
    alias :add_db :add

    def add(path, data, on_fs=false)
      super(path, data)
      add_fs_item(path, data) if on_fs
    end

    def add_fs(path, data)
      add(path, data, true)
    end

    private

=begin rdoc
Add a DB entry at the filesystem path 'path' with contents 'contents'
=end
    def add_fs_item( path, data )
      fs_path = @repo.top_level + ::File::SEPARATOR + path
      make_parent_dirs(fs_path)

      # Create file in filesystem
      @repo.exec_in_git_dir { ::File.open(fs_path, 'w') {|f| f.write(data)} }
    end

=begin rdoc
Add parent directories as-needed to create 'path' on the filesystem.
=end
    def make_parent_dirs(path)
      tmp_path = ''

      ::File.dirname(path).split(::File::SEPARATOR).each do |dir|
        next if dir.empty?
        tmp_path << ::File::SEPARATOR << dir
        Dir.mkdir(tmp_path) if not ::File.exist?(tmp_path)
      end
    end

  end

=begin rdoc
Index object for the Git staging index.
=end
  class StageIndex < Index

    attr_reader :sha

=begin rdoc
=end
    def initialize(repo)
      super
      read_sha1
      read_tree(@sha)
    end

=begin rdoc
=end
    def commit(msg, author)
      parent = repo.commits(repo.current_branch, 1)
      last_tree = parent.count > 0 ? parent.first.tree.id : nil
      super (msg, parent, author, last_tree, repo.current_branch) 
    end

=begin rdoc
=end
    def write
      @sha = super
      read_tree(@sha)
    end

    private

=begin rdoc
Reads the SHA for root tree of index on disk. 
Uses git write-tree.
=end
    def read_sha1
      @sha = @repo.exec_in_git_dir{`git write-tree`}.chomp
    end
  end


end

