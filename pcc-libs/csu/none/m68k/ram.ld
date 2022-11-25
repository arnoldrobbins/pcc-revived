MEMORY
{
	RAM (xrw)	: ORIGIN = 0x00000000, LENGTH =  512K
}

STACK_SIZE = 0x1000;

OUTPUT_FORMAT(srec)
ENTRY(_start)

INPUT( -lio )

PROVIDE(__stack_top = _estack);
PROVIDE(__bss_start = _sbss);
PROVIDE(__bss_end = _ebss);
PROVIDE(__bss_size = _ebss - _sbss);
PROVIDE(__heap_start = _sheap);
PROVIDE(__heap_end = _eheap);

SECTIONS
{
	_estack = ORIGIN(RAM) + LENGTH(RAM);

	. = 0x00000000;
	.text :
	{
		. = ALIGN(4);
		KEEP(*(.vtable))
		*(.text)                   /* remaining code */
		*(.text.*)
		*(.init)
		*(.init.*)
		*(.fini)
		*(.fini.*)
		*(.rodata)                 /* read-only data (constants) */
		*(.rodata.*)
		*(.ctors)
		*(.dtors)
		. = ALIGN(4);
		_etext = .;
		. = ALIGN(4);
		*(.data)
		*(.data.*)
	} >RAM

	.bss (NOLOAD):
	{
		. = ALIGN(4);
		_sbss = .;                 /* start of .bss label (for startup) */
		*(.bss)
		*(.bss.*)
		*(COMMON)
		. = ALIGN(4);
		_ebss = .;                 /* end of .bss label (for startup) */
	} >RAM

	_sheap = _ebss;
	_eheap = _estack - STACK_SIZE;

	DISCARD :    /* remove the debugging information from the standard libraries */
	{
		libc.a ( * )
		libpcc.a ( * )
	}

}
