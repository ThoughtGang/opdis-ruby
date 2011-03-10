#!/usr/bin/env ruby
# :title: Git-DB::Repo
=begin rdoc
Grit wrappers.

Copyright 2010 Thoughtgang <http://www.thoughtgang.org>
=end

require 'rubygems'
require 'grit'

require 'git-db/index'
require 'git-db/shared'

# TODO: locking ? config or something.
# TODO: sort out tags/branch
# TODO: When changing branch (head), be sure to change staging index.

module GitDB

=begin rdoc
A Git repository.

Note: StagingIndex is cached, as it is from the command line.
=end
  class Repo < Grit::Repo
    GIT_DIR = ::File::SEPARATOR + '.git'

    DEFAULT_TAG = '0.0.0'
    attr_reader :last_branch_tag

    DEFAULT_BRANCH='master'
    attr_reader :current_branch

    attr_reader :path

    # TODO: something more intelligent
    def self.create(path)
      `git init #{path}`
      new(path)
    end

=begin rdoc
Initialize a repo from the .git subdir in the given path.
=end
    def initialize(path)
      path.chomp(GIT_DIR) if path.end_with? GIT_DIR
      @path = (path.empty?) ? '.' : path

      # TODO: get last branch tag from repo
      #       prob as a git-config
      @last_branch_tag = DEFAULT_TAG
      @current_branch = DEFAULT_BRANCH

      super(@path + GIT_DIR)
    end

=begin rdoc
Return the top-level directory of the repo (the parent of .git).
=end
    def top_level
      git.git_dir.chomp(GIT_DIR)
    end

=begin rdoc
Return true if path exists in repo (on fs or in-tree)
    def include?( path )
      fetch_blob(path) ? true: false
    end
=end

# ----------------------------------------------------------------------
=begin rdoc
Return a cleaned-up version of the tag name, suitable for use as a filename.
Replaces all non-alphanumeric characters (except "-.,") with "_". 
=end
    def clean_tag(name)
      name.gsub( /[^-.,_[:alnum:]]/, '_' )
    end

=begin rdoc
Returns the next value for a tag. This is primarily used to auto-generate
tag names, e.g. 1.0.1, 1.0.2, etc.
=end
    def next_branch_tag
      @last_branch_tag.succ!
    end

=begin rdoc
Return the Head object for the specified branch
=end
    def branch(tag=@current_branch)
      get_head(tag)
    end

=begin rdoc
Creates a branch in refs/heads and associates it with the specified commit.
If sha is nil, the latest commit from 'master' is used.
The canonical name of the tag is returned.
=end
    def create_branch( tag=next_branch_tag(), sha=nil )
      sha = branches.first.commit.id if not sha
      name = clean_tag(name)
      update_ref(name, sha)
      name
    end

=begin rdoc
Sets the current branch to the specified tag. This changes the default
branch for all repo activity and sets HEAD.
=end
    def set_branch( tag, actor=BGO_ACTOR )
      # allow creating of new branches
      opt = (is_head? tag) ? '' : '-b'
      exec_git_cmd( "git checkout -q -m #{opt} '#{tag}'", actor )
      @current_branch = tag
    end

