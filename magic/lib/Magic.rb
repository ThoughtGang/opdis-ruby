#!/usr/bin/env ruby
# Copyright 2010 Thoughtgang <http://www.thoughtgang.org>
# Ruby additions to Magic module

# Load C extension wrapping libmagic.so
require 'MagicExt'

module Magic

=begin rdoc
Return the magic strings for the target.
Target: an IO, String of bytes, or Array of bytes.
Options:
  :show_all : Return all matches (default: on)
  :follow_symlinks : Follow symlinks (default: on)
  :allow_unprintable : Don't translate unprintable chars (default: on)
  :archive_contents : Check inside compressed files
  :device_contents : Look at the contents of devices
  :mime : Return the MIME type and encoding
  :apple :  Return the Apple creator and type
  :magic_file : Override the default magic.conf file
NOTE: libmagic has a bug that results in the file descriptor being passed to it
      being CLOSED after magic_descriptor() is called. Be advised that any
      IO object passed to Magic.identfy() WILL BE CLOSED by the backend 
      library.
=end

  def self.identify( target, options={} )
    ext_identify( target, options )
  end

end
