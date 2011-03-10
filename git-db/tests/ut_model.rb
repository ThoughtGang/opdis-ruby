#!/usr/bin/env ruby
# Copyright 2011 Thoughtgang <http://www.thoughtgang.org>
# Unit test for Git-DB Model class

require 'test/unit'
require 'fileutils'

require 'git-db/database'
require 'git-db/model'

# =============================================================================
# Generic ModelItems for testing the modules themselves
class TestModelItem
  include GitDB::ModelItem
  extend GitDB::ModelItemClass
end

# ----------------------------------------------------------------------
class TestDbModelItem
  include GitDB::DbModelItem
  extend GitDB::DbModelItemClass

  NAME = 'db-test-item'
  PROP_NAME = 'property'
  PROP_DATA = 'zzzZZZzzz'

  def self.name
    NAME
  end

  def self.fill(db, item_path, args)
    prop = item_path + File::SEPARATOR + PROP_NAME
    db.add( prop, PROP_DATA)
  end
end

# ----------------------------------------------------------------------
class TestFsModelItem
  include GitDB::FsModelItem
  extend GitDB::FsModelItemClass

  NAME = 'fs-test-item'
  PROP_NAME = 'property'
  PROP_DATA = 'xxxXXXxxx'

  def self.name
    NAME
  end

  def self.fill(db, item_path, args)
    prop = item_path + File::SEPARATOR + PROP_NAME
    db.add( prop, PROP_DATA, true)
  end
end

# =============================================================================
# Test database model:
#   user/                          : FsModelItemClass
#   user/$NAME/                    : FsModelItem
#   user/$NAME/id                  : Property
#   user/$NAME/role/               : DbModelItemClass
#   user/$NAME/role/$NAME/         : DbModelItem
#   user/$NAME/role/$NAME/auth     : Property
#            
#   group/                         : FsModelItemClass
#   group/$NAME/                   : FsModelItem
#   group/$NAME/id                 : Property

# ----------------------------------------------------------------------
class UserRoleModelItem
  include GitDB::DbModelItem
  extend GitDB::DbModelItemClass
end

# ----------------------------------------------------------------------
class UserModelItem
  include GitDB::FsModelItem
  extend GitDB::FsModelItemClass
  NAME = 'user'

  def self.name
    NAME
  end

  def self.fill(db, item_path, args)
    puts 'USER'
    puts item_path
    puts args.inspect
  end
end

# ----------------------------------------------------------------------
class GroupModelItem
  include GitDB::FsModelItem
  extend GitDB::FsModelItemClass
  NAME = 'group'

  def self.name
    NAME
  end

  def self.fill(db, item_path, args)
    puts 'GROUP'
    puts item_path
    puts args.inspect
  end
end


# =============================================================================
class TC_GitModelTest < Test::Unit::TestCase
  TMP = File.dirname(__FILE__) + File::SEPARATOR + 'tmp'

  attr_reader :repo

  def setup
    FileUtils.remove_dir(TMP) if File.exist?(TMP)
    Dir.mkdir(TMP)
    path = TMP + File::SEPARATOR + 'model_test'
    @db = GitDB::Database.new(path)
  end

  def teardown
    FileUtils.remove_dir(TMP) if File.exist?(TMP)
  end

  # ----------------------------------------------------------------------
  def test_model_item
    assert_raises(RuntimeError, 'Abstract class method called') {
      TestModelItem.name
    }
    assert_raises(RuntimeError, 'parent is not a ModelItem') {
      TestModelItem.create("root")
    }
    assert_raises(RuntimeError, 'Use Database.root instead of nil for parent') {
      TestModelItem.create("root")
    }
  end

  # ----------------------------------------------------------------------
  def test_db_model_item
    assert_equal(TestDbModelItem::NAME, TestDbModelItem.name,
                 'DbModelItem.name does not return class name')

    # create an in-DB ModelItem
    id = '101'
    data = '00110011'
    TestDbModelItem.create @db.root, {:ident => id, :data => data }
    # is item in DB?
    # is item NOT on FS?
    @db.exec_in_git_dir {
      assert( (not File.exist? TestDbModelItem.name + File::SEPARATOR + id),
             "TestDbModelItem did not create file on disk!")
    }
    # can item be listed?
    # does property contain data?
  end

  # ----------------------------------------------------------------------
  def test_fs_model_item
    assert_equal(TestFsModelItem::NAME, TestFsModelItem.name,
                 'FsModelItem.name does not return class name')

    # create an on-FS ModelItem
    id = '102'
    data = '11223344'
    TestFsModelItem.create @db.root, {:ident => id, :data => data }

    # is item in DB?
    # is item NOT on FS?
    @db.exec_in_git_dir {
      assert( (File.exist? TestFsModelItem.name + File::SEPARATOR + id),
              "TestFsModelItem did not create file on disk!")
    }
    # can item be listed?
    # does property contain data?
  end

  # ----------------------------------------------------------------------
  def test_user_create
    #UserModelItem.list
    #UserModelItem.create(@db.root, {})
    # list
    # add
    # list
  end

  def test_user_delete
    # list
    # add
    # list
    # delete
    # list
  end

  def test_user_role
    # create
    # read role
    # add role
    # read role
    # change role
    # read role
    # delete
  end

  # ----------------------------------------------------------------------
  def test_group_create
    # list
    # add
    # list
  end

  def test_group_delete
    # list
    # add
    # list
    # delete
    # list
  end

  def test_group_users
    # add
    # add users
    # add users to group [property is PATH]
    # list users
    # remove user from group
    # list users
    # delete users
    # delete
  end

end

