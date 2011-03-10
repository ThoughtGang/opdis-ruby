#!/usr/bin/env ruby
# :title: Git-DB::DB
=begin rdoc

Copyright 2011 Thoughtgang <http://www.thoughtgang.org>
=end

require 'fileutils'

require 'git-db/repo'
require 'git-db/model'
require 'git-db/shared'
require 'git-db/transaction'

# TODO: configuration
#       actor for connection
#       in transaction?
# transaction
#   stage { |idx|
#     @current_idx = idx
#     yield self
#   }

module GitDB

  class RootItem
    attr_reader :db

    def initialize(db)
      @db = db
    end

    def path
      ''
    end

    def children
      # TODO
    end

    def delete
      # nop
    end

    def self.name
      ''
    end

    def self.path
      ''
    end

    def self.create(parent, args)
      # nop
    end

    def self.list
      # nop
    end

  end

=begin rdoc
Exception raised when a closed database is accessed
=end
  class InvalidDbError < RuntimeError
  end

=begin rdoc
Actually DbConnection to the repository.

Note: all operations should be in exec or transaction blocks. These use a
persistent staging index, and are more efficient.
=end
  class Database < Repo

    # TODO: wrap get/set in mutex
    attr_reader :current_index
    attr_reader :stale
    attr_reader :root

=begin rdoc
Return a connection to the Git DB.
Creates the DB if it does not already exist.
=end
    def initialize(path)
      if not File.exist? path
        Repo.create(path)
      end

      @current_index = nil
      @stale = false
      @root = RootItem.new(self)
      super(path)
    end

=begin rdoc
=end
    def self.connect(path, create=true)
      return nil if (not create) && (not File.exist? path)
      new(path)
    end

=begin rdoc
Close DB connection, writing all changes to disk.

NOTE: This does not create a commit! Ony the staging index changes.
=end
    def close(save=true)
      raise InvalidDbError if @stale

      if save && @current_index
        @current_index.write
      end
      @current_index = nil
      @stale = true

      # TODO: remove all locks etc
    end

=begin rdoc
Delete Database (including entire repository) from disk.
=end
    def delete
      raise InvalidDbError if @stale

      close(false)
      FileUtils.remove_dir(@path) if ::File.exist?(@path)
    end

    def current_index
      # TODO: mutex
      @current_index
    end

    def current_index=(idx)
      # TODO: mutex
      @current_index = idx
    end

=begin rdoc
=end
    def exec(&block)
      raise InvalidDbError if @stale

      return exec_in_current_index(&block) if self.current_index

      self.current_index = self.staging
      exec_in_current_index(&block)
      self.current_index.write

      self.current_index = nil
    end

    def transaction(&block)
      raise InvalidDbError if @stale

      return transaction_in_current_index(true, &block) if self.current_index

      self.current_index = self.staging
      transaction_in_current_index(false, &block)
      self.current_index = nil
    end

=begin rdoc
=end
    def add(path, data='', on_fs=false)
      exec { |idx| idx.add(path, data, on_fs) }
    end

=begin rdoc
Wrapper for Grit::Repo#index that checks if Database has been closed.
=end
    def index
      raise InvalidDbError if @stale
      super
    end

=begin rdoc
Wrapper for Grit::Repo#head that checks if Database has been closed.
=end
    def head
      raise InvalidDbError if @stale
      super
    end

=begin rdoc
Wrapper for Grit::Repo#tree that checks if Database has been closed.
=end
    def tree(treeish = 'master', paths = [])
      raise InvalidDbError if @stale
      super
    end

    def branch_merge(&block)
      raise InvalidDbError if @stale
    # branch_merge...
    #   Branch.new
    #      name 'name'
    #      ...
    end


    private

=begin rdoc
Execute code block in context of current DB index
=end
    def exec_in_current_index(&block)
      yield self.current_index
    end

=begin rdoc
Perform transaction in context of current DB index
=end
    def transaction_in_current_index(nested, &block)
      t = Transaction.new(self.current_index, nested, &block)
      t.perform
    end

  end
end
