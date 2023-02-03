#include "lib9.h"
#include "isa.h"
#include "interp.h"

char exNomem[]		 = "out of memory: main";

typedef struct stringlist stringlist;
typedef struct stringlist {
    char *s;
    stringlist *next;
} stringlist;

static void
writedata(uchar *st, uchar *en, const char *prefix, const char *name)
{
    print("TEXT %s_%s(SB), 0, $-4\n", prefix, name);
    while (st < en) {
        ulong w = *(ulong *)st;
        st += 4;
        if (st > en)
            w &= 0xffffffff >> ((st - en) * 8);
	print("WORD $0x%08ux\n", w);
    }
    print("\n");
}

static void
writestring(uchar *s)
{
    int i, l;
    l = strlen(s);
    for (i = 0; i < l; i += 4, s += 4) {
        ulong w = *(ulong *)s;
        if (i > (l - 4))
            w &= 0xffffffff >> ((i - (l - 4)) * 8);
        print("WORD $0x%08ux\n", w);
    }
    if (i % 4 == 0)
        print("WORD $0\n");
}

static void
writestringlist(stringlist *sl, char *prefix, char *section)
{
    for (int i = 0; sl->next != nil; i++) {
        stringlist *slp = sl->next;
        print("TEXT %s_%s_str%d(SB), 0, $-4\n", prefix, section, i);
        writestring(sl->s);
        free(sl);
        sl = slp;
    }
    print("\n");
}

static stringlist*
newstringlist(void)
{
    return calloc(1, sizeof(stringlist));
}

static stringlist*
appendstring(stringlist *sl, char *s)
{
    sl->s = s;
    sl->next = calloc(1, sizeof(stringlist));
    return sl->next;
}

static int
operand(uchar **p)
{
	int c;
	uchar *cp;

	cp = *p;
	c = cp[0];
	switch(c & 0xC0) {
	case 0x00:
		*p = cp+1;
		return c;
	case 0x40:
		*p = cp+1;
		return c|~0x7F;
	case 0x80:
		*p = cp+2;
		if(c & 0x20)
			c |= ~0x3F;
		else
			c &= 0x3F;
		return (c<<8)|cp[1];		
	case 0xC0:
		*p = cp+4;
		if(c & 0x20)
			c |= ~0x3F;
		else
			c &= 0x3F;
		return (c<<24)|(cp[1]<<16)|(cp[2]<<8)|cp[3];		
	}
	return 0;	
}

int
brpatchsrc(Inst *ip, Module *m)
{
	switch(ip->op) {
	case ICALL:
	case IJMP:
	case IBEQW:
	case IBNEW:
	case IBLTW:
	case IBLEW:
	case IBGTW:
	case IBGEW:
	case IBEQB:
	case IBNEB:
	case IBLTB:
	case IBLEB:
	case IBGTB:
	case IBGEB:
	case IBEQF:
	case IBNEF:
	case IBLTF:
	case IBLEF:
	case IBGTF:
	case IBGEF:
	case IBEQC:
	case IBNEC:
	case IBLTC:
	case IBLEC:
	case IBGTC:
	case IBGEC:
	case IBEQL:
	case IBNEL:
	case IBLTL:
	case IBLEL:
	case IBGTL:
	case IBGEL:
	case ISPAWN:
		if(ip->d.imm < 0 || ip->d.imm >= m->nprog)
			return 0;
		//ip->d.imm = (WORD)&m->prog[ip->d.imm];
		break;
	}
	return 1;
}

Type*
dtype(void (*destroy)(Heap*, int), int size, uchar *map, int mapsize)
{
	Type *t;

	t = malloc(sizeof(Type)-sizeof(t->map)+mapsize);
	if(t != nil) {
		t->ref = 1;
		t->free = 0;
		t->mark = 0;
		t->size = size;
		t->np = mapsize;
		memmove(t->map, map, mapsize);
	}
	return t;
}

static ulong
disw(uchar **p)
{
	ulong v;
	uchar *c;

	c = *p;
	v  = c[0] << 24;
	v |= c[1] << 16;
	v |= c[2] << 8;
	v |= c[3];
	*p = c + 4;
	return v;
}

