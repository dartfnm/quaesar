// clang-format off
#include "sysconfig.h"
#include "sysdeps.h"
#include "options.h"
#include "memory.h"

#include "fsusage.h"
#include "fsdb.h"
#include "zfile.h"
// clang-format on

#include <SDL_log.h>
#include <stdio.h>
#include <iostream>
#include <filesystem>

#ifdef WIN32
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")
#endif // WIN32

#undef min

static int fsdb_debug = 0;


#ifdef TRACE
#undef TRACE
#endif
#define TRACE() SDL_Log("WARN: Using of unimplemented function: '%s()'", __func__)
#define debug(x, ...) SDL_Log("[%s()] " x, __func__, __VA_ARGS__)


/* these are deadly (but I think allowed on the Amiga): */
#define NUM_EVILCHARS 7
static TCHAR evilchars[NUM_EVILCHARS] = {'\\', '*', '?', '\"', '<', '>', '|'};
#define PATHPREFIX _T("\\\\?\\")
#define UAEFSDB2_LEN 1632
#define UAEFSDB_LEN 604

TCHAR start_path_data[MAX_DPATH];


int pissoff_value = 15000 * CYCLE_UNIT;
int pause_emulation = 0;

int my_existsdir(const TCHAR* directoryPath) {
    debug("dir: '%s'", directoryPath);
    struct stat st;

    if (stat(directoryPath, &st) == 0) {
        if (S_ISDIR(st.st_mode)) {
            return 1;
        }
    }

    return 0;
}

#ifdef _WIN32

#include <Windows.h>

int my_existsfile(const TCHAR* name) {
    debug("name: '%s'", name);
    DWORD attr;
    attr = GetFileAttributes(name);
    if (attr == INVALID_FILE_ATTRIBUTES)
        return 0;
    if (!(attr & FILE_ATTRIBUTE_DIRECTORY))
        return 1;
    return 0;
}

#else

int my_existsfile(const TCHAR* name) {
    if (access(name, F_OK) != -1) {
        return 1;
    } else {
        return 0;
    }
}

#endif


bool my_chmod(char const*, unsigned int) {
    UNIMPLEMENTED();
    return false;
}


int my_getvolumeinfo(char const* root) {
    debug("root: %s", root);
    std::filesystem::path filePath = root;

    if (!std::filesystem::exists(filePath))
        return -1;
    if (!std::filesystem::is_directory(filePath))
        return -2;
    int ret = 0;
    std::filesystem::perms perms = std::filesystem::status(filePath).permissions();
    if ((perms & std::filesystem::perms::owner_write) == std::filesystem::perms::none)
        ret |= MYVOLUMEINFO_READONLY;

#ifdef WINDOWS
    DWORD v, err;
    TCHAR volume[MAX_DPATH];

    if (GetVolumePathName(root, volume, sizeof(volume) / sizeof(TCHAR))) {
        TCHAR fsname[MAX_DPATH];
        DWORD comlen;
        DWORD flags;
        if (GetVolumeInformation(volume, NULL, 0, NULL, &comlen, &flags, fsname, sizeof fsname / sizeof(TCHAR))) {
            // write_log (_T("Volume %s FS=%s maxlen=%d flags=%08X\n"), volume, fsname, comlen, flags);
            if (flags & FILE_NAMED_STREAMS)
                ret |= MYVOLUMEINFO_STREAMS;
        }
    }
#endif // WINDOWS

    return ret;
}


int my_rmdir(char const*) {
    UNIMPLEMENTED();
    return 0;
}

void getpathpart(char*, int, char const*) {
    UNIMPLEMENTED();
}

int my_unlink(const TCHAR* /*name*/, bool /*dontrecycle*/) {
    UNIMPLEMENTED();
    return 0;
}


int get_fs_usage(const TCHAR* path, const TCHAR* disk, struct fs_usage* fsp) {
    SDL_Log("Warn using of unimplemented function: `%s`", __FUNCTION__);
    fsp->total = 1000000;
    fsp->avail = 1000000;
    return 0;
}


int isprinter() {
    return 0;
}

void to_lower(TCHAR* s, int len) {
    for (int i = 0; i < len; i++) {
        s[i] = tolower(s[i]);
    }
}

TCHAR* utf8u(const char* s) {
    if (s == NULL)
        return NULL;
    return ua(s);
}

char* uutf8(const TCHAR* s) {
    if (s == NULL)
        return NULL;
    return ua(s);
}

/*
TCHAR *au_copy (TCHAR *dst, int maxlen, const char *src)
{
    // this should match the WinUAE au_copy behavior, where either the
    // entire string is copied (and null-terminated), or the result is
    // an empty string
    if (uae_tcslcpy (dst, src, maxlen) >= maxlen) {
        dst[0] = '\0';
    }
    return dst;
}

char *ua_copy (char *dst, int maxlen, const TCHAR *src)
{
    return au_copy (dst, maxlen, src);
}
*/

TCHAR* my_strdup_ansi(const char* src) {
    return strdup(src);
}

#define NO_TRANSLATION

TCHAR* au_fs(const char* src) {
#ifdef NO_TRANSLATION
    if (src == NULL)
        return NULL;
    return strdup(src);
#else
    gsize read, written;
    gchar* result = g_convert(src, -1, "UTF-8", "ISO-8859-1", &read, &written, NULL);
    if (result == NULL) {
        write_log("WARNING: au_fs_copy failed to convert string %s", src);
        return strdup("");
    }
    return result;
#endif
}

