#!/usr/bin/env ruby
# :title: Git-DB
=begin rdoc

Copyright 2010 Thoughtgang <http://www.thoughtgang.org>
=end

require 'git-db/shared'
require 'git-db/repo'

module GitDB

=begin rdoc
Exception raised when a non-String datatype is passed to set_property
=end
  class InvalidDataError < RuntimeError
  end

=begin rdoc
Instance methods used by repo-backed objects
=end
    module ModelItem

=begin rdoc
Git Repo for object.
=end
      attr_reader :repo

=begin rdoc
Get a property from the Repo. Return default if property does not exist.
Note: Uses the staging index. 
=end
      def get_property(path, default=nil)
        val = self.repo.fetch_object(self.property_path(path))
        val ? val.chomp : default
      end


=begin rdoc
Return the path to named property relative to top level of repo. The default
is to assume 'name' is a top-level object; classes should override this
to include their base dir and object ident.
=end
      def property_path(name)
        name
      end

    end

    module FsModelItem
      include ModelItem

=begin rdoc
Set a property in the repo, creating a file on the filesystem.
Note: Uses the staging index. 
=end
      def set_property(path, data)
        raise InvalidDataError.new if not (data.kind_of? String)
        self.repo.add_fs_object(property_path(path), data + "\n")
      end
    end

    module DbModelItem
      include ModelItem

=begin rdoc
Set a property in the repo. Do not create a file on the filesystem.
Note: Uses the staging index. 
=end
      def set_property(path, data)
        raise InvalidDataError.new if not (data.kind_of? String)
        self.repo.add_db_object(property_path(path), data + "\n")
      end
    end
end