Module*
process(char *path, int fdisfd, uchar *code, ulong length, Dir *dir, char *prefix)
{
	Inst *ip;
	Type *pt;
	Module *m;
	int lsize, id, v, entry, entryt, tnp, tsz, siglen;
	int c, de, pc, i, n, isize, dsize, hsize;
	uchar *mod, *istream, **isp;
	Link *l;

	istream = code;
	isp = &istream;

	m = malloc(sizeof(Module));
	if(m == nil)
		return nil;

	m->dev = dir->dev;
	m->dtype = dir->type;
	m->qid = dir->qid;
	m->mtime = dir->mtime;
	m->origmp = H;
	m->pctab = nil;

	switch(operand(isp)) {
	default:
		print("bad magic");
		goto bad;
	case SMAGIC:
		siglen = operand(isp);
		n = length-(*isp-code);
		if(n < 0 || siglen > n){
			print("corrupt signature");
			goto bad;
		}
		*isp += siglen;
		break;		
	case XMAGIC:
		break;
	}

	m->rt = operand(isp);
	m->ss = operand(isp);
	isize = operand(isp);
	dsize = operand(isp);
	hsize = operand(isp);
	lsize = operand(isp);
	entry = operand(isp);
	entryt = operand(isp);

	if(isize < 0 || dsize < 0 || hsize < 0 || lsize < 0) {
		fprint(2, "implausible Dis file");
		goto bad;
	}

	m->nprog = isize;
	m->prog = malloc(isize*sizeof(Inst));
	if(m->prog == nil) {
		fprint(2, exNomem);
		goto bad;
	}

	m->ref = 1;

	/* Instructions - some instructions are patched inline. */
	print("TEXT %s_inst(SB), 0, $-4\n", prefix);
	ip = m->prog;
	for(i = 0; i < isize; i++) {
                int target = -1;
		ip->op = *istream++;
		ip->add = *istream++;
		ip->reg = 0;
		ip->s.imm = 0;
		ip->d.imm = 0;
		switch(ip->add & ARM) {
		case AXIMM:
		case AXINF:
		case AXINM:
			ip->reg = operand(isp);
		 	break;
		}
		switch(UXSRC(ip->add)) {
		case SRC(AFP):
		case SRC(AMP):	
		case SRC(AIMM):
			ip->s.ind = operand(isp);
			break;
		case SRC(AIND|AFP):
		case SRC(AIND|AMP):
			ip->s.i.f = operand(isp);
			ip->s.i.s = operand(isp);
			break;
		}
		switch(UXDST(ip->add)) {
		case DST(AFP):
		case DST(AMP):	
			ip->d.ind = operand(isp);
			break;
		case DST(AIMM):
			ip->d.ind = operand(isp);
			if(brpatchsrc(ip, m) == 0) {
				print("bad branch addr");
				goto bad;
			}
                        target = ip->d.imm;
			break;
		case DST(AIND|AFP):
		case DST(AIND|AMP):
			ip->d.i.f = operand(isp);
			ip->d.i.s = operand(isp);
			break;
		}

                print("// %02ux %02ux %04ux %08ux %08ux\n", ip->op, ip->add, ip->reg, (int)ip->s.imm, (int)ip->d.imm);
		print("WORD $0x%04ux%02ux%02ux\n", ip->reg, ip->add, ip->op);
                print("WORD $0x%08ux\n", (int)(ip->s.imm));
                if (target != -1)
		    print("WORD $%s_inst+%d(SB)\n", prefix, target * sizeof(Inst));
                else
		    print("WORD $0x%08ux\n", (int)(ip->d.imm));

		ip++;
	}
	print("\n");

	/* Just include the rest as raw data and process it at load-time. */

	/* Record the start of the type data. */
	uchar *tstart = istream;

	m->ntype = hsize;
	m->type = malloc(hsize*sizeof(Type*));
	if(m->type == nil) {
		fprint(2, exNomem);
		goto bad;
	}
	for(i = 0; i < hsize; i++) {
		id = operand(isp);
		if(id > hsize) {
			fprint(2, "heap id range");
			goto bad;
		}
		tsz = operand(isp);
		tnp = operand(isp);
		if(tsz < 0 || tnp < 0 || tnp > 128*1024){
			fprint(2, "implausible Dis file");
			goto bad;
		}
		pt = dtype(0, tsz, istream, tnp);
		if(pt == nil) {
			fprint(2, exNomem);
			goto bad;
		}
		istream += tnp;
		m->type[id] = pt;
	}

	if(dsize != 0) {
		pt = m->type[0];
		if(pt == 0) {
			fprint(2, "bad desc for mp: pt=nil\n");
			goto bad;
		} else if (pt->size != dsize) {
			fprint(2, "bad desc for mp: pt=%ulx size=%d\n", (ulong)pt, pt->size);
			goto bad;
		}
	}

	/* Data - just include the raw data in an array and leave processing
	   until later. */

	for(;;) {
		uchar sm = *istream++;
		if(sm == 0)
			break;
		n = DLEN(sm);
		if(n == 0)
			n = operand(isp);
		v = operand(isp);
		switch(DTYPE(sm)) {
		default:
			fprint(2, "bad data item");
			goto bad;
		case DEFS:
		case DEFB:
			istream += n;
			break;
		case DEFW:
			istream += n * sizeof(WORD);
			break;
		case DEFL:
			istream += n * sizeof(LONG);
			break;
		case DEFF:
			istream += n * sizeof(REAL);
			break;
		case DEFA:			/* Array */
			istream += 2 * sizeof(WORD);
			/* Handle checks at run-time. */
			break;
		case DIND:			/* Set index */
			istream += sizeof(WORD);
			/* Handle checks at run-time. */
			break;
		case DAPOP:
			/* Handle checks at run-time. */
			break;
		}
	}

	/* Module name - should be usable as it is. */
	mod = istream;
	if(memchr(mod, 0, 128) == 0) {
		print("bad module name");
		goto bad;
	}
	m->name = strdup((char*)mod);
	if(m->name == nil) {
		print(exNomem);
		goto bad;
	}

	c = 0;
	while(*istream++)
	    c++;

	l = m->ext = (Link*)malloc((lsize+1)*sizeof(Link));
	if(l == nil){
		fprint(2, exNomem);
		goto bad;
	}
	for(i = 0; i < lsize; i++, l++) {
		pc = operand(isp);
		de = operand(isp);
		v  = disw(isp);
		pt = nil;
		if(de != -1)
			pt = m->type[de];

		while(*istream++)
			;
	}
	l->name = nil;

	/* Imports - these should be usable as generated. */

	if(m->rt & HASLDT0){
		fprint(2, "obsolete dis");
		goto bad;
	}
	if(m->rt & HASLDT){
		int j, nl;
		Import *i1, **i2;

		nl = operand(isp);
		if (nl > 0) {
		    for(i = 0; i < nl; i++, i2++){
			    n = operand(isp);
			    for(j = 0; j < n; j++, i1++){
				    disw(isp);
				    while(*istream++)
					    ;
			    }
		    }
		    istream++;
                }
        }

	if(m->rt & HASEXCEPT){
		int j, nh;
		Handler *h;
		Except *e;

		nh = operand(isp);
		m->htab = malloc((nh+1)*sizeof(Handler));
		if(m->htab == nil){
			fprint(2, exNomem);
			goto bad;
		}
		h = m->htab;
		for(i = 0; i < nh; i++, h++){
			h->eoff = operand(isp);
			h->pc1 = operand(isp);
			h->pc2 = operand(isp);
			n = operand(isp);
			if(n != -1)
				h->t = m->type[n];
			n = operand(isp);
			h->ne = n>>16;
			n &= 0xffff;
			h->etab = malloc((n+1)*sizeof(Except));
			if(h->etab == nil){
				fprint(2, exNomem);
				goto bad;
			}
			e = h->etab;
			for(j = 0; j < n; j++, e++){
				e->s = strdup((char*)istream);
				if(e->s == nil){
					fprint(2, exNomem);
					goto bad;
				}
				while(*istream++)
					;
				e->pc = operand(isp);
			}
			e->s = nil;
			e->pc = operand(isp);
		}
		istream++;
	}
        writedata(tstart, istream, prefix, "data");

        /* Strip leading directories and prepend a slash. */
        char *p = path;
        while (*p) {
            if (*p != '.' && *p != '/') break;
            p++;
        }
        int pl = strlen(p);
        char *np = malloc(sizeof(pl + 2));
        *np = '/';
        strcpy(np + 1, p);

        print("/* %s */\n", np);
        print("TEXT %s_modpath(SB), 0, $-4\n", prefix);
        writestring(np);
        print("\n");

        /* Write the fdis file containing the magic number and the module name. */
        int magic = 0x15fd00c0;
        char null = 0;
        write(fdisfd, &magic, 4);
        write(fdisfd, np, strlen(np));
        write(fdisfd, &null, 1);
        close(fdisfd);
        free(np);

	/* Write out module information. */
        print("TEXT %smod(SB), 0, $-4\n", prefix);
	print("WORD $%d /* ss */\n", m->ss);
	print("WORD $%d /* rt */\n", m->rt);
	print("WORD $%d /* nprog */\n", isize);
	print("WORD $%d /* dsize */\n", dsize);
	print("WORD $%d /* ntype */\n", hsize);
	print("WORD $%d /* nlink */\n", lsize);
        print("WORD $%#lux /* entry */\n", entry);
        print("WORD $%#lux /* entryt */\n", entryt);
        print("WORD $%s_inst(SB)\n", prefix);
        print("WORD $%s_data(SB)\n", prefix);
        print("WORD $%s_modpath(SB)\n", prefix);

	return m;
bad:
	free(m);
	return nil;
}

