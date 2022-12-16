;/***************************************************************************
; *                                                                         *
; * Copyright (c) 2019 Nuvoton Technology. All rights reserved.             *
; *                                                                         *
; ***************************************************************************/

ENTRY(__vector)

MEMORY
{
  RAM  (rwx)   : ORIGIN = 0x000000, LENGTH = 0x08000000
}

SECTIONS
{
  .text : ALIGN(4)
  {
    PROVIDE(__image_start = .);
    PROVIDE(__text_start = .);
    PROVIDE(__vectors_start = .);
    *(.vectors);
    . = ALIGN(4);
    PROVIDE(__vectors_end = .);

    *(.text*)
    *(.rodata .rodata.* .constdata .constdata.*)

    /* C++ constructors etc */
    . = ALIGN(4);
    KEEP(*(.init))
            
    . = ALIGN(4);
    __preinit_array_start = .;
    KEEP (*(.preinit_array))
    __preinit_array_end = .;
            
    . = ALIGN(4);
    __init_array_start = .;
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array))
    __init_array_end = .;
            
    /* array of function pointers for the the static destructors */
    __fini_array_start = .;
    KEEP(*(SORT(.fini_array.*)))
    KEEP(*(.fini_array))
    __fini_array_end = .;
            
    KEEP(*(.fini));
            
    . = ALIGN(4);
    KEEP (*crtbegin.o(.ctors))
    KEEP (*(EXCLUDE_FILE (*crtend.o) .ctors))
    KEEP (*(SORT(.ctors.*)))
    KEEP (*crtend.o(.ctors))
            
    . = ALIGN(4);
    KEEP (*crtbegin.o(.dtors))
    KEEP (*(EXCLUDE_FILE (*crtend.o) .dtors))
    KEEP (*(SORT(.dtors.*)))
    KEEP (*crtend.o(.dtors))
    . = ALIGN(4);
    /* End C++ */

    etext = .;
    
    PROVIDE(__text_end = .);
  } > RAM

  . = ALIGN(4);
  _etext = . ;
  PROVIDE (etext = .);
   
  .data : AT (_etext)
  {
    PROVIDE(__data_start__ = .);
    _data = . ;
    *(.data*)
    . = ALIGN(8);

    . = ALIGN(32);

    PROVIDE(__data_end__ = .);
  } > RAM

  . = ALIGN(4);
  _edata = . ;
  PROVIDE (edata = .);

  /* Uninitialized data section
   * This region is not initialized by the C/C++ library and can be used to
   * store state across soft reboots. */
  .uninitialized (NOLOAD):
  {
    . = ALIGN(32);
     __uninitialized_start = .;
    *(.uninitialized)
    KEEP(*(.keep.uninitialized))
    . = ALIGN(32);
    __uninitialized_end = .;
  } > RAM

  sbss = .;
  .bss (NOLOAD):
  {
    PROVIDE (__bss_start__ = .);
    *(.bss*)
    *(COMMON)
    . = ALIGN(4);
    PROVIDE (__bss_end__ = .);
  }>RAM
  ebss = .;
  bss_size = ebss - sbss;

  .heap (NOLOAD):
  {
    . = ALIGN(8);
    end = .;
  } >RAM

  PROVIDE_HIDDEN (__exidx_start = .);
  .ARM.exidx : { *(.ARM.exidx* .gnu.linkonce.armexidx.*) }
  PROVIDE_HIDDEN (__exidx_end = .);
}

