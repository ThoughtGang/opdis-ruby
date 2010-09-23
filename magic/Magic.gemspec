#!/usr/bin/env ruby1.9

require 'rubygems'
spec = Gem::Specification.new do |spec|
  spec.name = 'Magic'
  spec.version = '1.0.2'
  spec.summary = 'Ruby extension library providing an API to libmagic'
  spec.description = %{This extension provides access to the libmagic
  library, which is used (e.g. by the file(1) command) to identify the
  type of a binary or text file based on its initial bytes. The module
  offers a simple interface, with the bulk of the interface with libmagic
  being performed behind-the-scenes.}

  spec.author = 'TG Community'
  spec.email = 'community@thoughtgang.org'
  spec.homepage = 'http://rubyforge.org/projects/opdis/'
  spec.rubyforge_project = 'opdis'
  spec.licenses = [ "GPLv3" ]

  spec.required_ruby_version = '>= 1.9.1'
  spec.requirements = [ 'libmagic dev file (library and header)' ]

  spec.files = Dir['module/*.c', 'module/*.h', 'lib/Magic.rb', 
                   'examples/*.rb', 'README', 'ChangeLog', 'LICENSE',
                   'LICENSE.README']
  spec.extensions = Dir['module/extconf.rb']
  spec.test_files = nil
end
