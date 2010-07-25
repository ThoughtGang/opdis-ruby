#!/usr/bin/env ruby1.9
# :title: BFD
=begin rdoc
=BFD
<i>Copyright 2010 Thoughtgang <http://www.thoughtgang.org></i>

=Gnu Binutils Binary File Descriptor (BFD) support

Requires GNU binutils, available at
http://www.gnu.org/software/binutils .

Documentation for GNU binutils is available at
http://sourceware.org/binutils/docs-2.20/bfd/index.html .

All of the original BFD data structures and constants can be found in
<b>bfd.h</b>, available on Linux installations in <b>/usr/include/bfd.h</b>.

=Basic Usage
require 'BFD'

t = Bfd::Target.new('/tmp/a.out', {})

# Display architecture info

puts t.arch_info.architecture

# Display list of sections

t.sections.each{ |name,sec| puts "%s at 0x%X" % [name, sec.vma] }

# Display list of symbols

t.symbols.each{ |name,sym| puts "%s : 0x%X" % [name, sym.value] }
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
<i>name</i>: Name of the symbol. 
Source: <b>symbol_info.name</b>.
=end
    attr_reader :name

=begin rdoc
<i>type</i>: Symbol type. 
Source: <b>symbol_info.type</b>
=end
    attr_reader :type

=begin rdoc
<i>value</i>: Value of symbol. 
Source: <b>symbol_info.value</b>
=end
    attr_reader :value

=begin rdoc
<i>flags</i>: Bit-flags from bfd_symbol type definition. 
Source: <b>bfd_symbol.flags</b>
=end
    attr_reader :flags

=begin rdoc
<i>section</i>: Name of section containing symbol. 
Source: <b>bfd_symbol.section.name</b>
=end
    attr_reader :section

=begin rdoc
<i>binding</i>: 'static' or 'dynamic'. 
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
Source: <b>bfd_section.flags</b>
=end
    attr_reader :flags 

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
Size of section when loaded. A section with no contents, e.g.
.bss, with still have a nonzero <i>size</i>.
Source: <b>bfd_section.size</b>
=end
    attr_reader :size 

=begin rdoc
Size of section on-disk. a section with no contents, e.g.
.bss, will have a zero <i>raw_size</i>.
Source: <b>bfd_section.rawsize</b>
=end
    attr_reader :raw_size

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
<i>id</i>: ID of target.
Source: <b>bfd.id</b>
=end
    attr_reader :id

=begin rdoc
<i>filename</i>: Filename of target.
Source: <b>bfd.filename</b>
=end
    attr_reader :filename

=begin rdoc
<i>format</i>: Format (object, archive, core) of target.
Source: <b>bfd.format</b>
=end
    attr_reader :format

=begin rdoc
<i>format_flags</i>: Flags for BFD format.
Source: <b>bfd.flags</b>
=end
    attr_reader :format_flags

=begin rdoc
<i>start_address</i>: Entry point (first instruction) of BFD if an executable.
Source: <b>bfd.start_address</b>
=end
    attr_reader :start_address

=begin rdoc
<i>arch_info</i>: Architecture information for target.
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
<i>flavour</i>: Flavour of BFD file, e.g. ELF, COFF, AOUT, etc.
Source: <b>bfd.flavour</b> 
=end
    attr_reader :flavour

=begin rdoc
<i>type</i>: Kind of target, e.g. elf64-x86-64.
Source: <b>bfd_target.name</b>
=end
    attr_reader :type

=begin rdoc
<i>type_flags</i>: Flags for target type.
Source: <b>bfd_target.object_flags</b>
=end
    attr_reader :type_flags

=begin rdoc
<i>endian</i>: Endianness (big or little) of target.
Source: <b>bfd_target.byteorder</b>
=end
    attr_reader :endian

=begin rdoc
<i>sections</i>: Hash of sections names to section objects.
Source: <b>bfd.sections</b>
=end
    attr_reader :sections

=begin rdoc
<i>symbols</i>: Hash of symbol names to symbol targets.
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