char* ua_fs(const TCHAR* s, int defchar) {
#ifdef NO_TRANSLATION
    if (s == NULL)
        return NULL;
    return strdup(s);
#else
    // we convert from fs-uae's internal encoding (UTF-8) to latin-1 here,
    // so file names can be read properly in the amiga

    char def[] = "?";
    if (defchar < 128) {
        def[0] = defchar;
    }

    gsize read, written;
    gchar* result = g_convert_with_fallback(s, -1, "ISO-8859-1", "UTF-8", def, &read, &written, NULL);
    if (result == NULL) {
        write_log("WARNING: ua_fs failed to convert string %s", s);
        return strdup("");
    }

    // duplicate with libc malloc
    char* result_malloced = strdup(result);
    free(result);
    return result_malloced;
#endif
}

TCHAR* au_fs_copy(TCHAR* dst, int maxlen, const char* src) {
#ifdef NO_TRANSLATION
    dst[0] = 0;
    strncpy(dst, src, maxlen);
    return dst;
#else
    gsize read, written;
    gchar* result = g_convert(src, -1, "UTF-8", "ISO-8859-1", &read, &written, NULL);
    if (result == NULL) {
        write_log("WARNING: au_fs_copy failed to convert string %s", src);
        dst[0] = '\0';
        return dst;
    }

    strncpy(dst, result, maxlen);
    free(result);
    return dst;
#endif
}

char* ua_fs_copy(char* dst, int maxlen, const TCHAR* src, int defchar) {
#ifdef NO_TRANSLATION
    dst[0] = 0;
    strncpy(dst, src, maxlen);
    return dst;
#else
    char def[] = "?";
    if (defchar < 128) {
        def[0] = defchar;
    }

    gsize read, written;
    gchar* result = g_convert_with_fallback(src, -1, "ISO-8859-1", "UTF-8", def, &read, &written, NULL);
    if (result == NULL) {
        write_log("WARNING: ua_fs_copy failed to convert string %s", src);
        dst[0] = '\0';
        return dst;
    }

    strncpy(dst, result, maxlen);
    free(result);
    return dst;
#endif
}

TCHAR* target_expand_environment(const TCHAR* path, TCHAR* out, int maxlen) {
    debug("path:'%s'", path);
    if (!path)
        return NULL;
    if (out == NULL) {
        return strdup(path);
    } else {
        _tcscpy(out, path);
        return out;
    }
}

typedef BOOL(CALLBACK* GETVOLUMEINFORMATIONBYHANDLEW)(_In_ HANDLE hFile, LPWSTR lpVolumeNameBuffer,
                                                      DWORD nVolumeNameSize, LPDWORD lpVolumeSerialNumber,
                                                      LPDWORD lpMaximumComponentLength, LPDWORD lpFileSystemFlags,
                                                      LPTSTR lpFileSystemNameBuffer, DWORD nFileSystemNameSize);


static bool isfat(HANDLE h) {
    TCHAR fsname[MAX_DPATH];
    DWORD comlen;
    DWORD flags;
    GETVOLUMEINFORMATIONBYHANDLEW pGetVolumeInformationByHandleW;

    pGetVolumeInformationByHandleW = (GETVOLUMEINFORMATIONBYHANDLEW)GetProcAddress(GetModuleHandle(_T("kernel32.dll")),
                                                                                   "GetVolumeInformationByHandleW");
    if (!pGetVolumeInformationByHandleW)
        return false;
    if (pGetVolumeInformationByHandleW(h, NULL, NULL, NULL, &comlen, &flags, fsname, sizeof fsname / sizeof(TCHAR))) {
        if (!_tcsncmp(fsname, _T("FAT"), 3)) {
            return true;
        }
    }
    return false;
}


bool my_stat(const TCHAR* name, struct mystat* statbuf) {
    DWORD attr, ok;
    FILETIME ft, lft;
    HANDLE h;
    BY_HANDLE_FILE_INFORMATION fi;
    const TCHAR* namep;
    bool fat;
    TCHAR path[MAX_DPATH];

    if (currprefs.win32_filesystem_mangle_reserved_names == false) {
        _tcscpy(path, PATHPREFIX);
        _tcscat(path, name);
        namep = path;
    } else {
        namep = name;
    }

    // FILE_FLAG_BACKUP_SEMANTICS = can also "open" directories
    h = CreateFile(namep, 0, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING,
                   FILE_ATTRIBUTE_NORMAL | FILE_FLAG_BACKUP_SEMANTICS, NULL);
    if (h == INVALID_HANDLE_VALUE)
        return false;
    fat = isfat(h);

    ok = GetFileInformationByHandle(h, &fi);
    CloseHandle(h);

    attr = 0;
    ft.dwHighDateTime = ft.dwLowDateTime = 0;
    if (ok) {
        attr = fi.dwFileAttributes;
        if (fat) {
            // fat lastwritetime only has 2 second resolution
            // fat creationtime has 10ms resolution
            // use creationtime if creationtime is inside lastwritetime 2s resolution
            ULARGE_INTEGER ct, wt;
            ct.HighPart = fi.ftCreationTime.dwHighDateTime;
            ct.LowPart = fi.ftCreationTime.dwLowDateTime;
            wt.HighPart = fi.ftLastWriteTime.dwHighDateTime;
            wt.LowPart = fi.ftLastWriteTime.dwLowDateTime;
            uae_u64 ctsec = ct.QuadPart / 10000000;
            uae_u64 wtsec = wt.QuadPart / 10000000;
            if (wtsec == ctsec || wtsec + 1 == ctsec) {
                ft = fi.ftCreationTime;
            } else {
                ft = fi.ftLastWriteTime;
            }
        } else {
            ft = fi.ftLastWriteTime;
        }
        statbuf->size = ((uae_u64)fi.nFileSizeHigh << 32) | fi.nFileSizeLow;
    } else {
        write_log(_T("GetFileInformationByHandle(%s) failed: %d\n"), namep, GetLastError());
        return false;
    }

    statbuf->mode = (attr & FILE_ATTRIBUTE_READONLY) ? FILEFLAG_READ : FILEFLAG_READ | FILEFLAG_WRITE;
    if (attr & FILE_ATTRIBUTE_ARCHIVE)
        statbuf->mode |= FILEFLAG_ARCHIVE;
    if (attr & FILE_ATTRIBUTE_DIRECTORY)
        statbuf->mode |= FILEFLAG_DIR;

    FileTimeToLocalFileTime(&ft, &lft);
    uae_u64 t = (*(__int64*)&lft - ((__int64)(369 * 365 + 89) * (__int64)(24 * 60 * 60) * (__int64)10000000));
    statbuf->mtime.tv_sec = t / 10000000;
    statbuf->mtime.tv_usec = (t / 10) % 1000000;

    return true;
}


