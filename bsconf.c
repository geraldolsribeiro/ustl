/* This file is part of bsconf - a configure replacement.
** Copyright (c) 2003 by Mike Sharov <msharov@talentg.com>
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Library General Public
** License as published by the Free Software Foundation; either
** version 2 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.
**
** You should have received a copy of the GNU Library General Public
** License along with this library; if not, write to the 
** Free Software Foundation, Inc., 59 Temple Place - Suite 330, 
** Boston, MA  02111-1307  USA.
*/

/*
 * This file was written to replace the autoconf-made configure script,
 * which by its prohibitively large size has fallen out of favor with
 * many developers. The configure script performs many time-consuming
 * tests, the results of which are usually unused because no progammer
 * likes to pollute his code with ifdefs. This program performs the
 * program and header lookup and substitutes @CONSTANTS@ in specified
 * files. bsconf.h is used to define which files, programs, and headers
 * to look for. config.h is generated, but the headers are only checked
 * for existence and the functions are assumed to exist (nobody is going
 * to ifdef around every memchr). Most compilers these days have all the
 * headers you want, and if yours does not, there is always gcc. There
 * is simply no excuse for using a braindead compiler when better ones
 * are freely available.
*/

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/utsname.h>

/*--------------------------------------------------------------------*/

#define BUFSIZE		0x10000
#define VectorSize(v)	(sizeof(v) / sizeof(*v))

typedef char string_t [64];

typedef enum {
    vv_prefix,
    vv_exec_prefix,
    vv_bindir,
    vv_sbindir,
    vv_libexecdir,
    vv_datadir,
    vv_sysconfdir,
    vv_sharedstatedir,
    vv_localstatedir,
    vv_libdir,
    vv_includedir,
    vv_oldincludedir,
    vv_infodir,
    vv_mandir,
    vv_build,
    vv_host,
    vv_last
} EVV;

/*--------------------------------------------------------------------*/

static void GetConfigVarValues (int argc, char** argv);
static void FillInDefaultConfigVarValues (void);
static void FindPrograms (void);
static void SubstitutePaths (void);
static void SubstituteEnvironment (int bForce);
static void SubstitutePrograms (void);
static void SubstituteHostOptions (void);
static void SubstituteHeaders (void);
static void SubstituteFunctions (void);
static void SubstituteCustomVars (void);

static void DetermineHost (void);
static void DefaultConfigVarValue (EVV v, EVV root, char* suffix);
static void Substitute (char *matchStr, char *replaceStr);
static void MakeSubstString (char *str, char *substString);
static int  IsBadInstallDir (char* match);

/* Unreliable (according to autoconf) libc stuff */
static int   StrLen (char *str);
static void  Lowercase (char* str);
static int   compare (char *str1, char *str2);
static char* copy (char *src, char *dest);
static char* copy_n (char *src, char *dest, int n);
static char* append (char* src, char* dest);
static void  fill_n (char *str, int n, char v);
static char* copy_backward (char *src, char *dest, int n);
static void  ReadFile (char *filename);
static void  WriteFile (char *filename);
static void  FatalError (char *errortext);

/*--------------------------------------------------------------------*/

static int  g_BufSize;
static char g_Buf [BUFSIZE];

#include "bsconf.h"

static string_t g_ConfigV [vv_last] = {
    "prefix",
    "exec_prefix",
    "bindir",
    "sbindir",
    "libexecdir",
    "datadir",
    "sysconfdir",
    "sharedstatedir",
    "localstatedir",
    "libdir",
    "includedir",
    "oldincludedir",
    "infodir",
    "mandir",
    "build",
    "host"
};

static string_t g_ConfigVV [vv_last];
static string_t g_ProgLocs [VectorSize (g_ProgVars) / 4];

static struct utsname g_Uname;

/*--------------------------------------------------------------------*/

int main (int argc, char** argv)
{
    unsigned int f;
    string_t srcFile;

    GetConfigVarValues (--argc, ++argv);
    FillInDefaultConfigVarValues();
    FindPrograms();

    for (f = 0; f < VectorSize(g_Files); ++ f) {
	copy (g_Files[f], srcFile);
	copy_n (".in", srcFile + StrLen(g_Files[f]), 4);
	ReadFile (srcFile);
	SubstituteHostOptions();
	SubstitutePaths();
	SubstituteEnvironment (0);
	SubstitutePrograms();
	SubstituteHeaders();
	SubstituteFunctions();
	SubstituteCustomVars();
	SubstituteEnvironment (1);
	WriteFile (g_Files[f]);
    }
    return (0);
}

