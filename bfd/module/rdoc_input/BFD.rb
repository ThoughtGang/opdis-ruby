#!/usr/bin/env ruby1.9
# :title: BFD
=begin rdoc
=BFD
<i>Copyright 2010 Thoughtgang <http://www.thoughtgang.org></i>

<b>Gnu Binutils Binary File Descriptor (BFD) support</b>
Requires binutils at...
Defintions are in bfd.h /usr/include/bfd.h

require 'BFD'
t = Bfd::Target.new('/tmp/a.out', {})
=end


=begin rdoc
GNU Binary File Description (BFD) support.
=end
module Bfd

=begin rdoc
A symbol (usually a named address) in a BFD object. 
Source: <b>typedef struct bfd_symbol</b>.

<i>name</i>: Name of the symbol. 
Source: <b>symbol_info.name</b>.

<i>type</i>: Symbol type. 
Source: <b>symbol_info.type</b>

<i>value</i>: Value of symbol. 
Source: <b>symbol_info.value</b>

<i>flags</i>: Bit-flags from bfd_symbol type definition. 
Source: <b>bfd_symbol.flags</b>

<i>section</i>: Name of section containing symbol. 
Source: <b>bfd_symbol.section.name</b>

<i>binding</i>: 'static' or 'dynamic'. 
Source: Table containing symbol.
=end
  class Symbol
    attr_reader :name, :type, :value, :flags, :section, :binding
    DYNAMIC="dynamic"
    STATIC="static"
  end

=begin rdoc
A section (usually a container for code, data, or metadata) in a BFD object.
Source: <b>typedef struct bfd_section</b>.

<i>id</i>: ID of section.
Source: <b>bfd_section.id</b>

<i>name</i>: Name of section.
Source: <b>bfd_section.name</b>

<i>index</i>: Index of section.
Source: <b>bfd_section.index</b>

<i>flags</i>: Section flags.
Source: <b>bfd_section.flags</b>

<i>vma</i>: Virtual Memory (Load) address of section.
Source: <b>bfd_section.vma</b>

<i>lma</i>: Address of section in ROM image.
Source: <b>bfd_section.lma</b>

<i>size</i>: Size of section when loaded. A section with no contents, e.g.
.bss, with still have a nonzero <i>size</i>.
Source: <b>bfd_section.size</b>

<i>raw_size</i>: Size of section on-disk. a section with no contents, e.g.
.bss, will have a zero <i>raw_size</i>.
Source: <b>bfd_section.rawsize</b>

<i>alignment_power</i>: Alignment of section, as an exponent of 2 (e.g. 3 means 
aligned to 2^3 or 8 bytes).
Source: <b>bfd_section.alignment_power</b>

<i>file_pos</i>: Offset in file where section appears.
Source: <b>bfd_section.filepos</b>

<i>contents</i>: Binary (raw) contents of section.
Source: <b>bfd_section.contents</b>

=end
  class Section
    attr_reader :id, :name, :index, :flags, :vma, :lma, :size, :raw_size,
                :alignment_power, :file_pos, :contents
  end

=begin rdoc
A Binary File Descriptor for a target.
Source: <b>struct bfd</b>.

<i>id</i>: ID of target.
Source: <b>bfd.id</b>

<i>filename</i>: Filename of target.
Source: <b>bfd.filename</b>

<i>format</i>: Format (object, archive, core) of target.
Source: <b>bfd.format</b>

<i>format_flags</i>: Flags for BFD format.
Source: <b>bfd.flags</b>

<i>start_address</i>: Entry point (first instruction) of BFD if an executable.
Source: <b>bfd.start_address</b>

<i>arch_info</i>: Architecture information for target.
This is a hash containing the following values:
  * bits_per_word
  * bits_per_address
  * bits_per_byte
  * architecture
  * section_align_power
Source: <b>bfd.arch_info</b> (see <b>struct bfd_arch_info</b>)

<i>flavour</i>: Flavour of BFD file, e.g. ELF, COFF, AOUT, etc.
Source: <b>bfd.flavour</b> 

<i>type</i>: Kind of target, e.g. elf64-x86-64.
Source: <b>bfd_target.name</b>

<i>type_flags</i>: Flags for target type.
Source: <b>bfd_target.object_flags</b>

<i>endian</i>: Endianness (big or little) of target.
Source: <b>bfd_target.byteorder</b>

<i>sections</i>: Hash of sections names to section objects.
Source: <b>bfd.sections</b>

<i>symbols</i>: Hash of symbol names to symbol targets.
Includes both static and dynamic symbols.
Source: <b>bfd_canonicalize_symtab()</b> and 
<b>bfd_canonicalize_dynamic_symtab()</b>.

=end
  class Target
    attr_reader :id, :filename, :format, :format_flags, :start_address,
                :arch_info, :flavour, :type, :type_flags, :endian,
                :sections, :symbols
=begin rdoc
Create a new Bfd::Target object for <i>target</i>, which can be a String
containing a file path, or an IO object for an already-loaded file.
Currently no options are supported via args.
=end
    def initialize(target, args)
    end
end