struct my_opendir_s {
    HANDLE h;
    WIN32_FIND_DATA fd;
    int first;
};


int my_readdir(struct my_opendir_s* mod, TCHAR* name) {
    if (mod->first) {
        _tcscpy(name, mod->fd.cFileName);
        mod->first = 0;
        return 1;
    }
    if (!FindNextFile(mod->h, &mod->fd))
        return 0;
    _tcscpy(name, mod->fd.cFileName);
    return 1;
}




struct my_opendir_s* my_opendir(const TCHAR* name, const TCHAR* mask) {
    struct my_opendir_s* mod;
    TCHAR tmp[MAX_DPATH];

    tmp[0] = 0;
    if (currprefs.win32_filesystem_mangle_reserved_names == false)
        _tcscpy(tmp, PATHPREFIX);
    _tcscat(tmp, name);
    _tcscat(tmp, _T("\\"));
    _tcscat(tmp, mask);
    mod = xmalloc(struct my_opendir_s, 1);
    if (!mod)
        return NULL;
    mod->h = FindFirstFile(tmp, &mod->fd);
    if (mod->h == INVALID_HANDLE_VALUE) {
        xfree(mod);
        return NULL;
    }
    mod->first = 1;
    return mod;
}

struct my_opendir_s* my_opendir(const TCHAR* name) {
    return my_opendir(name, _T("*.*"));
}


void my_closedir(struct my_opendir_s* mod) {
    if (mod)
        FindClose(mod->h);
    xfree(mod);
}
/*
int hdf_write_target(struct hardfiledata* hfd, void* buffer, uae_u64 offset, int len) {
    UNIMPLEMENTED();
    return 0;
}
*/

int hdf_write_target(struct hardfiledata* hfd, void* buffer, uae_u64 offset, int len, uint32_t* error) {
    UNIMPLEMENTED();
    return 0;
}


struct a_inode_struct;

int fsdb_set_file_attrs(a_inode_struct* aino) {
    UNIMPLEMENTED();
    return 0;
}

void fetch_nvrampath(TCHAR* out, int size) {
    UNIMPLEMENTED();
}

void fetch_configurationpath(TCHAR* out, int size) {
    debug("out:'%s'", out);
    out[0] = _T('/');
    out[1] = _T('.');
    out[2] = 0;
}



/* Return nonzero for any name we can't create on the native filesystem.  */
static int fsdb_name_invalid_2x(const TCHAR* n, int dir) {
    size_t i;
    static char s1[MAX_DPATH];
    static TCHAR s2[MAX_DPATH];
    TCHAR a = n[0];
    TCHAR b = (a == '\0' ? a : n[1]);
    TCHAR c = (b == '\0' ? b : n[2]);
    TCHAR d = (c == '\0' ? c : n[3]);
    size_t l = _tcslen(n);
    int ll;

    /* the reserved fsdb filename */
    if (_tcscmp(n, FSDB_FILE) == 0)
        return -1;

    if (dir) {
        if (n[0] == '.' && l == 1)
            return -1;
        if (n[0] == '.' && n[1] == '.' && l == 2)
            return -1;
    }

    if (a >= 'a' && a <= 'z')
        a -= 32;
    if (b >= 'a' && b <= 'z')
        b -= 32;
    if (c >= 'a' && c <= 'z')
        c -= 32;

    s1[0] = 0;
    s2[0] = 0;
    ua_fs_copy(s1, MAX_DPATH, n, -1);
    au_fs_copy(s2, MAX_DPATH, s1);
    if (_tcscmp(s2, n) != 0)
        return 1;

    if (currprefs.win32_filesystem_mangle_reserved_names) {
        /* reserved dos devices */
        ll = 0;
        if (a == 'A' && b == 'U' && c == 'X')
            ll = 3; /* AUX  */
        if (a == 'C' && b == 'O' && c == 'N')
            ll = 3; /* CON  */
        if (a == 'P' && b == 'R' && c == 'N')
            ll = 3; /* PRN  */
        if (a == 'N' && b == 'U' && c == 'L')
            ll = 3; /* NUL  */
        if (a == 'L' && b == 'P' && c == 'T' && (d >= '0' && d <= '9'))
            ll = 4; /* LPT# */
        if (a == 'C' && b == 'O' && c == 'M' && (d >= '0' && d <= '9'))
            ll = 4; /* COM# */
        /* AUX.anything, CON.anything etc.. are also illegal names */
        if (ll && (l == ll || (l > ll && n[ll] == '.')))
            return 3;

        /* spaces and periods at the end are a no-no */
        i = l - 1;
        if (n[i] == '.' || n[i] == ' ')
            return 1;
    }

    /* these characters are *never* allowed */
    for (i = 0; i < NUM_EVILCHARS; i++) {
        if (_tcschr(n, evilchars[i]) != 0)
            return 2;
    }

    return 0; /* the filename passed all checks, now it should be ok */
}


