#include "lib9.h"
#include "isa.h"
#include "interp.h"
#include "raise.h"
#include <kernel.h>

#define	A(r)	*((Array**)(r))

FrozenMods* frozen;
extern Module* modules;

extern double canontod(ulong v[2]);
extern ulong disw(uchar **p);
extern int operand(uchar **p);

void
addfrozenmod(FrozenMod *f)
{
    FrozenMods *nf = malloc(sizeof(FrozenMods));
    nf->mod = f;
    nf->link = frozen;
    frozen = nf;
}

static Module*
_loadfrozen(Module *m, FrozenMod *f)
{
    Array *ary;
    Heap *h;
    String *s;
    Type *pt;
    WORD lo, hi;
    int dasp, i, id, n, tnp, tsz, v;
    uchar *addr, *dastack[DADEPTH], *si, sm;
    uchar *istream;
    uchar **isp = &istream;
    ulong ul[2];

    /* Fill in the members of the Module struct. */
    m->ref = 1;
    m->compiled = 0;
    m->frozen = 1;
    m->ss = f->ss;
    m->rt = f->rt;
    m->nprog = f->nprog;
    m->prog = f->inst;
    m->ntype = f->ntype;
    m->name = f->name;
    m->path = f->path;

    m->type = malloc(m->ntype*sizeof(Type*));
    if(m->type == nil) {
        kwerrstr(exNomem);
        goto bad;
    }

    istream = f->types;
    for(i = 0; i < m->ntype; i++) {
        id = operand(isp);
        if(id > m->ntype) {
            kwerrstr("heap id range");
            goto bad;
        }
        tsz = operand(isp);
        tnp = operand(isp);
        if(tsz < 0 || tnp < 0 || tnp > 128*1024){
            kwerrstr("implausible Dis file");
            goto bad;
        }
        pt = dtype(freeheap, tsz, istream, tnp);
        if(pt == nil) {
            kwerrstr(exNomem);
            goto bad;
        }
        istream += tnp;
        m->type[id] = pt;
    }

    if(f->dsize != 0) {
        pt = m->type[0];
        if(pt == 0 || pt->size != f->dsize) {
            kwerrstr("bad desc for mp");
            goto bad;
        }
        h = heapz(pt);
        m->origmp = H2D(uchar*, h);
    }

    istream = f->data;
    addr = m->origmp;
    dasp = 0;
    for(;;) {
        sm = *istream++;
        if(sm == 0)
            break;
        n = DLEN(sm);
        if(n == 0)
            n = operand(isp);
        v = operand(isp);
        si = addr + v;
        switch(DTYPE(sm)) {
        default:
            kwerrstr("bad data item");
            goto bad;
        case DEFS:
            s = c2string((char*)istream, n);
            istream += n;
            *(String**)si = s;
            break;
        case DEFB:
            for(i = 0; i < n; i++)
                *si++ = *istream++;
            break;
        case DEFW:
            for(i = 0; i < n; i++) {
                *(WORD*)si = disw(isp);
                si += sizeof(WORD);
            }
            break;
        case DEFL:
            for(i = 0; i < n; i++) {
                hi = disw(isp);
                lo = disw(isp);
                *(LONG*)si = (LONG)hi << 32 | (LONG)(ulong)lo;
                si += sizeof(LONG);
            }
            break;
        case DEFF:
            for(i = 0; i < n; i++) {
                ul[0] = disw(isp);
                ul[1] = disw(isp);
                *(REAL*)si = canontod(ul);
                si += sizeof(REAL);
            }
            break;
        case DEFA:            /* Array */
            v = disw(isp);
            if(v < 0 || v > m->ntype) {
                kwerrstr("bad array type");
                goto bad;
            }
            pt = m->type[v];
            v = disw(isp);
            h = nheap(sizeof(Array)+(pt->size*v));
            h->t = &Tarray;
            h->t->ref++;
            ary = H2D(Array*, h);
            ary->t = pt;
            ary->len = v;
            ary->root = H;
            ary->data = (uchar*)ary+sizeof(Array);
            memset((void*)ary->data, 0, pt->size*v);
            initarray(pt, ary);
            A(si) = ary;
            break;            
        case DIND:            /* Set index */
            ary = A(si);
            if(ary == H || D2H(ary)->t != &Tarray) {
                kwerrstr("ind not array");
                goto bad;
            }
            v = disw(isp);
            if(v > ary->len || v < 0 || dasp >= DADEPTH) {
                kwerrstr("array init range");
                goto bad;
            }
            dastack[dasp++] = addr;
            addr = ary->data+v*ary->t->size;
            break;
        case DAPOP:
            if(dasp == 0) {
                kwerrstr("pop range");
                goto bad;
            }
            addr = dastack[--dasp];
            break;
        }
    }

    /* Initialise links from the encoded structures. */
    Link *l = m->ext = (Link*)malloc((f->nlink+1)*sizeof(Link));
    if (l == nil) {
        fprint(2, "out of memory: main");
        goto bad;
    }
    for (i = 0; i < f->nlink; i++, l++) {
        int pc = f->links[i].pc;
        int de = f->links[i].de;
        int v  = f->links[i].v;
        uchar *fn_name = f->links[i].fn_name;
        pt = nil;
        if(de != -1)
            pt = m->type[de];
        mlink(m, l, fn_name, v, pc, pt);
    }
    l->name = nil;

    if(m->rt & HASLDT0){
        kwerrstr("obsolete dis");
        goto bad;
    }

    /* Initialise imports as encoded. */
    if(m->rt & HASLDT)
        m->ldt = f->imports;

    istream = f->handlers;
    if(m->rt & HASEXCEPT){
        int j, nh;
        Handler *h;
        Except *e;

        nh = operand(isp);
        m->htab = malloc((nh+1)*sizeof(Handler));
        if(m->htab == nil){
            kwerrstr(exNomem);
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
                kwerrstr(exNomem);
                goto bad;
            }
            e = h->etab;
            for(j = 0; j < n; j++, e++){
                e->s = strdup((char*)istream);
                if(e->s == nil){
                    kwerrstr(exNomem);
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

    /* Fill in entry information. */
    m->entryt = nil;
    m->entry = m->prog;
    if((ulong)f->entry < f->nprog && (ulong)f->entryt < f->ntype) {
        m->entry = &m->prog[f->entry];
        m->entryt = m->type[f->entryt];
    }

    m->link = modules;
    modules = m;

    return m;

bad:
    destroy(m->origmp);
    free(m);
    return nil;
}

/*
    istream = (uchar *)imports;
    if(m->rt & HASLDT){
        int j, nl;
        Import *i1, **i2;

        nl = operand(isp);
        i2 = m->ldt = (Import**)malloc((nl+1)*sizeof(Import*));
        if(i2 == nil){
            kwerrstr(exNomem);
            goto bad;
        }
        for(i = 0; i < nl; i++, i2++){
            n = operand(isp);
            i1 = *i2 = (Import*)malloc((n+1)*sizeof(Import));
            if(i1 == nil){
                kwerrstr(exNomem);
                goto bad;
            }
            for(j = 0; j < n; j++, i1++){
                i1->sig = disw(isp);
                i1->name = strdup((char*)istream);
                if(i1->name == nil){
                    kwerrstr(exNomem);
                    goto bad;
                }
                while(*istream++)
                    ;
            }
        }
        istream++;
    }
*/

Module*
loadfrozen(Module *m, uchar *path)
{
    FrozenMods *fm;

    for (fm = frozen; fm != nil; fm = fm->link) {
        if (strcmp((char *)path, fm->mod->path) == 0)
            return _loadfrozen(m, fm->mod);
    }
    free(m);
    return nil;
}
