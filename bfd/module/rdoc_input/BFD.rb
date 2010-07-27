#!/usr/bin/env ruby1.9
# :title: BFD
=begin rdoc
=BFD
<i>Copyright 2010 Thoughtgang <http://www.thoughtgang.org></i>

= Gnu Binutils Binary File Descriptor (BFD) support

Requires GNU binutils, available at
http://www.gnu.org/software/binutils .

Documentation for GNU BFD is available at
http://sourceware.org/binutils/docs-2.20/bfd/index.html .

All of the original BFD data structures and constants can be found in
<b>bfd.h</b>, available on Linux installations at <b>/usr/include/bfd.h</b>.

== Summary
A wrapper for libbfd, distributed with GNU binutils.

== Example
require 'BFD'

t = Bfd::Target.new('/tmp/a.out', {})


puts t.arch_info.architecture # Display architecture info


t.sections.each{ |name,sec| puts "%s at 0x%X" % [name, sec.vma] } 
# Display list of sections


t.symbols.each{ |name,sym| puts "%s : 0x%X" % [name, sym.value] }
# Display list of symbols

== Disclaimer
This is a minimal implementation of BFD, intended only for use as a source
of input to classes in the Opdis gem. It should not be expected to support
the entire functionality of the GNU BFD library. In particular, changes made
to the Ruby BFD objects will not be propagated to on-disk BFD objects.

== Contact
Support:: community@thoughtgang.org
Project:: http://rubyforge.org/projects/opdis/
=end


=begin rdoc
GNU Binary File Description (BFD) support.
=end
module Bfd

=begin rdoc
A symbol (usually a named address) in a BFD object. 
Source: <b>typedef struct bfd_symbol</b>.
=end

  class Symbol

=begin rdoc
Name of the symbol. 
Source: <b>symbol_info.name</b>.
=end
    attr_reader :name

=begin rdoc
Symbol type. 
Source: <b>symbol_info.type</b>
=end
    attr_reader :type

=begin rdoc
Value of symbol. 
Source: <b>symbol_info.value</b>
=end
    attr_reader :value

=begin rdoc
Bit-flags from bfd_symbol type definition. 
Source: <b>bfd_symbol.raw_flags</b>
=end
    attr_reader :raw_flags

=begin rdoc
Name of section containing symbol. 
Source: <b>bfd_symbol.section.name</b>
=end
    attr_reader :section

=begin rdoc
'static' or 'dynamic'. 
Source: Table containing symbol.
=end
    attr_reader :binding

    # Dynamic binding
    DYNAMIC="dynamic"
    # Static binding
    STATIC="static"

  end

=begin rdoc
A section (usually a container for code, data, or metadata) in a BFD object.
Source: <b>typedef struct bfd_section</b>.
=end
  class Section

=begin rdoc
ID of section.
Source: <b>bfd_section.id</b>
=end
    attr_reader :id

=begin rdoc
Name of section.
Source: <b>bfd_section.name</b>
=end
    attr_reader :name

=begin rdoc
Index of section.
Source: <b>bfd_section.index</b>
=end
    attr_reader :index

=begin rdoc
Section flags.
Source: <b>bfd_section.raw_flags</b>
=end
    attr_reader :raw_flags 

=begin rdoc
Virtual Memory (Load) address of section.
Source: <b>bfd_section.vma</b>
=end
    attr_reader :vma 

=begin rdoc
Address of section in ROM image.
Source: <b>bfd_section.lma</b>
=end
    attr_reader :lma

=begin rdoc
Size of section. 
Source: <b>bfd_section_size()</b>
=end
    attr_reader :size 

=begin rdoc
Alignment of section, as an exponent of 2 (e.g. 3 means 
aligned to 2^3 or 8 bytes).
Source: <b>bfd_section.alignment_power</b>
=end
    attr_reader :alignment_power 

=begin rdoc
Offset in file where section appears.
Source: <b>bfd_section.filepos</b>
=end
    attr_reader :file_pos

=begin rdoc
Binary (raw) contents of section.
Source: <b>bfd_section.contents</b>
=end
    attr_reader :contents

  end

=begin rdoc
A Binary File Descriptor for a target.
Source: <b>struct bfd</b>.
=end
  class Target

=begin rdoc
ID of target.
Source: <b>bfd.id</b>
=end
    attr_reader :id

=begin rdoc
Filename of target.
Source: <b>bfd.filename</b>
=end
    attr_reader :filename

=begin rdoc
Format (object, archive, core) of target.
Source: <b>bfd.format</b>
=end
    attr_reader :format

=begin rdoc
Flags for BFD format.
Source: <b>bfd.flags</b>
=end
    attr_reader :raw_format_flags

=begin rdoc
Entry point (first instruction) of BFD if an executable.
Source: <b>bfd.start_address</b>
=end
    attr_reader :start_address

=begin rdoc
Architecture information for target.
This is a hash containing the following values:
  * bits_per_word
  * bits_per_address
  * bits_per_byte
  * architecture
  * section_align_power
Source: <b>bfd.arch_info</b> (see <b>struct bfd_arch_info</b>)
=end
    attr_reader :arch_info

=begin rdoc
Flavour of BFD file, e.g. ELF, COFF, AOUT, etc.
Source: <b>bfd.flavour</b> 
=end
    attr_reader :flavour

=begin rdoc
Kind of target, e.g. elf64-x86-64.
Source: <b>bfd_target.name</b>
=end
    attr_reader :type

=begin rdoc
Flags for target type.
Source: <b>bfd_target.object_flags</b>
=end
    attr_reader :raw_type_flags

=begin rdoc
Endianness (big or little) of target.
Source: <b>bfd_target.byteorder</b>
=end
    attr_reader :endian

=begin rdoc
Hash of sections names to section objects.
Source: <b>bfd.sections</b>
=end
    attr_reader :sections

=begin rdoc
Hash of symbol names to symbol targets.
Includes both static and dynamic symbols.
Source: <b>bfd_canonicalize_symtab()</b> and 
<b>bfd_canonicalize_dynamic_symtab()</b>.
=end
    attr_reader :symbols

=begin rdoc
Create a new Bfd::Target object for <i>target</i>, which can be a String
containing a file path, or an IO object for an already-loaded file.
Currently no options are supported via args.
=end
    def initialize(target, args)
    end
end
