// macros to define absolute symbols in the ColdStore
// Copyright (C) 1998,1999 Colin McCormack,
// see LICENSE (MD5 d7a11a4148f3858e544bf89cdde91727) for terms

// macros to define absolute symbols in the coldstore
#define STARTABS asm(".section .absolute");asm(".globl _4Mmap.end"); asm(".type _4Mmap.end,@object"); asm(".size _4Mmap.end,0"); asm(".set _4Mmap.end,0x50000000"); asm(".globl _4Mmap.header"); asm(".type _4Mmap.header,@object"); asm(".set _4Mmap.header,_4Mmap.end");
#define ABS(NAME, TYPE, SIZE) asm(".section .absolute"); asm(".globl " #NAME ); asm(".type " #NAME "," #TYPE ); asm(".size " #NAME "," #SIZE ); asm(".set " #NAME ",_4Mmap.end"); asm("_4Mmap.end=_4Mmap.end+" #SIZE);
#define OBJECT(NAME, SIZE) asm(".section .absolute"); asm(".globl " #NAME ); asm(".type " #NAME ", @object "); asm(".size " #NAME "," #SIZE ); asm(".set " #NAME ",_4Mmap.end"); asm("_4Mmap.end=_4Mmap.end+" #SIZE);
#define FUNCTION(NAME) asm(".section .absolute"); asm(".globl " #NAME ); asm(".type " #NAME ", @function "); asm(".size " #NAME ",8"); asm(".set " #NAME ",_4Mmap.end"); asm("_4Mmap.end=_4Mmap.end+8");
#define ENDABS asm(".section .absolute"); asm(".globl _4Mmap.store"); asm("_4Mmap.store=(_4Mmap.end+4095) & ~4095"); asm(".size _4Mmap.header,(_4Mmap.end - _4Mmap.header)"); asm(".size _4Mmap.end,(_4Mmap.store - _4Mmap.end)");

STARTABS
// sundry interned globals
OBJECT(coldmagic, 4);		// coldstore magic value
OBJECT(coldroot, 4);		// coldstore root pointer
OBJECT(store, 4);		// coldstore's personality object
OBJECT(_t5gPool1Z7Integer.all, 4);		// coldstore's Pool set
OBJECT(_t5gPool1Z6String.all, 4);
OBJECT(_10Connection.all, 4);
OBJECT(_6Server.all, 4);
OBJECT(_10Connection.pending, 4);
OBJECT(_4File.all, 4);
OBJECT(_5DopeV.all, 4);
OBJECT(_8noopNode.noop, 4);

// chaos interned globals
OBJECT(_5Chaos.no_error, 4);
OBJECT(_5Chaos.dquote, 4);
OBJECT(_5Chaos.hex, 4);
OBJECT(_5Chaos.integer, 4);
OBJECT(_5Chaos.quote, 4);
OBJECT(_5Chaos.real, 4);
OBJECT(_5Chaos.token, 4);
OBJECT(_5Chaos.var, 4);
OBJECT(_5Chaos.ws, 4);

// qvmm interned globals
OBJECT(_4Mmap.size, 4);		// size of coldstore
OBJECT(_5BTree.tree, 4);	// storage BTree root
OBJECT(BTreeRoot, 12);		// BTree root
OBJECT(_5Latch.latch, 4096);	// Latch array
OBJECT(__vt_8RefCount, 20);	// RefCount's vtable

// Elf interned globals
OBJECT(_3Elf.all, 12);		// Q of all Elf objects
OBJECT(__vt_3Elf, 12);		// Elf vtable
OBJECT(__vt_4Csym, 12);		// Csym vtable
OBJECT(__vt_t1Q1Z3Elf, 12);	// Q<Elf> vtable
OBJECT(__vt_t1Q1Z4Csym, 12);	// Q<Csym> vtable
ENDABS
