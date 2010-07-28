#!/usr/bin/env ruby1.9
# :title: Opdis::Callbacks
=begin rdoc
=Opdis Callbacks
<i>Copyright 2010 Thoughtgang <http://www.thoughtgang.org></i>

= Opdis Callback Routines

== Summary

== Example

== Contact
Support:: community@thoughtgang.org
Project:: http://rubyforge.org/projects/opdis/ 
=end

module Opdis

  # Hash members:
  # :vma
  # :offset
  # :size
  # :buffer
  # :items
  # :raw_insn
  # :branch_delay
  # :data_size
  # :type
  # :target
  # :target2

=begin rdoc
=end
  class InstructionDecoder

=begin rdoc
=end
    def decode( insn, hash )
      #invoke opdis generic instruction decoder
    end

  end

=begin rdoc
=end
  class X86Decoder < InstructionDecoder

=begin rdoc
=end
    def decode( insn, hash )
      # invoke opdis default x86 decoder with AT&T syntax
    end

  end

=begin rdoc
=end
  class X86IntelDecoder < InstructionDecoder

=begin rdoc
=end
    def decode( insn, hash )
      # invoke opdis default x86 decoder with Intel syntax
    end

  end

=begin rdoc
=end
  class VisitedAddressTracker

    # invoke opdis default handler
    # uses opdis tree to store addresses
=begin rdoc
=end
    def visited?
    end

  end

=begin rdoc
=end
  class AddressResolver

=begin rdoc
=end
    def resolve( insn )
      # invoke opdis default resolver 
    end

  end
end
