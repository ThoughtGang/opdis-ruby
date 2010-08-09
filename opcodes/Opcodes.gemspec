#!/usr/bin/env ruby1.9

require 'rubygems'
spec = Gem::Specification.new do |spec|
  spec.name = 'Opcodes'
  spec.version = '1.1.0'
  spec.summary = 'Ruby extension library providing an API to GNU libopcodes'
  spec.description = %{Libopcodes is the disassembler library used by
  GNU binutils. This extension provides access to libopcodes for Ruby
  IO and BFD objects.}

  spec.author = 'TG Community'
  spec.email = 'community@thoughtgang.org'
  spec.homepage = 'http://rubyforge.org/projects/opdis/'
  spec.rubyforge_project = 'opdis'

  spec.required_ruby_version = '>= 1.9.1'
  spec.requirements = [ 'GNU binutils library and headers' ]

  spec.files = Dir['module/*.c', 'module/*.h', 'module/Opcodes.rb',
                   'examples/*.rb', 'README']
  spec.extra_rdoc_files = Dir['module/rdoc_input/*.rb']
  spec.extensions = Dir['module/extconf.rb']
  spec.test_files = nil
end