static int fsdb_name_invalid_2(a_inode* aino, const TCHAR* n, int dir) {
    int v = fsdb_name_invalid_2x(n, dir);
    if (v <= 1 || !aino)
        return v;
    TCHAR* p = build_nname(aino->nname, n);
    HANDLE h = CreateFile(p, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                          FILE_ATTRIBUTE_NORMAL | FILE_FLAG_BACKUP_SEMANTICS, NULL);
    DWORD err = -1;
    DWORD type = -1;
    if (h != INVALID_HANDLE_VALUE) {
        type = GetFileType(h);
        CloseHandle(h);
    } else {
        err = GetLastError();
    }
    write_log(_T("H=%p TYPE=%08x ERR=%08X '%s'\n"), h, type, err, p);
    xfree(p);
    if (h != INVALID_HANDLE_VALUE && type != FILE_TYPE_DISK)
        return 1;
    if (err == ERROR_INVALID_NAME || err == ERROR_ACCESS_DENIED || err == ERROR_INVALID_HANDLE)
        return 1;
    if (currprefs.win32_filesystem_mangle_reserved_names && err == ERROR_FILE_NOT_FOUND)
        return 1;
    return 0;
}


int fsdb_name_invalid_dir(a_inode* aino, const TCHAR* n) {
    int v = fsdb_name_invalid_2(aino, n, 1);
    if (v <= 0)
        return v;
    write_log(_T("FILESYS: '%s' illegal filename\n"), n);
    return v;
}


int fsdb_mode_supported(const a_inode*) {
    UNIMPLEMENTED();
    return 0;
}


TCHAR* fsdb_create_unique_nname(a_inode_struct*, char const*) {
    UNIMPLEMENTED();
    return nullptr;
}

int fsdb_mode_representable_p(a_inode_struct const*, int) {
    UNIMPLEMENTED();
    return 0;
}

int fsdb_name_invalid(a_inode_struct*, char const*) {
    UNIMPLEMENTED();
    return 0;
}


DWORD GetFileAttributesSafe(const TCHAR* name) {
    DWORD attr, last;
    const TCHAR* namep;
    TCHAR path[MAX_DPATH];

    last = SetErrorMode(SEM_FAILCRITICALERRORS);
    if (currprefs.win32_filesystem_mangle_reserved_names == false) {
        _tcscpy(path, PATHPREFIX);
        _tcscat(path, name);
        namep = path;
    } else {
        namep = name;
    }
    attr = GetFileAttributes(namep);
    SetErrorMode(last);
    return attr;
}


BOOL SetFileAttributesSafe(const TCHAR* name, DWORD attr) {
    DWORD last;
    BOOL ret;
    const TCHAR* namep;
    TCHAR path[MAX_DPATH];

    last = SetErrorMode(SEM_FAILCRITICALERRORS);
    if (currprefs.win32_filesystem_mangle_reserved_names == false) {
        _tcscpy(path, PATHPREFIX);
        _tcscat(path, name);
        namep = path;
    } else {
        namep = name;
    }
    ret = SetFileAttributes(namep, attr);
    SetErrorMode(last);
    return ret;
}


void my_canonicalize_path(const TCHAR* path, TCHAR* out, int size) {
    TCHAR tmp[MAX_DPATH];
    int v;

    // don't attempt to validate and canonicalize invalid or fake paths
    if (path[0] == ':' || path[0] == 0 || _tcscmp(path, _T("\\")) == 0 || _tcscmp(path, _T("/")) == 0) {
        _tcsncpy(out, path, size);
        out[size - 1] = 0;
        return;
    }
    // skip network paths, prevent common few seconds delay.
    if (path[0] == '\\' && path[1] == '\\') {
        _tcsncpy(out, path, size);
        out[size - 1] = 0;
        return;
    }
    v = GetLongPathName(path, tmp, sizeof tmp / sizeof(TCHAR));
    if (v > sizeof tmp / sizeof(TCHAR)) {
        _tcsncpy(out, path, size);
        out[size - 1] = 0;
        return;
    }
    if (!v)
        _tcscpy(tmp, path);
    GetFullPathName(tmp, size, out, NULL);
}


typedef struct _REPARSE_DATA_BUFFER {
    ULONG ReparseTag;
    USHORT ReparseDataLength;
    USHORT Reserved;
    union {
        struct {
            USHORT SubstituteNameOffset;
            USHORT SubstituteNameLength;
            USHORT PrintNameOffset;
            USHORT PrintNameLength;
            ULONG Flags;
            WCHAR PathBuffer[1];
        } SymbolicLinkReparseBuffer;
        struct {
            USHORT SubstituteNameOffset;
            USHORT SubstituteNameLength;
            USHORT PrintNameOffset;
            USHORT PrintNameLength;
            WCHAR PathBuffer[1];
        } MountPointReparseBuffer;
        struct {
            UCHAR DataBuffer[1];
        } GenericReparseBuffer;
    };
} REPARSE_DATA_BUFFER, *PREPARSE_DATA_BUFFER;


