# partfs.b
#
# Written in 2019 by David Boddie <david@boddie.org.uk>
#
# To the extent possible under law, the author(s) have dedicated all copyright
# and related and neighboring rights to this software to the public domain
# worldwide. This software is distributed without any warranty.
#
# You should have received a copy of the CC0 Public Domain Dedication along with
# this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.

# Queries USB mass storage devices that can be accessed from outside the USB
# driver framework.

implement PartFs;

include "sys.m";
    sys: Sys;

include "string.m";
    str: String;

include "draw.m";

include "styx.m"; styx: Styx;
    Rmsg, Tmsg: import styx;

include "styxservers.m";
    styxservers: Styxservers;
    Styxserver, Navigator, Navop, Enotfound, Enotdir: import styxservers;

# Convenient variable for use with fprint
stderr: ref sys->FD;

# Global file handle for the underlying disk file
disk_file: ref sys->FD;

block_size: con big 512;

PartFs: module
{
    init: fn(ctxt: ref Draw->Context, args: list of string);
};

# The initialisation function finds a device and its endpoints then starts a
# file system to handle interaction with the device.

init(nil: ref Draw->Context, args: list of string)
{
    sys = load Sys Sys->PATH;
    str = load String String->PATH;
    styx = load Styx Styx->PATH;
    styxservers = load Styxservers Styxservers->PATH;

    stderr = sys->fildes(2);

    if (len args != 2) {
        sys->fprint(stderr, "usage: partfs disk\n");
        exit;
    }

    args = tl args;
    disk_file_name := hd args;

    disk_file = sys->open(disk_file_name, Sys->ORDWR);

    if (disk_file == nil) {
        sys->fprint(stderr, "%r");
        exit;
    }

    startfs();
}

# File server functionality

Qroot, Qctl, Qdata, Qfirstpartition: con iota;
tab := array[] of {
    (Qroot, ".", Sys->DMDIR|8r555),
    (Qctl, "ctl", 8r644),
    (Qdata, "data", 8r444)
};

Partition: adt {
    type_: string;
    begin, end: big;

    info: fn(p: self ref Partition): string;
};

Partition.info(p: self ref Partition): string
{
    return sys->sprint("part %s %ubd %ubd\n", p.type_, p.begin, p.end);
}

partitions: array of ref Partition;

user: string;

startfs()
{
    # Make the current user the file owner, falling back to a default user.
    user = readfile("/dev/user");
    if(user == nil)
        user = "inferno";

    # Initialise the modules before doing anything else.
    styx->init();
    styxservers->init(styx);

    # Create a channel and pass it to the navigator thread before starting it.
    navch := chan of ref Navop;
    spawn navigator(navch);

    # Create a navigator to handle communication with the navigator thread.
    nav := Navigator.new(navch);
    (tc, srv) := Styxserver.new(sys->fildes(0), nav, big Qroot);
    servloop(tc, srv);
}

navigator(c: chan of ref Navop)
{
    loop: while (1) {
        navop := <- c;
        pick op := navop {
        Stat =>
            index := int op.path;
            if (index < Qfirstpartition)
                op.reply <-= (tab_dir(index), nil);
            else
                op.reply <-= (part_dir(index - Qfirstpartition), nil);

        Walk =>
            if (op.name == "..") {
                op.reply <-= (tab_dir(Qroot), nil);
                continue loop;
            }
            case int op.path & 16rff {
            Qroot =>
                # Examine the fixed entries first.
                for (i := 1; i < Qfirstpartition; i++) {
                    if (tab[i].t1 == op.name) {
                        op.reply <-= (tab_dir(i), nil);
                        # Break out to the outer while loop.
                        continue loop;
                    }
                }
                # Examine the partitions afterwards.
                for (i = 0; i < len partitions; i++) {
                    if (op.name == sys->sprint("p%ud", i + 1)) {
                        op.reply <-= (part_dir(i), nil);
                        # Break out to the outer while loop.
                        continue loop;
                    }
                }
                op.reply <-= (nil, Enotfound);
            * =>
                op.reply <-= (nil, Enotdir);
            }

        Readdir =>
            # Qctl, Qdata + partitions (exclude Qroot)
            nentries := (len tab) + (len partitions) - 1;
            for (i := 0; i < op.count && i + op.offset < nentries; i++) {
                j := i + op.offset + 1;
                if (j < Qfirstpartition)
                    op.reply <-= (tab_dir(j), nil);
                else {
                    p := j - Qfirstpartition;
                    op.reply <-= (part_dir(p), nil);
                }
            }
            op.reply <-= (nil, nil);
        }
    }
}

