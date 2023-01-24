implement Ls;

include "draw.m";
include "sys.m";
include "workdir.m";

Ls: module
{
    init:	fn(nil: ref Draw->Context, nil: list of string);
};

init(nil: ref Draw->Context, argv: list of string)
{
    sys := load Sys Sys->PATH;
    gwd := load Workdir Workdir->PATH;

    if (len(argv) < 2) {
        wd := gwd->init();
        if (wd == nil)
            return;
        argv = wd::nil;
    } else
        argv = tl argv;

    for (; argv != nil; argv = tl argv) {

        file := hd argv;
	(ok, dir) := sys->stat(file);
        if (ok < 0) continue;

        if (dir.mode & Sys->DMDIR != 0) {
            fd := sys->open(dir.name, Sys->OREAD);
            if (fd != nil) {
                for (;;) {
                    (nr, b) := sys->dirread(fd);
                    if (nr <= 0) break;
                    for (i := 0; i < nr; i++)
                        sys->print("%s\n", b[i].name);
                }
            }
        } else
            sys->print("%s\n", dir.name);
    }
}