static void PrintHelp (void)
{
    printf (
"`configure' configures " PACKAGE_STRING " to adapt to many kinds of systems.\n"
"\n"
"Usage: configure [OPTION]... \n"
"\n"
"Configuration:\n"
"  --help              display this help and exit\n"
"  --version           display version information and exit\n"
"\n"
"Installation directories:\n"
"  --prefix=PREFIX         install architecture-independent files in PREFIX\n"
"                          [/usr/local]\n"
"  --exec-prefix=EPREFIX   install architecture-dependent files in EPREFIX\n"
"                          [PREFIX]\n"
"\n"
"By default, `make install' will install all the files in\n"
"`/usr/local/bin', `/usr/local/lib' etc.  You can specify\n"
"an installation prefix other than `/usr/local' using `--prefix',\n"
"for instance `--prefix=$HOME'.\n"
"\n"
"For better control, use the options below.\n"
"\n"
"Fine tuning of the installation directories:\n"
"  --bindir=DIR           user executables [EPREFIX/bin]\n"
"  --sbindir=DIR          system admin executables [EPREFIX/sbin]\n"
"  --libexecdir=DIR       program executables [EPREFIX/libexec]\n"
"  --datadir=DIR          read-only architecture-independent data [PREFIX/share]\n"
"  --sysconfdir=DIR       read-only single-machine data [PREFIX/etc]\n"
"  --sharedstatedir=DIR   modifiable architecture-independent data [PREFIX/com]\n"
"  --localstatedir=DIR    modifiable single-machine data [PREFIX/var]\n"
"  --libdir=DIR           object code libraries [EPREFIX/lib]\n"
"  --includedir=DIR       C header files [PREFIX/include]\n"
"  --oldincludedir=DIR    C header files for non-gcc [/usr/include]\n"
"  --infodir=DIR          info documentation [PREFIX/info]\n"
"  --mandir=DIR           man documentation [PREFIX/man]\n"
"\n"
"System types:\n"
"  --build=BUILD     configure for building on BUILD [guessed]\n"
"  --host=HOST       cross-compile to build programs to run on HOST [BUILD]\n"
"\n"
"Some influential environment variables:\n"
"  CXX         C++ compiler command\n"
"  CXXFLAGS    C++ compiler flags\n"
"  LDFLAGS     linker flags, e.g. -L<lib dir> if you have libraries in a\n"
"              nonstandard directory <lib dir>\n"
"  CPPFLAGS    C/C++ preprocessor flags, e.g. -I<include dir> if you have\n"
"              headers in a nonstandard directory <include dir>\n"
"  CC          C compiler command\n"
"  LD          Linker command\n"
"  CFLAGS      C compiler flags\n"
"  CPP         C preprocessor\n"
"\n"
"Use these variables to override the choices made by `configure' or to help\n"
"it to find libraries and programs with nonstandard names/locations.\n"
"\n"
"Report bugs to " PACKAGE_BUGREPORT ".\n");
}

static void PrintVersion (void)
{
    printf (PACKAGE_NAME " configure " PACKAGE_VERSION "\n"
	    "\nUsing bsconf package version 0.1\n"
	    "Copyright 2003, Mike Sharov <msharov@talentg.com>\n"
	    "This configure script and the bsconf package are free software.\n"
	    "Unlimited permission to copy, distribute, and modify is granted.\n");
}

/*--------------------------------------------------------------------*/

static void GetConfigVarValues (int argc, char** argv)
{
    int a, cv, apos, cvl;
    for (cv = 0; cv < vv_last; ++ cv)
	fill_n (g_ConfigVV[cv], sizeof(string_t), 0);
    /* --var=VALUE */
    for (a = 0; a < argc; ++ a) {
	apos = 2;
	if (compare (argv[a], "--"))
	    continue;
	if (!compare (argv[a] + apos, "help"))
	    PrintHelp();
	if (!compare (argv[a] + apos, "version"))
	    PrintVersion();
	for (cv = 0; cv < vv_last; ++ cv)
	    if (!compare (argv[a] + apos, g_ConfigV[cv]))
		break;
	if (cv == vv_last)
	    continue;
	apos += StrLen (g_ConfigV[cv]) + 1;
	cvl = StrLen (argv[a]) - apos;
	if (cvl > 0)
	    copy_n (argv[a] + apos, g_ConfigVV[cv], cvl + 1);
    }
}

