#include "bootstrap_baremetal_guard.h"

#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>

#define RAF_OK 0
#define RAF_ERR_PARAM -1
#define RAF_ERR_PREFIX -2
#define RAF_ERR_PAGESZ -3
#define RAF_ERR_DIRS -4
#define RAF_ERR_BINS -5
#define RAF_ERR_JSON -6

static int has_usr_suffix(const char* prefix) {
    return strstr(prefix, "/files/usr") != NULL;
}

static int join_path(char* out, size_t out_sz, const char* prefix, const char* suffix) {
    int n;
    if (!out || !prefix || !suffix || out_sz == 0) return RAF_ERR_PARAM;
    n = snprintf(out, out_sz, "%s%s", prefix, suffix);
    if (n <= 0 || n >= (int)out_sz) return RAF_ERR_PARAM;
    return RAF_OK;
}

int raf_bootstrap_guard_check_page_size(void) {
    long sz = sysconf(_SC_PAGESIZE);
    if (sz == 4096 || sz == 16384) return (int)sz;
    return RAF_ERR_PAGESZ;
}

int raf_bootstrap_guard_check_exec(const char* path) {
    struct stat st;
    if (!path || !*path) return RAF_ERR_PARAM;
    if (stat(path, &st) != 0) return RAF_ERR_BINS;
    if (!S_ISREG(st.st_mode)) return RAF_ERR_BINS;
    if ((st.st_mode & S_IXUSR) == 0) return RAF_ERR_BINS;
    return RAF_OK;
}

int raf_bootstrap_guard_check_basic_dirs(const char* prefix) {
    static const char* dirs[] = {"/bin", "/etc", "/lib", "/tmp", "/var"};
    char path[PATH_MAX];
    struct stat st;
    size_t i;

    if (!prefix || !*prefix) return RAF_ERR_PARAM;

    for (i = 0; i < sizeof(dirs) / sizeof(dirs[0]); i++) {
        if (join_path(path, sizeof(path), prefix, dirs[i]) < 0) return RAF_ERR_DIRS;
        if (stat(path, &st) != 0 || !S_ISDIR(st.st_mode)) return RAF_ERR_DIRS;
    }

    return RAF_OK;
}

int raf_bootstrap_guard_check_required_bins(const char* prefix) {
    static const char* bins[] = {"/bin/sh", "/bin/pkg"};
    char path[PATH_MAX];
    size_t i;

    if (!prefix || !*prefix) return RAF_ERR_PARAM;

    for (i = 0; i < sizeof(bins) / sizeof(bins[0]); i++) {
        if (join_path(path, sizeof(path), prefix, bins[i]) < 0) return RAF_ERR_BINS;
        if (raf_bootstrap_guard_check_exec(path) < 0) return RAF_ERR_BINS;
    }

    return RAF_OK;
}

int raf_bootstrap_guard_validate_prefix(const char* prefix, char* out_json, int cap) {
    int page_size;
    int dirs_rc;
    int bins_rc;
    int sh_ok = 0;
    int pkg_ok = 0;
    int busybox_ok = 0;
    int proot_ok = 0;
    int exec_ok;
    int n;
    char path[PATH_MAX];

    if (!out_json || cap <= 0) return RAF_ERR_PARAM;
    out_json[0] = '\0';

    if (!prefix) return RAF_ERR_PARAM;
    if (!prefix[0]) return RAF_ERR_PREFIX;
    if (!has_usr_suffix(prefix)) return RAF_ERR_PREFIX;

    page_size = raf_bootstrap_guard_check_page_size();
    if (page_size < 0) return page_size;

    if (join_path(path, sizeof(path), prefix, "/bin/sh") == RAF_OK) {
        sh_ok = raf_bootstrap_guard_check_exec(path) == RAF_OK;
    }
    if (join_path(path, sizeof(path), prefix, "/bin/pkg") == RAF_OK) {
        pkg_ok = raf_bootstrap_guard_check_exec(path) == RAF_OK;
    }
    if (join_path(path, sizeof(path), prefix, "/bin/busybox") == RAF_OK) {
        busybox_ok = raf_bootstrap_guard_check_exec(path) == RAF_OK;
    }
    if (join_path(path, sizeof(path), prefix, "/bin/proot") == RAF_OK) {
        proot_ok = raf_bootstrap_guard_check_exec(path) == RAF_OK;
    }

    exec_ok = sh_ok && pkg_ok;
    dirs_rc = raf_bootstrap_guard_check_basic_dirs(prefix);
    bins_rc = raf_bootstrap_guard_check_required_bins(prefix);

    n = snprintf(out_json, (size_t)cap,
        "{\"guard\":\"bootstrap_baremetal\",\"prefix\":\"%s\",\"page_size\":%d,"
        "\"bin_sh\":%s,\"bin_pkg\":%s,\"busybox\":%s,\"proot\":%s,\"exec_ok\":%s,\"status\":\"%s\"}",
        prefix,
        page_size,
        sh_ok ? "true" : "false",
        pkg_ok ? "true" : "false",
        busybox_ok ? "true" : "false",
        proot_ok ? "true" : "false",
        exec_ok ? "true" : "false",
        (dirs_rc == RAF_OK && bins_rc == RAF_OK) ? "OK" : "ERROR");

    if (n < 0 || n >= cap) return RAF_ERR_JSON;
    if (dirs_rc != RAF_OK) return RAF_ERR_DIRS;
    if (bins_rc != RAF_OK) return RAF_ERR_BINS;
    return RAF_OK;
}

int raf_bootstrap_guard_selftest(char* out_json, int cap) {
    int page_size;
    int n;

    if (!out_json || cap <= 0) return RAF_ERR_PARAM;

    page_size = raf_bootstrap_guard_check_page_size();
    n = snprintf(out_json, (size_t)cap,
        "{\"guard\":\"bootstrap_baremetal\",\"prefix\":\"selftest\",\"page_size\":%d,"
        "\"bin_sh\":false,\"bin_pkg\":false,\"busybox\":false,\"proot\":false,\"exec_ok\":false,\"status\":\"%s\"}",
        page_size > 0 ? page_size : 0,
        page_size > 0 ? "OK" : "ERROR");
    if (n < 0 || n >= cap) return RAF_ERR_JSON;
    if (page_size < 0) return page_size;
    return RAF_OK;
}
