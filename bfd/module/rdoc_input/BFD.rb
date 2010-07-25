#!/usr/bin/env ruby1.9
# :title: BFD
=begin rdoc
=BFD
<i>Copyright 2010 Thoughtgang <http://www.thoughtgang.org></i>

descr

require 'BFD'
t = Bfd::Target.new('/tmp/a.out', {})
=end


module Bfd
  class Symbol
    attr_reader :name, :type, :value, :flags, :section, :binding
    DYNAMIC="dynamic"
    STATIC="static"
  end

  class Section
    attr_reader :id, :name, :index, :flags, :vma, :lma, :size, :raw_size,
                :alignment_power, :file_pos, :contents, :symbol
  end

  class Target
    attr_reader :id, :filename, :format, :format_flags, :start_address,
                :arch_info, :flavour, :type, :type_flags, :endian,
                :sections, :symbols
    # arch info:
    # :bits_per_word
    # :bits_per_address
    # :bits_per_byte
    # :architecture
    # :section_align_power

    def initialize(target, args)
    end
end
