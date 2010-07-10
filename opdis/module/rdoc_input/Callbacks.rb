#!/usr/bin/env ruby1.9
# :title: Opdis::Callbacks
=begin rdoc
=Opdis Callbacks
<i>Copyright 2010 Thoughtgang <http://www.thoughtgang.org></i>

descr
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

  class InstructionDecoder
    def decode( insn, hash )
      #invoke opdis generic instruction decoder
    end
  end

  class X86Decoder < InstructionDecoder
    def decode( insn, hash )
      # invoke opdis default x86 decoder with AT&T syntax
    end
  end

  class X86IntelDecoder < InstructionDecoder
    def decode( insn, hash )
      # invoke opdis default x86 decoder with Intel syntax
    end
  end

  class VisitedAddressTracker
    # invoke opdis default handler
    # uses opdis tree to store addresses
    def visited?
    end
  end

  class AddressResolver
    def resolve( insn )
      # invoke opdis default resolver 
    end
  end
end
