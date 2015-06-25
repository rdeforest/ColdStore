// Elf - dumping of Elf DSOs
// Copyright (C) 1998,1999 Colin McCormack,
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms
// 	$Id

void dumpSectName(const LINK_MAP *l, Elf32_Section s)
{
    switch (s) {
        case SHN_UNDEF:
            cerr << "*UND*"; break;
        case SHN_ABS:
            cerr << "*ABS*"; break;
        case SHN_COMMON:
            cerr << "*COM*"; break;
        default:
            if (s < SHN_LORESERVE) {
                cerr << s;
            } else {
                cerr << "*UNKNOWN*" << s;
            }
    }
}

void dumpBind(const LINK_MAP *l, int b)
{
    switch (b) {
        case STB_LOCAL:
            cerr << "LOCAL"; break;
        case STB_GLOBAL:
            cerr << "GLOBAL"; break;
        case STB_WEAK:
            cerr << "WEAK"; break;
        default:
            cerr << "UNKNOWN"; break;
    }
}

void dumpType(const LINK_MAP *l, int t)
{
    switch (t) {
        case STT_NOTYPE:
            cerr << "NOTYPE"; break;
        case STT_OBJECT:
            cerr << "OBJECT"; break;
        case STT_FUNC:
            cerr << "FUNC"; break;
        case STT_SECTION:
            cerr << "SECTION"; break;
        case STT_FILE:
            cerr << "FILE"; break;
        default:
            cerr << "UNKNOWN"; break;
    }
}

void dumpSym(const LINK_MAP *l, const ELFSYM *sym)
{
    const char *strtab = (const char*)(l->STRINGTABLE);

    cerr << sym << ' ' << strtab + sym->st_name << ':';
    cerr << "\n\tvalue: " << (void*)(sym->st_value);
    cerr << "\n\tsize: " << sym->st_size;
    cerr << "\n\ttype: "; dumpType(l, ELF32_ST_TYPE(sym->st_info));
    cerr << "\n\tbind: "; dumpBind(l, ELF32_ST_BIND(sym->st_info));
    //cerr << "\n\tother: " << sym->st_other;
    cerr << "\n\tsection: "; dumpSectName(l, sym->st_shndx);
    cerr << '\n';
}

void dumpSymIdx(const LINK_MAP *l, ELFSYMNDX symidx)
{
    const ELFSYM *sym = (ELFSYM *)(l->SYM_TAB);
    sym = sym + symidx;

    // dump the symbol sym
    dumpSym(l, sym);
}

void dumpObj(const LINK_MAP *l)
{
#ifndef __FreeBSD__
    int *hash = (int *)(l->l_info[DT_HASH]->d_un.d_ptr + l->l_addr);
#endif
    cerr << l << ':';
    cerr << " l_next: " << l->LINK_NEXT;
#ifndef __FreeBSD__
    cerr << " l_prev: " << l->l_prev;
    cerr << "\n\tl_type: " << l->l_type;
#endif
    cerr << '\n';
    cerr << l->LINK_NAME;
    cerr << "\n\tl_addr: " << (void*)(l->START_ADDRESS);
    cerr << "\n\tsymtab: " << (void *)(l->SYM_TAB);
#ifndef __FreeBSD__
    cerr << "\n\thash: " << hash;
    cerr << "\tbuckets: " << *hash;
    cerr << "\tchains: " << hash[1];
#else
    cerr << "\tbuckets: " << l->NBUCKETS;
    cerr << "\tchains: " << l->nchains;
#endif
    //cerr << "\tsyment: " << l->l_info[DT_SYMENT]->d_un.d_val;
    cerr << "\n\tl_nbuckets: " << l->NBUCKETS;
    cerr << "\tl_buckets: " << l->BUCKETS;
    cerr << "\tl_chain: " << l->CHAIN;
    cerr << "\n\tstrtab: " << (void*)l->STRINGTABLE;
    cerr << "\tstrtab size: " << l->STRINGSIZE;
#ifndef __FreeBSD__
    cerr << "\n\tl_versyms: " << l->l_versyms;
#endif
    cerr << '\n';
}
