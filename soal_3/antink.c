#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <stdlib.h>
#include <time.h>
#define LOGFILE "/var/log/it24.log"
#define ORIGINAL_DIR "/it24_host"

void rot13(char *s) {
    while (*s) {
        if (*s >= 'a' && *s <= 'z') {
            *s = ((*s - 'a' + 13) % 26) + 'a';
        } else if (*s >= 'A' && *s <= 'Z') {
            *s = ((*s - 'A' + 13) % 26) + 'A';
        }
        s++;
    }
}

int is_malicious(const char *name) {
    return strstr(name, "nafis") || strstr(name, "kimcun");
}

void reverse_str(char *str) {
    int len = strlen(str);
    for (int i = 0; i < len / 2; i++) {
        char tmp = str[i];
        str[i] = str[len - i - 1];
        str[len - i - 1] = tmp;
    }
}

void log_event(const char *msg) {
    FILE *f = fopen(LOGFILE, "a");
    if (!f) return;
time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    char time_str[64];
    strftime(time_str, sizeof(time_str), "[%d-%m-%Y][%H:%M:%S]", tm_info);

    fprintf(f, "%s - %s\n", time_str, msg);
    fflush(f);
    fclose(f);
}

static int do_getattr(const char *path, struct stat *stbuf) {
    char fpath[1000];
    sprintf(fpath, "%s%s", ORIGINAL_DIR, path);
    int res = lstat(fpath, stbuf);


    
    char logmsg[1024];
    snprintf(logmsg, sizeof(logmsg), "[GETATTR] Path: %s", path);
    log_event(logmsg);

    return res == -1 ? -errno : 0;
}
static int do_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                      off_t offset, struct fuse_file_info *fi) {
    DIR *dp;
    struct dirent *de;

    char fpath[1000];
    sprintf(fpath, "%s%s", ORIGINAL_DIR, path);

    (void) offset;
    (void) fi;

    dp = opendir(fpath);
    if (dp == NULL)
        return -errno;
    
        char logmsg[1024];
    snprintf(logmsg, sizeof(logmsg), "[READDIR] Dir: %s", path);
    log_event(logmsg);

    while ((de = readdir(dp)) != NULL) {
        char name[256];
        strcpy(name, de->d_name);
        if (is_malicious(name)) {
            reverse_str(name);
            char rev_msg[1024];
            snprintf(rev_msg, sizeof(rev_msg), "[REVERSED] Original: %s", de->d_name);
            log_event(rev_msg);
        }

        struct stat st;
        memset(&st, 0, sizeof(st));
        st.st_ino = de->d_ino;
        st.st_mode = de->d_type << 12;
        if (filler(buf, name, &st, 0))
            break;
    }

    closedir(dp);
    return 0;
}

static int do_open(const char *path, struct fuse_file_info *fi) {
    char fpath[1000];
    sprintf(fpath, "%s%s", ORIGINAL_DIR, path);
    int fd = open(fpath, O_RDONLY);
    if (fd == -1)
        return -errno;

    close(fd);

    char logmsg[1024];
    snprintf(logmsg, sizeof(logmsg), "[OPEN] File: %s", path);
    log_event(logmsg);

    return 0;
}

static int do_read(const char *path, char *buf, size_t size, off_t offset,
                   struct fuse_file_info *fi) {
    char fpath[1000];
    sprintf(fpath, "%s%s", ORIGINAL_DIR, path);

    if (is_malicious(path)) {
        FILE *f = fopen(fpath, "r");
        if (!f) return -errno;
        fread(buf, 1, size, f);
        fclose(f);

         char decrypt[1024];
        strcpy(decrypt, buf);
        rot13(decrypt);

        char logmsg[1024];
        snprintf(logmsg, sizeof(logmsg), "[READ MALICIOUS] File: %s", path);
        log_event(logmsg);


        return strlen(buf);
    }

    int fd = open(fpath, O_RDONLY);
    if (fd == -1)
        return -errno;

    int res = pread(fd, buf, size, offset);
    close(fd);

    if (res > 0) {
        buf[res] = '\0';
        rot13(buf);
    }

    char logmsg[1024];
    snprintf(logmsg, sizeof(logmsg), "[READ] File: %s", path);
    log_event(logmsg);

    return res;
}

static struct fuse_operations antink_oper = {
    .getattr    = do_getattr,
    .readdir    = do_readdir,
    .open       = do_open,
    .read       = do_read,
};

int main(int argc, char *argv[]) {
    return fuse_main(argc, argv, &antink_oper, NULL);
}
