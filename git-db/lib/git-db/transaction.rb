#!/usr/bin/env ruby
# :title: Git-DB::Transaction
=begin rdoc

Copyright 2010 Thoughtgang <http://www.thoughtgang.org>
=end

require 'git-db/shared'

module GitDB

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
    attr_reader :index
    attr_reader :commit_msg
    attr_reader :commit_author
    attr_reader :block

    def initialize(index, msg=nil, &block)
      @index = index
      @message = msg
      @block = block
      # Default to author of last commit
      @commit_author = index.repo.commits.last.author
    end

=begin rdoc
Set a commit message for this transaction.
A commit is only performed if the commit message is not nil, and if the
block passed to perform is successful.
=end
    def message(str)
      @commit_msg = str
    end

    def actor(name, email)
      @commit_author = Grit::Actor.new(name, email)
    end

=begin rdoc
A transaction is considered successful if the transaction block returns true.
=end
    def perform
      rv = false
      begin
        rv = @block.call(@current_index)
      rescue Exception => e
        # nop. log?
      end

      if rv
        @current_index.write
        @current_index.commit(@commit_msg, @commit_author) if @commit_msg
      end

      rv
    end

  end
end
