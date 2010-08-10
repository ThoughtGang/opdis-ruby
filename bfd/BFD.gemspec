#!/usr/bin/env ruby1.9

require 'rubygems'
spec = Gem::Specification.new do |spec|
  spec.name = 'BFD'
  spec.version = '1.2.0'
  spec.summary = 'Ruby extension library providing an API to GNU BFD'
  spec.description = %{BFD is the Binary File Descriptor object used by
  GNU binutils. This extension provides a barebones BFD wrapper which 
  allows binary files to be accessed as BFD objects from within Ruby.}

  spec.author = 'TG Community'
  spec.email = 'community@thoughtgang.org'
  spec.homepage = 'http://rubyforge.org/projects/opdis/'
  spec.rubyforge_project = 'opdis'

  spec.required_ruby_version = '>= 1.9.1'
  spec.requirements = [ 'GNU binutils library and headers' ]

  spec.files = Dir['module/*.c', 'module/*.h', 'module/Bfd.rb', 'examples/*.rb']
  spec.extra_rdoc_files = Dir['module/rdoc_input/*.rb']
  spec.extensions = Dir['module/extconf.rb']
  spec.test_files = nil
end