servloop(tc: chan of ref Tmsg, srv: ref Styxserver)
{
    loop: while((tmsg := <-tc) != nil) {

        pick tm := tmsg {
        Open =>
            srv.default(tm);

        Read =>
            (f, err) := srv.canread(tm);

            if (f == nil) {
                srv.reply(ref Rmsg.Error(tm.tag, err));
                break;
            }

            if (f.qtype & Sys->QTDIR) {
                srv.default(tm);
                continue loop;
            }

            data := array[tm.count] of byte;

            case int f.path {
            Qctl =>
                srv.reply(styxservers->readstr(tm, partitions_ctl()));
            Qdata =>
                sys->seek(disk_file, tm.offset, Sys->SEEKSTART);
                if (sys->readn(disk_file, data, tm.count) > 0)
                    srv.reply(ref Rmsg.Read(tm.tag, data));
                else
                    srv.reply(ref Rmsg.Error(tm.tag, "partfs: file read error"));
            * =>
                if (int f.path < Qfirstpartition + len partitions) {
                    # Handle files corresponding to partitions.
                    partition := partitions[int f.path - Qfirstpartition];
                    offset := (partition.begin * block_size) + tm.offset;

                    sys->seek(disk_file, offset, Sys->SEEKSTART);
                    if (sys->readn(disk_file, data, tm.count) > 0)
                        srv.reply(ref Rmsg.Read(tm.tag, data));
                    else
                        srv.reply(ref Rmsg.Error(tm.tag, "partfs: file read error"));
                } else {
                    # Issue a default response.
                    srv.default(tm);
                }
            }
        Write =>
            (f, err) := srv.canwrite(tm);

            if (f == nil) {
                srv.reply(ref Rmsg.Error(tm.tag, err));
                break;
            }

            if (f.qtype & Sys->QTDIR) {
                srv.default(tm);
                continue loop;
            }

            case int f.path {
            Qctl =>
                if (write_ctl(tm.data) == 0)
                    srv.reply(ref Rmsg.Write(tm.tag, len tm.data));
                else
                    srv.reply(ref Rmsg.Error(tm.tag, sys->sprint("%r")));
            * =>
                if (int f.path < Qfirstpartition + len partitions) {
                    # Handle files corresponding to partitions.
                    partition := partitions[int f.path - Qfirstpartition];
                    offset := (partition.begin * block_size) + tm.offset;

                    sys->seek(disk_file, offset, Sys->SEEKSTART);
                    count := len tm.data;
                    if (sys->write(disk_file, tm.data, count) == count)
                        srv.reply(ref Rmsg.Write(tm.tag, count));
                    else
                        srv.reply(ref Rmsg.Error(tm.tag, "partfs: file write error"));
                } else {
                    # Issue a default response.
                    srv.default(tm);
                }
            }
        * =>
            srv.default(tmsg);
        }
    }
}

# Here are a few utility functions, not particularly required reading.

# Reads a file (or the first chunk if its contents don't fit into one read())
readfile(f: string): string
{
    fd := sys->open(f, Sys->OREAD);
    if(fd == nil)
        return nil;
    buf := array[Sys->ATOMICIO] of byte;
    n := sys->read(fd, buf, len buf);
    if(n <= 0)
        return nil;
    return string buf[0:n];
}

tab_dir(path: int): ref Sys->Dir
{
    (nil, name, perm) := tab[path];
    return dir(name, perm, path);
}

part_dir(index: int): ref Sys->Dir
{
    return dir(sys->sprint("p%ud", index + 1), 8r644, Qfirstpartition + index);
}

# Given a path inside the table, this returns a Sys->Dir representing that path.
dir(name: string, perm: int, path: int): ref Sys->Dir
{
    d := ref sys->zerodir;
    d.name = name;
    d.uid = d.gid = user;
    d.qid.path = big path;
    if (perm & Sys->DMDIR)
        d.qid.qtype = Sys->QTDIR;
    else
        d.qid.qtype = Sys->QTFILE;
    d.mtime = d.atime = 0;
    d.mode = perm;
    case path {
        Qroot =>
            ;
        Qctl =>
            d.length = big len partitions_ctl();
        Qdata =>
            (ok, stat) := sys->fstat(disk_file);
            if (ok)
                d.length = stat.length;
        * =>
            partition := partitions[path - Qfirstpartition];
            d.length = block_size * (partition.end - partition.begin);
    }
    return d;
}

partitions_ctl(): string
{
    s := "";
    for (i := 0; i < len partitions; i++)
        s += partitions[i].info();

    return s;
}

write_ctl(data: array of byte): int
{
    lines := string data;
    new_partitions: list of ref Partition;

    while (lines != nil) {
        line: string;
        (line, lines) = str->splitstrl(string data, "\n");

        pieces := split(line);

        if ((len pieces) == 4 && pieces[0] == "part") {
            begin := big pieces[2];
            end := big pieces[3];
            if (begin >= big 0 && begin < end) {
                part := ref Partition(pieces[1], begin, end);
                new_partitions = part::new_partitions;
            }
        }

        if (lines != nil)
            lines = lines[1:];
    }

    # Extend the partition entries with the new ones.
    parts := array[len partitions + len new_partitions] of ref Partition;
    i := 0;
    for (; i < len partitions; i++)
        parts[i] = partitions[i];

    while (new_partitions != nil) {
        parts[i] = hd new_partitions;
        new_partitions = tl new_partitions;
        i++;
    }

    partitions = parts;

    return 0;
}

split(s: string): array of string
{
    l: list of string;
    while (s != nil) {
        i: string;
        (i, s) = str->splitstrl(s, " ");

        if (i != nil)
            l = str->append(i, l);

        if (s != nil)
            s = s[1:];
    }

    a := array[len l] of string;
    i := 0;
    while (l != nil) {
        a[i] = hd l;
        l = tl l;
        i++;
    }

    return a;
}