=begin rdoc
Merge specified branch into master.
=end
    def merge_branch( tag=@current_branch, actor=BGO_ACTOR )
      raise "Invalid branch '#{tag}'" if not (is_head? tag)

      tag.gsub!(/['\\]/, '')
      # switch to master branch
      exec_git_cmd( "git checkout -q -m '#{DEFAULT_BRANCH}'", actor )
      # merge current branch to master branch
      exec_git_cmd( "git merge --no-ff --no-log --no-squash '#{tag}'", actor )

      @current_branch = DEFAULT_BRANCH
    end

=begin rdoc
Tag (name) an object, e.g. a commit.
=end
    def tag_object(tag, sha)
      git.fs_write("refs/tags/#{clean_tag(tag)}", sha)
    end

# ----------------------------------------------------------------------
=begin rdoc
Return an empty git index for the repo.
=end
    def index
      Index.new(self)
    end

=begin rdoc
Return the staging index for the repo.
=end
    def staging
      @staging_index ||= StageIndex.new(self)
    end

    # NOTE: stage_sha1 replaced by self.stage.sha
    # NOTE: stage_tree replaced by self.tree(stage.sha). Could be replaced
    #       by a private method.

=begin rdoc 
Yield staging index to the provided block, then write the index when the
block returns.
This allows the Git staging index to be modified from within Ruby, with all
changes being visible to the Git command-line tools.
Returns staging index for chaining purposes.
=end
    def stage(&block)
      idx = self.staging
      yield idx
      idx.write
      idx
    end

    # NOTE: stage_commit replaced by stage_and_commit
=begin rdoc
Read the Git staging index, then commit it with the provided message and
author info.
Returns SHA of commit.
=end
    def stage_and_commit(msg, actor=nil, head=@current_branch, &block)
      idx = stage(&block)
      parent = commits(head, 1)
      last_tree = parent.count > 0 ? parent.first.tree.id : nil
      idx.commit(msg, parent, actor, last_tree, head)
    end

# ----------------------------------------------------------------------
=begin rdoc
Change to the Repo#top_level dir, yield to block, then pop the dir stack.
=end
    def exec_in_git_dir(&block)
      curr = Dir.getwd
      Dir.chdir top_level
      result = yield
      Dir.chdir curr
      result
    end

=begin rdoc
Execute the specified command using Repo#exec_in_git_dir.
=end
    def exec_git_cmd( cmd, actor=nil )
      old_aname = ENV['GIT_AUTHOR_NAME']
      old_aemail = ENV['GIT_AUTHOR_EMAIL']
      old_cname = ENV['GIT_COMMITTER_NAME']
      old_cemail = ENV['GIT_COMMITTER_EMAIL']
      old_pager = ENV['GIT_PAGER']

      if actor
        ENV['GIT_AUTHOR_NAME'] = actor.name
        ENV['GIT_AUTHOR_EMAIL'] = actor.email
        ENV['GIT_COMMITTER_NAME'] = actor.name
        ENV['GIT_COMMITTER_EMAIL'] = actor.email
      end
      ENV['GIT_PAGER'] = ''

      # Note: we cannot use Grit#raw_git_call as it requires an index file
      exec_in_git_dir { `#{cmd}` }

      ENV['GIT_AUTHOR_NAME'] = old_aname
      ENV['GIT_AUTHOR_EMAIL'] = old_aemail
      ENV['GIT_COMMITTER_NAME'] = old_cname
      ENV['GIT_COMMITTER_EMAIL'] = old_cemail
      ENV['GIT_PAGER'] = old_pager
    end

# ----------------------------------------------------------------------
# TODO: needed? Should use models.
    alias :add_files :add

    # NOTE: add_fs_object replaced by add
    # NOTE: add_db_object replaced by add
=begin rdoc
Add a DB entry at the filesystem path 'path' with contents 'data'. If
'on_fs' is true, the file is created in the filesystem as well.
This uses the staging index.
=end
    def add(path, data='', on_fs=false)
      self.stage { |idx| idx.add(path, data, on_fs) }
    end

    # NOTE: fetch_object replaced by object_data
=begin rdoc
Fetch the contents of a DB or FS object from the object database. This uses
the staging index.
=end
    def object_data(path)
      # TODO: Implement more effectively
      blob = object_blob(path)
      blob ? blob.data : nil
    end

# ----------------------------------------------------------------------
=begin rdoc
Return a 2-D hash of the tree:
['blob']['FILENAME'] = {:mode => '100644', :sha => SHA}
['tree']['DIRNAME'] = {:mode => '040000', :sha => SHA}
...etc.
Top-level keys are 'commit', 'blob', 'link', 'tree'.
=end
    def list_tree(path, head=@current_branch)
      sha = tree_sha1(path, head)
      sha ? git.ruby_git.list_tree( sha ) : {}
    end

=begin rdoc
Return an array of the trees that are subtrees of 'path'. This is generally
used to list the instances of an object type, e.g.
    list_subtrees( 'file' )
    list_subtrees( 'file/a.out/sections' )
=end
    def list_subtrees(path, head=@current_branch)
      contents = list_tree(path, head)
      (contents and contents.length > 0 and contents['tree']) ? \
                                        contents['tree'].keys : []
    end

=begin rdoc
Return an array of the files that are in the directory 'path'. This is 
generally used to list the contents of a Hash, e.g.
    list_files( 'function/8040100/names' )
=end
    def list_files(path, head=@current_branch)
      contents = list_tree(path, head)
      (contents and contents.length > 0 and contents['blob']) ? \
                                        contents['blob'].keys : []
    end

=begin rdoc
Get contents of tree, recursively.
=end
    def tree_contents(path, head=@current_branch)
      sha = tree_sha1(path, head)
      sha ? git.ruby_git.get_raw_tree( sha ) : {}
    end

# ----------------------------------------------------------------------
    private

    # NOTE: fetch_blob replaced by object_blob
=begin rdoc
Fetch a blob from the staging index based on an item path.
=end
# was fetch_blob
    def object_blob(path)
      tree = self.tree(stage.sha)
      tree ? (tree./path) : nil
    end

=begin rdoc
Get SHA1 for path.
=end
    def tree_sha1(path, head=@current_branch)
      # try staging index first
      dir = staging.current_tree./path

      # try repo if this fails (is this necessary?)
      dir = tree(head, [path]) if (not dir.kind_of? Grit::Tree) ||
                                  dir.contents.empty?

      dir.contents.length > 0 ? dir.contents.first.id : nil
    end

  end

end