void
main(int argc, char *argv[])
{
    if (argc != 4) {
	fprint(2, "Usage: freeze <prefix> <dis file> <fdis file>\n");
        fprint(2, "Writes an assembly file on stdout.\n");
	return;
    }

    char *prefix = argv[1];
    char *fname = argv[2];
    char *fdisname = argv[3];

    Dir *dir = dirstat(fname);
    if (dir == nil) {
	fprint(2, "File not found: %s\n", fname);
	return;
    }

    int fd = open(fname, OREAD);
    if (fd < 0) {
	fprint(2, "Failed to open file '%s' for reading\n", fname);
	return;
    }

    int ffd = create(fdisname, OWRITE, 0666);
    if (ffd < 0) {
	fprint(2, "Failed to open file '%s' for writing\n", fdisname);
	return;
    }

    uchar *code = malloc(dir->length);
    if (code == nil) {
	fprint(2, "Unable to allocate memory\n");
	goto done;
    }

    int n = read(fd, code, dir->length);
    if (n != dir->length) {
	free(code);
	fprint(2, "Failed to read file: %s\n", fname);
	goto done;
    }

    /* ### The file name needs to be converted to a full path. */
    process(fname, ffd, code, dir->length, dir, prefix);

done:
    if (fd >= 0) close(fd);
}