static int my_resolvessymboliclink2(TCHAR* linkfile, int size) {
    WIN32_FIND_DATA fd;
    HANDLE h;
    int ret = -1;
    DWORD returnedDataSize;
    uae_u8 tmp[MAX_DPATH * 2];
    TCHAR tmp2[MAX_DPATH];

    h = FindFirstFile(linkfile, &fd);
    if (h == INVALID_HANDLE_VALUE)
        return -1;
    FindClose(h);
    if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) || fd.dwReserved0 != IO_REPARSE_TAG_SYMLINK)
        return -1;
    h = CreateFile(linkfile, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING,
                   FILE_FLAG_BACKUP_SEMANTICS | FILE_ATTRIBUTE_REPARSE_POINT | FILE_FLAG_OPEN_REPARSE_POINT, NULL);
    if (h == INVALID_HANDLE_VALUE)
        return 0;
    if (DeviceIoControl(h, FSCTL_GET_REPARSE_POINT, NULL, 0, tmp, sizeof tmp, &returnedDataSize, NULL)) {
        REPARSE_DATA_BUFFER* rdb = (REPARSE_DATA_BUFFER*)tmp;
        if (size > 0) {
            if (rdb->SymbolicLinkReparseBuffer.Flags & 1) {  // SYMLINK_FLAG_RELATIVE
                _tcscpy(tmp2, linkfile);
                PathRemoveFileSpec(tmp2);
                _tcscat(tmp2, _T("\\"));
                TCHAR* p = tmp2 + _tcslen(tmp2);
                memcpy(p,
                       (uae_u8*)rdb->SymbolicLinkReparseBuffer.PathBuffer +
                           rdb->SymbolicLinkReparseBuffer.SubstituteNameOffset,
                       rdb->SymbolicLinkReparseBuffer.SubstituteNameLength);
                p[rdb->SymbolicLinkReparseBuffer.SubstituteNameLength / 2] = 0;
            } else {
                memcpy(tmp2,
                       (uae_u8*)rdb->SymbolicLinkReparseBuffer.PathBuffer +
                           rdb->SymbolicLinkReparseBuffer.SubstituteNameOffset,
                       rdb->SymbolicLinkReparseBuffer.SubstituteNameLength);
                tmp2[rdb->SymbolicLinkReparseBuffer.SubstituteNameLength / 2] = 0;
            }
            if (!_tcsnicmp(tmp2, _T("\\??\\"), 4)) {
                memmove(tmp2, tmp2 + 4, (_tcslen(tmp2 + 4) + 1) * sizeof(TCHAR));
            }
            my_canonicalize_path(tmp2, linkfile, size);
        }
        ret = 1;
    }
    CloseHandle(h);
    return ret;
}


bool my_resolvessymboliclink(TCHAR* linkfile, int size) {
    return my_resolvessymboliclink2(linkfile, size) > 0;
}


TCHAR* fsdb_search_dir(const TCHAR* dirname, TCHAR* rel, TCHAR** relalt) {
    WIN32_FIND_DATA fd;
    HANDLE h;
    TCHAR *tmp, *p = 0;
    const TCHAR* namep;
    TCHAR path[MAX_DPATH];

    *relalt = NULL;
    tmp = build_nname(dirname, rel);
    if (!tmp)
        return NULL;
    if (currprefs.win32_filesystem_mangle_reserved_names == false) {
        _tcscpy(path, PATHPREFIX);
        _tcscat(path, tmp);
        namep = path;
    } else {
        namep = tmp;
    }
    h = FindFirstFile(namep, &fd);
    if (h != INVALID_HANDLE_VALUE) {
        if (_tcscmp(fd.cFileName, rel) == 0)
            p = rel;
        else
            p = my_strdup(fd.cFileName);
        FindClose(h);
    } else {
        // check if it is *.lnk shortcut
        TCHAR tmp[MAX_DPATH];
        _tcscpy(tmp, namep);
        _tcscat(tmp, _T(".lnk"));
        DWORD flags = GetFileAttributesSafe(tmp);
        if (flags != INVALID_FILE_ATTRIBUTES && !(flags & FILE_ATTRIBUTE_SYSTEM) &&
            !(flags & FILE_ATTRIBUTE_DIRECTORY)) {
            h = FindFirstFile(tmp, &fd);
            if (h != INVALID_HANDLE_VALUE) {
                TCHAR tmp2[MAX_DPATH];
                _tcscpy(tmp2, tmp);
                if (my_resolvesoftlink(tmp2, sizeof tmp2 / sizeof(TCHAR), false)) {
                    if (_tcslen(fd.cFileName) > 4 && !_tcsicmp(fd.cFileName + _tcslen(fd.cFileName) - 4, _T(".lnk"))) {
                        fd.cFileName[_tcslen(fd.cFileName) - 4] = 0;
                    }
                    if (_tcscmp(fd.cFileName, rel) == 0)
                        p = rel;
                    else
                        p = my_strdup(fd.cFileName);
                    _tcscpy(tmp2, p);
                    _tcscat(tmp2, _T(".lnk"));
                    *relalt = my_strdup(tmp2);
                }
                FindClose(h);
            }
        }
    }
    xfree(tmp);
    return p;
}


int fsdb_exists(char const*) {
    UNIMPLEMENTED();
    return 0;
}



static TCHAR* make_uaefsdbpath(const TCHAR* dir, const TCHAR* name) {
    size_t len;
    TCHAR* p;

    len = _tcslen(dir) + 1 + 1;
    if (name)
        len += 1 + _tcslen(name);
    if (currprefs.win32_filesystem_mangle_reserved_names == false)
        len += _tcslen(PATHPREFIX);
    len += 1 + _tcslen(FSDB_FILE);
    p = xmalloc(TCHAR, len);
    if (!p)
        return NULL;
    if (name)
        _stprintf(p, _T("%s%s\\%s:%s"), currprefs.win32_filesystem_mangle_reserved_names == false ? PATHPREFIX : _T(""),
                  dir, name, FSDB_FILE);
    else
        _stprintf(p, _T("%s%s:%s"), currprefs.win32_filesystem_mangle_reserved_names == false ? PATHPREFIX : _T(""),
                  dir, FSDB_FILE);
    return p;
}


