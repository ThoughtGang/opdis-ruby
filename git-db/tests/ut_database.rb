#!/usr/bin/env ruby
# Copyright 2010 Thoughtgang <http://www.thoughtgang.org>
# Unit test for Git-DB Database class

require 'test/unit'
require 'fileutils'

require 'git-db/database'

class TC_GitDatabaseTest < Test::Unit::TestCase
  TMP = File.dirname(__FILE__) + File::SEPARATOR + 'tmp'

  attr_reader :db

  def setup
    FileUtils.remove_dir(TMP) if File.exist?(TMP)
    Dir.mkdir(TMP)

    path = TMP + File::SEPARATOR + 'db_test'
    @db = GitDB::Database.new(path)
  end

  def teardown
    FileUtils.remove_dir(TMP) if File.exist?(TMP)
  end

  def test_connect
    path = TMP + File::SEPARATOR + 'test-db-conn'

    # no-create should refuse to create the database
    GitDB::Database.connect(path, false)
    assert( (not File.exist?(path + File::SEPARATOR + '.git')), 
            "Repo #{path} incorrectly created by connect" ) 

    # ...otherwise connect should create the db
    db = GitDB::Database.connect(path)
    assert( File.exist?(path + File::SEPARATOR + '.git'), 
            "Repo #{path} not created by connect" ) 

    # verify that closing the DB works.
    db.close

    # verify that a closed database cannot be acted on
    # NOTE: this should verify ever
    assert_raise( GitDB::InvalidDbError ) { db.exec }
    assert_raise( GitDB::InvalidDbError ) { db.transaction }
    assert_raise( GitDB::InvalidDbError ) { db.delete }
    assert_raise( GitDB::InvalidDbError ) { db.close }
  end

  def test_create_delete
    path = TMP + File::SEPARATOR + 'test-db'

    # test that ctor creates a database by default
    GitDB::Database.new(path)
    assert( File.exist?(path + File::SEPARATOR + '.git'), 
            "Repo #{path} not created via new" )

    # test that connecting to an existing database works
    db = GitDB::Database.connect(path, false)
    assert_not_nil( db, 'Connect to existing DB failed' )

    # test that deleting a database works
    db.delete
    assert( (not File.exist?(path + File::SEPARATOR + '.git')),
            "Repo #{path} did not get deleted" )
  end

  def test_current_index
    assert_nil(@db.current_index, 'db.current_index not nil by default')

    index = GitDB::StageIndex.new(@db)
    @db.current_index = index
    assert_equal(index, @db.current_index, 'db.current_index= method failed')

    @db.current_index = nil
    assert_nil(@db.current_index, 'db.current_index=nil method failed')
  end

  def test_exec
    # test with no index
    fname, data = 'a_test_file', '123456'
    @db.exec { |idx| idx.add(fname, data)  }
    assert_nil(@db.current_index, 'exec created current index!')

    # Verify that the exec worked
    index = GitDB::StageIndex.new(@db)
    blob = index.current_tree./(fname)
    assert_not_nil(blob, "db.exec did not create '#{fname}' in staging")
    assert_equal(data, blob.data, "BLOB data for '#{fname}' does not match")

    # test exec with current_index set
    @db.current_index = index
    @db.exec { |idx| idx.delete(fname) }
    blob = index.current_tree./(fname)
    assert_equal(index, @db.current_index, 'db.exec clobbered current_index')
    @db.current_index.write
    # TODO: why does this fail? 
    #assert_nil(blob, "db.exec did not delete '#{fname}' in staging")

    # test nested exec
    name1, data1 = 'test1', '!!##$$@@^^**&&%%'
    name2, data2 = 'test2', '_-_-_-'
    @db.exec { |idx|
      idx.add(name1, data1)
      @db.exec { |idx| idx.add(name2, data2) }
    }
    assert_equal(index, @db.current_index, 
                 'nested db.exec clobbered current_index')
    @db.current_index.write

    # verify that both files in nested exec were created
    blob = index.current_tree./(name1)
    assert_not_nil(blob, "nested db.exec did not create '#{name1}' in staging")
    assert_equal(data1, blob.data, "BLOB data for '#{name1}' does not match")

    blob = index.current_tree./(name2)
    assert_not_nil(blob, "nested db.exec did not create '#{name2}' in staging")
    assert_equal(data2, blob.data, "BLOB data for '#{name2}' does not match")

    # cleanup
    index.delete(name1)
    index.delete(name2)
    index.write
    @db.current_index = nil
  end

  def test_transaction
    # test with no index
    # test with current index
    # test nested transactions
  end

end