static void DefaultConfigVarValue (EVV v, EVV root, char* suffix)
{
    if (!*(g_ConfigVV [v])) {
	copy (g_ConfigVV [root], g_ConfigVV [v]);
	append (suffix, g_ConfigVV [v]);
    }
}

static void FillInDefaultConfigVarValues (void)
{
    if (!*(g_ConfigVV [vv_prefix]))
	copy ("/usr/local", g_ConfigVV [vv_prefix]);
    else if (g_ConfigVV [vv_prefix][0] == '/' && !g_ConfigVV [vv_prefix][1])
	g_ConfigVV [vv_prefix][0] = 0;
    if (!*(g_ConfigVV [vv_exec_prefix]))
	DefaultConfigVarValue (vv_exec_prefix,	vv_prefix,	"");
    else if (g_ConfigVV [vv_exec_prefix][0] == '/' && !g_ConfigVV [vv_exec_prefix][1])
	g_ConfigVV [vv_exec_prefix][0] = 0;
    if (!*(g_ConfigVV [vv_oldincludedir]))
	copy ("/usr/include", g_ConfigVV [vv_oldincludedir]);

    DefaultConfigVarValue (vv_bindir,		vv_exec_prefix,	"/bin");
    DefaultConfigVarValue (vv_sbindir,		vv_exec_prefix,	"/sbin");
    DefaultConfigVarValue (vv_libexecdir,	vv_prefix,	"/libexec");
    DefaultConfigVarValue (vv_datadir,		vv_prefix,	"/share");
    DefaultConfigVarValue (vv_sysconfdir,	vv_prefix,	"/etc");
    DefaultConfigVarValue (vv_sharedstatedir,	vv_prefix,	"/com");
    DefaultConfigVarValue (vv_localstatedir,	vv_prefix,	"/var");
    DefaultConfigVarValue (vv_libdir,		vv_exec_prefix,	"/lib");
    DefaultConfigVarValue (vv_includedir,	vv_prefix,	"/include");
    DefaultConfigVarValue (vv_infodir,		vv_prefix,	"/info");
    DefaultConfigVarValue (vv_mandir,		vv_prefix,	"/man");

    if (!*(g_ConfigVV [vv_prefix]))
	copy ("/", g_ConfigVV [vv_prefix]);
    if (!*(g_ConfigVV [vv_exec_prefix]))
	copy ("/", g_ConfigVV [vv_exec_prefix]);

    if (!*(g_ConfigVV [vv_host]))
	DetermineHost();
    if (!*(g_ConfigVV [vv_build]))
	copy (g_ConfigVV [vv_host], g_ConfigVV [vv_build]);
}

static void DetermineHost (void)
{
    fill_n ((char*) &g_Uname, sizeof(struct utsname), 0);
    uname (&g_Uname);
    Lowercase (g_Uname.machine);
    Lowercase (g_Uname.sysname);
    copy (g_Uname.machine, g_ConfigVV [vv_host]);
    append ("-", g_ConfigVV [vv_host]);
#ifdef __GNUC__
    append ("gnu", g_ConfigVV [vv_host]);
#else
    append ("unknown", g_ConfigVV [vv_host]);
#endif
    append ("-", g_ConfigVV [vv_host]);
    append (g_Uname.sysname, g_ConfigVV [vv_host]);
}

static char* CopyPathEntry (char* pi, char* dest)
{
    while (*pi && *pi != ':')
	*dest++ = *pi++;
    *dest = 0;
    return (*pi ? ++pi : NULL);
}

static int IsBadInstallDir (char* match)
{
    return (!compare (match, "/etc/") ||
	    !compare (match, "/usr/sbin/") ||
	    !compare (match, "/c/") ||
	    !compare (match, "/C/") ||
	    !compare (match, "/usr/etc/") ||
	    !compare (match, "/sbin/") ||
	    !compare (match, "/usr/ucb/") ||
	    !compare (match, "/usr/afsws/bin/"));
}

