#!/usr/bin/env ruby
# :title: Git-DB::Transaction
=begin rdoc

Copyright 2010 Thoughtgang <http://www.thoughtgang.org>
=end

require 'git-db/shared'

module GitDB

  class TransactionRollback < RuntimeError
  end

=begin rdoc
    # NOTE: transaction is all or nothing BUT does not really nest.
    #       nested transactions are flattened.
    # transaction...
    #   Transaction.new
    #     message ''
    # begin...end, rollback (and log) on exceptions
    # otherwise block must exit false
=end
  class Transaction

=begin rdoc
The GitDB::Index on which the transaction operates.
=end
    attr_reader :index
=begin rdoc
The message to use for the commit at the end of the transaction.
If nil, a commit is not performed (this is the default).
=end
    attr_reader :commit_msg
=begin rdoc
The Git author for the commit performed at the end of the transaction.
See commit_msg.
=end
    attr_reader :commit_author
=begin rdoc
The body of the transaction. The transaction is considered successful if no
exceptions are raised.
To cancel a transaction, use the rollback method.
=end
    attr_reader :block
=begin rdoc
Is transaction nested (inside a parent)?
If true, a write and commit will not be performed, and exceptions will
be propagated to the parent.
=end
    attr_reader :nested

    def initialize(index, nested, msg=nil, &block)
      @index = index
      @nested = nested
      @block = block
      # Default to no commit
      @commit_msg = msg
      # Default to config[user.name] and config[user.email]
      @commit_author = nil
    end

=begin rdoc
Set a commit message for this transaction.
A commit is only performed if the commit message is not nil, and if the
block passed to perform is successful.
=end
    def message(str)
      @commit_msg = str
    end

=begin rdoc
Set the Git Author info for the transaction commit. By default, this information
is pulled from the Git config file.
=end
    def author(name, email)
      @commit_author = Grit::Actor.new(name, email)
    end

=begin rdoc
A transaction is considered successful if the transaction block returns true.
=end
    def perform
      rv = true
      begin
        @block.call(self, @index)
      rescue Exception => e
        throw e if @nested
        rv = false
      end

      if rv and (not @nested)
        @index.write
        @index.commit(@commit_msg, @commit_author) if @commit_msg
      end

      rv
    end

=begin rdoc
Abort the transaction.
=end
    def rollback
      raise TransactionRollback.new
    end

  end
end