static int read_uaefsdb(const TCHAR* dir, const TCHAR* name, uae_u8* fsdb) {
    TCHAR* p;
    HANDLE h;
    DWORD read;

    read = 0;
    p = make_uaefsdbpath(dir, name);
    h = CreateFile(p, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (fsdb_debug)
        write_log(_T("read_uaefsdb '%s' = %x\n"), p, h);
    xfree(p);
    if (h != INVALID_HANDLE_VALUE) {
        memset(fsdb, 0, UAEFSDB2_LEN);
        ReadFile(h, fsdb, UAEFSDB2_LEN, &read, NULL);
        CloseHandle(h);
        if (read == UAEFSDB_LEN || read == UAEFSDB2_LEN) {
            if (fsdb_debug) {
                TCHAR *an, *nn, *co;
                write_log(_T("->ok\n"));
                an = au_fs((char*)fsdb + 5);
                nn = au_fs((char*)fsdb + 262);
                co = au_fs((char*)fsdb + 519);
                write_log(_T("v=%02x flags=%08x an='%s' nn='%s' c='%s'\n"), fsdb[0], ((uae_u32*)(fsdb + 1))[0], an, nn,
                          co);
                xfree(co);
                xfree(nn);
                xfree(an);
            }
            return 1;
        }
    }
    if (fsdb_debug)
        write_log(_T("->fail %d, %d\n"), read, GetLastError());
    memset(fsdb, 0, UAEFSDB2_LEN);
    return 0;
}


uae_u32 filesys_parse_mask(uae_u32 mask) {
    return mask ^ 0xf;
}


static int write_uaefsdb(const TCHAR* item, uae_u8* fsdb) {
    TCHAR* p;
    HANDLE h;
    DWORD written = 0, itemflag, itemattr;
    DWORD attr = INVALID_FILE_ATTRIBUTES;
    FILETIME t1, t2, t3;
    int time_valid = FALSE;
    int ret = 0;
    const TCHAR* namep;
    TCHAR path[MAX_DPATH];

    if (currprefs.win32_filesystem_mangle_reserved_names == false) {
        _tcscpy(path, PATHPREFIX);
        _tcscat(path, item);
        namep = path;
    } else {
        namep = item;
    }

    p = make_uaefsdbpath(item, NULL);

    itemattr = GetFileAttributesSafe(item);
    itemflag = FILE_ATTRIBUTE_NORMAL;
    if (itemflag != INVALID_FILE_ATTRIBUTES && (itemattr & FILE_ATTRIBUTE_DIRECTORY))
        itemflag = FILE_FLAG_BACKUP_SEMANTICS; /* argh... */
    h = CreateFile(namep, GENERIC_READ, 0, NULL, OPEN_EXISTING, itemflag, NULL);
    if (h != INVALID_HANDLE_VALUE) {
        if (GetFileTime(h, &t1, &t2, &t3)) {
            if (fsdb_debug) {
                write_log(_T("time ok (%08x-%08x %08x-%08x %08x-%08x)\n"), t1.dwHighDateTime, t1.dwLowDateTime,
                          t2.dwHighDateTime, t2.dwLowDateTime, t3.dwHighDateTime, t3.dwLowDateTime);
            }
            time_valid = TRUE;
        }
        CloseHandle(h);
    }
    h = CreateFile(p, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (fsdb_debug) {
        TCHAR *an, *nn, *co;
        an = au_fs((char*)fsdb + 5);
        nn = au_fs((char*)fsdb + 262);
        co = au_fs((char*)fsdb + 519);
        write_log(_T("write_uaefsdb '%s' = %p\n"), p, h);
        write_log(_T("v=%02x flags=%08x an='%s' nn='%s' c='%s'\n"), fsdb[0], ((uae_u32*)(fsdb + 1))[0], an, nn, co);
        xfree(co);
        xfree(nn);
        xfree(an);
    }
    if (h == INVALID_HANDLE_VALUE) {
        DWORD err = GetLastError();
        if (fsdb_debug) {
            write_log(_T("fail %d\n"), err);
        }
        if (err == ERROR_ACCESS_DENIED) {
            attr = GetFileAttributes(p);
            if (fsdb_debug) {
                write_log(_T("attrs %08x\n"), attr);
            }
            if (attr != INVALID_FILE_ATTRIBUTES) {
                if (attr & (FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN)) {
                    uae_u32 attr2 = attr & ~(FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN);
                    if (!SetFileAttributes(p, attr2)) {
                        write_log(_T("'%s' SetFileAttributes1 %08x %d\n"), p, attr2, GetLastError());
                    }
                    h = CreateFile(p, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
                    if (fsdb_debug) {
                        write_log(_T("write_uaefsdb (2) '%s' = %p %d\n"), p, h, GetLastError());
                    }
                }
            }
        }
    }
    if (h != INVALID_HANDLE_VALUE) {
        WriteFile(h, fsdb, UAEFSDB2_LEN, &written, NULL);
        CloseHandle(h);
        if (written == UAEFSDB2_LEN) {
            if (fsdb_debug)
                write_log(_T("->ok\n"));
            ret = 1;
            goto end;
        }
    }
    if (fsdb_debug)
        write_log(_T("->fail %d, %d\n"), written, GetLastError());

    DeleteFile(p);
end:
    if (attr != INVALID_FILE_ATTRIBUTES) {
        if (!SetFileAttributes(p, attr)) {
            write_log(_T("'%s' SetFileAttributes2 %08x %d\n"), p, attr, GetLastError());
        }
    }
    if (time_valid) {
        h = CreateFile(namep, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, itemflag, NULL);
        if (h != INVALID_HANDLE_VALUE) {
            if (!SetFileTime(h, &t1, &t2, &t3)) {
                write_log(_T("'%s' SetFileTime %d\n"), namep, GetLastError());
            }
            CloseHandle(h);
        }
    }
    xfree(p);
    return ret;
}

static void create_uaefsdb(a_inode* aino, uae_u8* buf, int winmode) {
    TCHAR* nn;
    char* s;
    buf[0] = 1;
    do_put_mem_long((uae_u32*)(buf + 1), aino->amigaos_mode);
    s = ua_fs(aino->aname, -1);
    strncpy((char*)buf + 5, s, 256);
    buf[5 + 256] = '\0';
    xfree(s);
    nn = nname_begin(aino->nname);
    s = ua_fs(nn, -1);
    strncpy((char*)buf + 5 + 257, s, 256);
    buf[5 + 257 + 256] = '\0';
    xfree(s);
    s = ua_fs(aino->comment ? aino->comment : _T(""), -1);
    strncpy((char*)buf + 5 + 2 * 257, s, 80);
    buf[5 + 2 * 257 + 80] = '\0';
    xfree(s);
    do_put_mem_long((uae_u32*)(buf + 5 + 2 * 257 + 81), winmode);
    _tcsncpy((TCHAR*)(buf + 604), aino->aname, 256);
    _tcsncpy((TCHAR*)(buf + 1118), nn, 256);
    aino->has_dbentry = 0;
}


static a_inode* aino_from_buf(a_inode* base, uae_u8* buf, int* winmode) {
    uae_u32 mode;
    a_inode* aino = xcalloc(a_inode, 1);
    uae_u8* buf2;
    TCHAR* s;

    buf2 = buf + 604;
    mode = do_get_mem_long((uae_u32*)(buf + 1));
    buf += 5;
    if (buf2[0]) {
        aino->aname = my_strdup((TCHAR*)buf2);
    } else {
        aino->aname = au_fs((char*)buf);
    }
    buf += 257;
    buf2 += 257 * 2;
    if (buf2[0]) {
        aino->nname = build_nname(base->nname, (TCHAR*)buf2);
    } else {
        s = au_fs((char*)buf);
        aino->nname = build_nname(base->nname, s);
        xfree(s);
    }
    buf += 257;
    aino->comment = *buf != '\0' ? my_strdup_ansi((char*)buf) : 0;
    buf += 81;
    aino->amigaos_mode = mode;
    *winmode = do_get_mem_long((uae_u32*)buf);
    aino->dir = ((*winmode) & FILE_ATTRIBUTE_DIRECTORY) ? 1 : 0;
    *winmode &= FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_ARCHIVE | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN;
    aino->has_dbentry = 0;
    aino->dirty = 0;
    aino->db_offset = 0;
    if ((mode = GetFileAttributesSafe(aino->nname)) == INVALID_FILE_ATTRIBUTES) {
        write_log(_T("xGetFileAttributes('%s') failed! error=%d, aino=%p\n"), aino->nname, GetLastError(), aino);
        return aino;
    }
    aino->dir = (mode & FILE_ATTRIBUTE_DIRECTORY) ? 1 : 0;
    return aino;
}


/* For an a_inode we have newly created based on a filename we found on the
 * native fs, fill in information about this file/directory.  */
int fsdb_fill_file_attrs(a_inode* base, a_inode* aino) {
    int mode, winmode, oldamode;
    uae_u8 fsdb[UAEFSDB2_LEN];
    int reset = 0;

    if ((mode = GetFileAttributesSafe(aino->nname)) == INVALID_FILE_ATTRIBUTES) {
        write_log(_T("GetFileAttributes('%s') failed! error=%d, aino=%p dir=%d\n"), aino->nname, GetLastError(), aino,
                  aino->dir);
        return 0;
    }
    aino->dir = (mode & FILE_ATTRIBUTE_DIRECTORY) ? 1 : 0;

    if (mode & FILE_ATTRIBUTE_REPARSE_POINT) {
        WIN32_FIND_DATA fd;
        HANDLE h = FindFirstFile(aino->nname, &fd);
        if (h != INVALID_HANDLE_VALUE) {
            FindClose(h);
            if (fd.dwReserved0 == IO_REPARSE_TAG_SYMLINK && (fd.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) &&
                !(fd.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM)) {
                if (my_resolvessymboliclink(aino->nname, -1)) {
                    // write_log (_T("1 '%s'\n"), aino->nname);
                    aino->softlink = 1;
                }
            }
        }
    }

    if (!aino->softlink && !aino->dir && !(mode & FILE_ATTRIBUTE_SYSTEM)) {
        TCHAR* ext = _tcsrchr(aino->nname, '.');
        if (ext && !_tcsicmp(ext, _T(".lnk"))) {
            TCHAR tmp[MAX_DPATH];
            _tcscpy(tmp, aino->nname);
            if (my_resolvesoftlink(tmp, sizeof tmp / sizeof(TCHAR), false)) {
                // write_log (_T("2 '%s'\n"), aino->nname);
                ext = _tcsrchr(aino->aname, '.');
                if (ext && !_tcsicmp(ext, _T(".lnk")))
                    *ext = 0;
                aino->softlink = 2;
            }
        }
    }

    mode &= FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_ARCHIVE | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN;

    if ((base->volflags & MYVOLUMEINFO_STREAMS) && read_uaefsdb(aino->nname, NULL, fsdb)) {
        aino->amigaos_mode = do_get_mem_long((uae_u32*)(fsdb + 1));
        xfree(aino->comment);
        aino->comment = NULL;
        if (fsdb[5 + 2 * 257])
            aino->comment = my_strdup_ansi((char*)fsdb + 5 + 2 * 257);
        xfree(aino_from_buf(base, fsdb, &winmode));
        if (winmode == mode) /* no Windows-side editing? */
            return 1;
        write_log(_T("FS: '%s' protection flags edited from Windows-side\n"), aino->nname);
        reset = 1;
        /* edited from Windows-side -> use Windows side flags instead */
    }

    oldamode = aino->amigaos_mode;
    aino->amigaos_mode = A_FIBF_EXECUTE | A_FIBF_READ;
    if (!(FILE_ATTRIBUTE_ARCHIVE & mode))
        aino->amigaos_mode |= A_FIBF_ARCHIVE;
    if (!(FILE_ATTRIBUTE_READONLY & mode))
        aino->amigaos_mode |= A_FIBF_WRITE | A_FIBF_DELETE;
    if (FILE_ATTRIBUTE_SYSTEM & mode)
        aino->amigaos_mode |= A_FIBF_PURE;
    if (FILE_ATTRIBUTE_HIDDEN & mode)
        aino->amigaos_mode |= A_FIBF_HIDDEN;
    aino->amigaos_mode = filesys_parse_mask(aino->amigaos_mode);
    aino->amigaos_mode |= oldamode & A_FIBF_SCRIPT;
    if (reset && (base->volflags & MYVOLUMEINFO_STREAMS)) {
        create_uaefsdb(aino, fsdb, mode);
        write_uaefsdb(aino->nname, fsdb);
    }

    return 1;
}


struct my_openfile_s {
    HANDLE h;
};


void my_close(struct my_openfile_s* mos) {
    CloseHandle(mos->h);
    xfree(mos);
}


bool my_createshortcut(char const*, char const*, char const*) {
    UNIMPLEMENTED();
    return false;
}


bool my_isfilehidden(char const*) {
    UNIMPLEMENTED();
    return false;
}

int my_issamevolume(char const*, char const*, char*) {
    UNIMPLEMENTED();
    return 0;
}


uae_s64 my_lseek(struct my_openfile_s* mos, uae_s64 offset, int whence) {
    LARGE_INTEGER li, old;

    old.QuadPart = 0;
    old.LowPart = SetFilePointer(mos->h, 0, &old.HighPart, FILE_CURRENT);
    if (old.LowPart == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR)
        return -1;
    if (offset == 0 && whence == SEEK_CUR)
        return old.QuadPart;
    li.QuadPart = offset;
    li.LowPart = SetFilePointer(mos->h, li.LowPart, &li.HighPart,
                                whence == SEEK_SET ? FILE_BEGIN : (whence == SEEK_END ? FILE_END : FILE_CURRENT));
    if (li.LowPart == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR)
        return -1;
    return old.QuadPart;
}


uae_s64 my_fsize(struct my_openfile_s* mos) {
    LARGE_INTEGER li;
    if (!GetFileSizeEx(mos->h, &li))
        return -1;
    return li.QuadPart;
}


int my_mkdir(const TCHAR* name) {
    const TCHAR* namep;
    TCHAR path[MAX_DPATH];

    if (currprefs.win32_filesystem_mangle_reserved_names == false) {
        _tcscpy(path, PATHPREFIX);
        _tcscat(path, name);
        namep = path;
    } else {
        namep = name;
    }
    return CreateDirectory(namep, NULL) == 0 ? -1 : 0;
}



struct my_openfile_s* my_open(const TCHAR* name, int flags) {
    struct my_openfile_s* mos;
    HANDLE h;
    DWORD DesiredAccess = GENERIC_READ;
    DWORD ShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;
    DWORD CreationDisposition = OPEN_EXISTING;
    DWORD FlagsAndAttributes = FILE_ATTRIBUTE_NORMAL;
    DWORD attr;
    const TCHAR* namep;
    TCHAR path[MAX_DPATH];

    if (currprefs.win32_filesystem_mangle_reserved_names == false) {
        _tcscpy(path, PATHPREFIX);
        _tcscat(path, name);
        namep = path;
    } else {
        namep = name;
    }
    mos = xmalloc(struct my_openfile_s, 1);
    if (!mos)
        return NULL;
    attr = GetFileAttributesSafe(name);
    if (flags & O_TRUNC)
        CreationDisposition = CREATE_ALWAYS;
    else if (flags & O_CREAT)
        CreationDisposition = OPEN_ALWAYS;
    if (flags & O_WRONLY)
        DesiredAccess = GENERIC_WRITE;
    if (flags == O_RDONLY) {
        DesiredAccess = GENERIC_READ;
        CreationDisposition = OPEN_EXISTING;
    }
    if (flags & O_RDWR)
        DesiredAccess = GENERIC_READ | GENERIC_WRITE;
    if (CreationDisposition == CREATE_ALWAYS && attr != INVALID_FILE_ATTRIBUTES &&
        (attr & (FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN)))
        SetFileAttributesSafe(name, FILE_ATTRIBUTE_NORMAL);
    h = CreateFile(namep, DesiredAccess, ShareMode, NULL, CreationDisposition, FlagsAndAttributes, NULL);
    if (h == INVALID_HANDLE_VALUE) {
        DWORD err = GetLastError();
        if (err == ERROR_ACCESS_DENIED && (DesiredAccess & GENERIC_WRITE)) {
            DesiredAccess &= ~GENERIC_WRITE;
            h = CreateFile(namep, DesiredAccess, ShareMode, NULL, CreationDisposition, FlagsAndAttributes, NULL);
            if (h == INVALID_HANDLE_VALUE)
                err = GetLastError();
        }
        if (h == INVALID_HANDLE_VALUE) {
            write_log(_T("failed to open '%s' %x %x err=%d\n"), namep, DesiredAccess, CreationDisposition, err);
            xfree(mos);
            mos = NULL;
            goto err;
        }
    }
    mos->h = h;
err:
    // write_log (_T("open '%s' = %x\n"), namep, mos ? mos->h : 0);
    return mos;
}


FILE* my_opentext(const TCHAR* name) {
    FILE* f;
    uae_u8 tmp[4];
    size_t v;

    f = _tfopen(name, _T("rb"));
    if (!f)
        return NULL;
    v = fread(tmp, 1, sizeof tmp, f);
    fclose(f);
    if (v == 4) {
        if (tmp[0] == 0xef && tmp[1] == 0xbb && tmp[2] == 0xbf)
            return _tfopen(name, _T("r, ccs=UTF-8"));
        if (tmp[0] == 0xff && tmp[1] == 0xfe)
            return _tfopen(name, _T("r, ccs=UTF-16LE"));
    }
    return _tfopen(name, _T("r"));
}


unsigned int my_read(struct my_openfile_s* mos, void* b, unsigned int size) {
    DWORD read = 0;
    ReadFile(mos->h, b, size, &read, NULL);
    return read;
}


unsigned int my_write(struct my_openfile_s* mos, void* b, unsigned int size) {
    DWORD written = 0;
    WriteFile(mos->h, b, size, &written, NULL);
    return written;
}


void my_setfilehidden(char const*, bool) {
    UNIMPLEMENTED();
}