static void FindPrograms (void)
{
    unsigned int i, count;
    char *path, *pi;
    string_t match;

    path = getenv ("PATH");
    if (!path)
	path = "";

    for (i = 0; i < VectorSize(g_ProgLocs); ++ i) {
	fill_n (g_ProgLocs[i], sizeof(string_t), 0);
	fill_n (match, sizeof(string_t), 0);
	count = 0;
	for (pi = path; pi; pi = CopyPathEntry (pi, match)) {
	    append ("/", match);
	    /* Ignore "bad" versions of install, like autoconf does. */
	    if (!compare (g_ProgVars[i * 4 + 1], "install") && IsBadInstallDir (match))
		continue;
	    append (g_ProgVars[i * 4 + 1], match);
	    if (access (match, X_OK) == 0) {
		++ count;
		break;
	    }
	}
	if (count && !compare (g_ProgVars[i * 4 + 1], "install"))
	    copy (match, g_ProgLocs[i]);
	else
	    copy (g_ProgVars[i * 4 + 2 + !count], g_ProgLocs[i]);
    }
}

static void SubstitutePaths (void)
{
    string_t match;
    int cv;
    for (cv = 0; cv < vv_last; ++ cv) {
	MakeSubstString (g_ConfigV [cv], match);
	Substitute (match, g_ConfigVV [cv]);
    }
}

static void SubstituteEnvironment (int bForce)
{
    string_t match;
    unsigned int i;
    char* envval;

    for (i = 0; i < VectorSize(g_EnvVars); ++ i) {
	envval = getenv (g_EnvVars[i]);
	if (!envval) {
	    if (!bForce)
		continue;
	    envval = "";
	}
	MakeSubstString (g_EnvVars[i], match);
	Substitute (match, envval);
    }
}

static void SubstitutePrograms (void)
{
    unsigned int i;
    string_t match;
    for (i = 0; i < VectorSize(g_ProgLocs); ++ i) {
	MakeSubstString (g_ProgVars [i * 4], match);
	Substitute (match, g_ProgLocs [i]);
    }
}

static void SubstituteHostOptions (void)
{
    if (!compare (g_Uname.sysname, "osx") ||
	!compare (g_Uname.sysname, "darwin"))
	Substitute ("@SYSWARNS@", "-Wno-long-double");
    else if (!compare (g_Uname.sysname, "sun") ||
	     !compare (g_Uname.sysname, "solaris"))
	Substitute ("@SYSWARNS@", "-Wno-redundant-decls");
    else
	Substitute ("@SYSWARNS@", "");

    if (!compare (g_Uname.sysname, "linux"))
	Substitute ("@BUILD_SHARED_LIBRARIES@", "MAJOR\t\t= @LIB_MAJOR@\nMINOR\t\t= @LIB_MINOR@\nBUILD\t\t= @LIB_BUILD@");
    else
	Substitute ("@BUILD_SHARED_LIBRARIES@\n", "");

    Substitute ("#undef RETSIGTYPE", "#define RETSIGTYPE void");
    Substitute ("#undef const", "/* #define const */");
    Substitute ("#undef inline", "/* #define inline __inline */");
    Substitute ("#undef off_t", "/* typedef long off_t; */");
    Substitute ("#undef size_t", "/* typedef long size_t; */");

    Substitute ("#undef LSTAT_FOLLOWS_SLASHED_SYMLINK", "#define LSTAT_FOLLOWS_SLASHED_SYMLINK 1");
    Substitute ("#undef HAVE_STAT_EMPTY_STRING_BUG", "/* #undef HAVE_STAT_EMPTY_STRING_BUG */");

    Substitute ("#undef PACKAGE_BUGREPORT",	"#define PACKAGE_BUGREPORT \"" PACKAGE_BUGREPORT "\"");
    Substitute ("#undef PACKAGE_NAME",		"#define PACKAGE_NAME \"" PACKAGE_NAME "\"");
    Substitute ("#undef PACKAGE_STRING",	"#define PACKAGE_STRING \"" PACKAGE_STRING "\"");
    Substitute ("#undef PACKAGE_TARNAME",	"#define PACKAGE_TARNAME \"" PACKAGE_TARNAME "\"");
    Substitute ("#undef PACKAGE_VERSION",	"#define PACKAGE_VERSION \"" PACKAGE_VERSION "\"");
}

