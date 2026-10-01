OUTPUT_FORMAT("elf32-bigscore", "elf32-bigscore", "elf32-littlescore")
OUTPUT_ARCH(score)
ENTRY(_hardware_init)
SEARCH_DIR("/home/aladdin/working/Score/install/score-linux-elf/lib");
GROUP(-lc -lm -lgcc)
SECTIONS
{
  /* Exception vector hook area. */
  .exception 0xA00001FC :
  {
    KEEP(*(.exception_vec))
  } = 0
  /* Stack top. S+Core stack grows downward. */
  .stack 0xA07FFFFC :
  {
    _stack = .;
    KEEP(*(.stack))
  }
  /* Main image starts here. */
  . = 0xA0001000;
  .interp         : { *(.interp) }
  .hash           : { *(.hash) }
  .dynsym         : { *(.dynsym) }
  .dynstr         : { *(.dynstr) }
  .gnu.version    : { *(.gnu.version) }
  .gnu.version_d  : { *(.gnu.version_d) }
  .gnu.version_r  : { *(.gnu.version_r) }
  .rel.dyn :
  {
    *(.rel.init)
    *(.rel.text .rel.text.* .rel.gnu.linkonce.t.*)
    *(.rel.fini)
    *(.rel.rodata .rel.rodata.* .rel.gnu.linkonce.r.*)
    *(.rel.data .rel.data.* .rel.gnu.linkonce.d.*)
    *(.rel.tdata .rel.tdata.* .rel.gnu.linkonce.td.*)
    *(.rel.tbss .rel.tbss.* .rel.gnu.linkonce.tb.*)
    *(.rel.ctors)
    *(.rel.dtors)
    *(.rel.got)
    *(.rel.sdata .rel.sdata.* .rel.gnu.linkonce.s.*)
    *(.rel.sbss .rel.sbss.* .rel.gnu.linkonce.sb.*)
    *(.rel.sdata2 .rel.sdata2.* .rel.gnu.linkonce.s2.*)
    *(.rel.sbss2 .rel.sbss2.* .rel.gnu.linkonce.sb2.*)
    *(.rel.bss .rel.bss.* .rel.gnu.linkonce.b.*)
  }
  .rela.dyn :
  {
    *(.rela.init)
    *(.rela.text .rela.text.* .rela.gnu.linkonce.t.*)
    *(.rela.fini)
    *(.rela.rodata .rela.rodata.* .rela.gnu.linkonce.r.*)
    *(.rela.data .rela.data.* .rela.gnu.linkonce.d.*)
    *(.rela.tdata .rela.tdata.* .rela.gnu.linkonce.td.*)
    *(.rela.tbss .rela.tbss.* .rela.gnu.linkonce.tb.*)
    *(.rela.ctors)
    *(.rela.dtors)
    *(.rela.got)
    *(.rela.sdata .rela.sdata.* .rela.gnu.linkonce.s.*)
    *(.rela.sbss .rela.sbss.* .rela.gnu.linkonce.sb.*)
    *(.rela.sdata2 .rela.sdata2.* .rela.gnu.linkonce.s2.*)
    *(.rela.sbss2 .rela.sbss2.* .rela.gnu.linkonce.sb2.*)
    *(.rela.bss .rela.bss.* .rela.gnu.linkonce.b.*)
  }
  .rel.plt        : { *(.rel.plt) }
  .rela.plt       : { *(.rela.plt) }
  .hardware_init :
  {
    KEEP(*(.hardware_init))
  } = 0
  .init :
  {
    KEEP(*(.init))
  } = 0
  .plt : { *(.plt) }
  .text :
  {
    *(.text .stub .text.* .gnu.linkonce.t.*)
    *(.gnu.warning)
    *(.glue_7t)
    *(.glue_7)
  } = 0
  .fini :
  {
    KEEP(*(.fini))
  } = 0
  PROVIDE(__etext = .);
  PROVIDE(_etext = .);
  PROVIDE(etext = .);
  .rodata         : { *(.rodata .rodata.* .gnu.linkonce.r.*) }
  .rodata1        : { *(.rodata1) }
  .sdata2         : { *(.sdata2 .sdata2.* .gnu.linkonce.s2.*) }
  .sbss2          : { *(.sbss2 .sbss2.* .gnu.linkonce.sb2.*) }
  .eh_frame_hdr   : { *(.eh_frame_hdr) }
  . = ALIGN(256);
  . = ALIGN(4);
  PROVIDE(__preinit_array_start = .);
  .preinit_array : { KEEP(*(.preinit_array)) }
  PROVIDE(__preinit_array_end = .);
  PROVIDE(__init_array_start = .);
  .init_array : { KEEP(*(SORT(.init_array.*))) KEEP(*(.init_array)) }
  PROVIDE(__init_array_end = .);
  PROVIDE(__fini_array_start = .);
  .fini_array : { KEEP(*(SORT(.fini_array.*))) KEEP(*(.fini_array)) }
  PROVIDE(__fini_array_end = .);
  .data :
  {
    __data_start = .;
    *(.data .data.* .gnu.linkonce.d.*)
    SORT(CONSTRUCTORS)
  }
  .data1          : { *(.data1) }
  .tdata          : { *(.tdata .tdata.* .gnu.linkonce.td.*) }
  .tbss           : { *(.tbss .tbss.* .gnu.linkonce.tb.*) *(.tcommon) }
  .eh_frame       : { KEEP(*(.eh_frame)) }
  .gcc_except_table : { *(.gcc_except_table) }
  .dynamic        : { *(.dynamic) }
  .ctors :
  {
    KEEP(*crtbegin.o(.ctors))
    KEEP(*(EXCLUDE_FILE(*crtend.o) .ctors))
    KEEP(*(SORT(.ctors.*)))
    KEEP(*(.ctors))
  }
  .dtors :
  {
    KEEP(*crtbegin.o(.dtors))
    KEEP(*(EXCLUDE_FILE(*crtend.o) .dtors))
    KEEP(*(SORT(.dtors.*)))
    KEEP(*(.dtors))
  }
  .jcr : { KEEP(*(.jcr)) }
  .got : { *(.got.plt) *(.got) }
  /*
   * Small-data area.
   * _gp is deliberately defined once and placed 0x4000 bytes after the
   * beginning of the small-data window. This matches a signed +/-0x4000
   * GP-relative window assumption.
   */
  . = ALIGN(4);
  __small_data_start = .;
  _sdata_start = .;
  .sdata :
  {
    *(.sdata .sdata.* .gnu.linkonce.s.*)
  }
  . = ALIGN(4);
  __bss_start = .;
  __bss_start__ = .;
  .sbss :
  {
    PROVIDE(__sbss_start = .);
    PROVIDE(___sbss_start = .);
    *(.dynsbss)
    *(.sbss .sbss.* .gnu.linkonce.sb.*)
    *(.scommon)
    PROVIDE(__sbss_end = .);
    PROVIDE(___sbss_end = .);
  }
  __small_data_end = .;
  _gp = __small_data_start + 0x4000;
  ASSERT((__small_data_end - __small_data_start) <= 0x8000,
         "small data exceeds GP-relative range")
  _edata = .;
  PROVIDE(edata = .);
  . = ALIGN(4);
  .bss :
  {
    *(.dynbss)
    *(.bss .bss.* .gnu.linkonce.b.*)
    *(COMMON)
    . = ALIGN(4);
  }
  . = ALIGN(4);
  _end = .;
  _bss_end__ = .;
  __bss_end__ = .;
  __end__ = .;
  PROVIDE(end = .);
  .stab          0 : { *(.stab) }
  .stabstr       0 : { *(.stabstr) }
  .stab.excl     0 : { *(.stab.excl) }
  .stab.exclstr  0 : { *(.stab.exclstr) }
  .stab.index    0 : { *(.stab.index) }
  .stab.indexstr 0 : { *(.stab.indexstr) }
  .comment       0 : { *(.comment) }
  .debug          0 : { *(.debug) }
  .line           0 : { *(.line) }
  .debug_srcinfo  0 : { *(.debug_srcinfo) }
  .debug_sfnames  0 : { *(.debug_sfnames) }
  .debug_aranges  0 : { *(.debug_aranges) }
  .debug_pubnames 0 : { *(.debug_pubnames) }
  .debug_info     0 : { *(.debug_info .gnu.linkonce.wi.*) }
  .debug_abbrev   0 : { *(.debug_abbrev) }
  .debug_line     0 : { *(.debug_line) }
  .debug_frame    0 : { *(.debug_frame) }
  .debug_str      0 : { *(.debug_str) }
  .debug_loc      0 : { *(.debug_loc) }
  .debug_macinfo  0 : { *(.debug_macinfo) }
  .debug_weaknames 0 : { *(.debug_weaknames) }
  .debug_funcnames 0 : { *(.debug_funcnames) }
  .debug_typenames 0 : { *(.debug_typenames) }
  .debug_varnames  0 : { *(.debug_varnames) }
}