static void SubstituteCustomVars (void)
{
    unsigned int i;
    string_t match;
    for (i = 0; i < VectorSize(g_CustomVars); ++ i) {
	MakeSubstString (g_CustomVars [i * 2], match);
	Substitute (match, g_CustomVars [i * 2 + 1]);
    }
}

static void SubstituteHeaders (void)
{
    unsigned int i, j;
    string_t match, paths [3];

    copy (g_ConfigVV [vv_includedir], paths[0]);
    copy (g_ConfigVV [vv_oldincludedir], paths[1]);
    copy ("/usr/lib/gcc-lib/i386-redhat-linux/3.2/include", paths[2]);	/* yeah, yeah... */

    for (i = 0; i < VectorSize(g_Headers) / 3; ++ i) {
	for (j = 0; j < VectorSize(paths); ++ j) {
	    copy (paths[j], match);
	    append ("/", match);
	    append (g_Headers [i * 3], match);
	    if (access (match, R_OK) == 0)
		Substitute (g_Headers [i * 3 + 1], g_Headers [i * 3 + 2]);
	}
    }
}

static void SubstituteFunctions (void)
{
    unsigned int i;
    for (i = 0; i < VectorSize(g_Functions) / 3; ++ i)
	Substitute (g_Functions [i * 3 + 1], g_Functions [i * 3 + 2]);
}

/*--------------------------------------------------------------------*/

static void Substitute (char* matchStr, char* replaceStr)
{
    int rsl = StrLen (replaceStr);
    int taill, delta = rsl - StrLen (matchStr);
    char *cp = g_Buf;
    for (; cp < g_Buf + g_BufSize; ++ cp) {
	if (compare (cp, matchStr))
	    continue;
	if (g_BufSize + delta >= BUFSIZE)
	    FatalError ("buffer overflow");
	g_BufSize += delta;
	taill = g_BufSize - (cp - g_Buf);
	if (delta > 0)
	    copy_backward (cp, cp + delta, taill);
	else if (delta < 0)
	    copy_n (cp + (-delta), cp, taill);
	cp = copy_n (replaceStr, cp, rsl);
    }
}

static void MakeSubstString (char* str, char* substString)
{
    copy ("@", substString);
    append (str, substString);
    append ("@", substString);
}

/*--------------------------------------------------------------------*/

static void FatalError (char* errortext)
{
    perror (errortext);
    exit(-1);
}

static int StrLen (char* str)
{
    int l;
    for (l = 0; *str; ++ l, ++ str);
    return (l);
}

static void Lowercase (char* str)
{
    for (; *str; ++ str)
	if (*str >= 'A' && *str <= 'Z')
	    *str += 'a' - 'A';
}

static int compare (char* str1, char* str2)
{
    while (*str1 && *str2 && *str1 == *str2)
	++ str1, ++ str2;
    return (*str2);
}

static char* copy (char* src, char* dest)
{
    while (*src) *dest++ = *src++;
    *dest = 0;
    return (dest);
}

static char* copy_n (char* src, char* dest, int n)
{
    while (n--)
	*dest++ = *src++;
    return (dest);
}

static char* append (char* src, char* dest)
{
    while (*dest) ++ dest;
    return (copy (src, dest));
}

static void fill_n (char* str, int n, char v)
{
    while (n--)
	*str++ = v;
}

static char* copy_backward (char* src, char* dest, int n)
{
    dest += n; src += n;
    while (n--)
	*--dest = *--src;
    return (dest + n);
}

static void ReadFile (char* filename)
{
    FILE* fp = fopen (filename, "r");
    if (!fp)
	FatalError ("fopen");
    fill_n (g_Buf, BUFSIZE, 0);
    g_BufSize = fread (g_Buf, 1, BUFSIZE - 1, fp);
    if (g_BufSize <= 0)
	FatalError ("fread");
    fclose (fp);
}

static void WriteFile (char* filename)
{
    int bw;
    FILE* fp = fopen (filename, "w");
    if (!fp)
	FatalError ("fopen");
    bw = fwrite (g_Buf, 1, g_BufSize, fp);
    if (bw != g_BufSize)
	FatalError ("fwrite");
    if (fclose (fp))
	FatalError ("fclose");
}
