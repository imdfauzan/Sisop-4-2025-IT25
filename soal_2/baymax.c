#define FUSE_USE_VERSION 31

#include <fuse3/fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>

#define FRAGMENT_SIZE 1024
#define MAX_FRAGMENTS 100
#define MAX_PATH 512
#define RELICS_PATH "./relics"
#define LOG_FILE "./activity.log"

static const char *virtual_file = "Baymax.jpeg";

//tulis log
void log_activity(const char *action, const char *detail) {
    FILE *log = fopen(LOG_FILE, "a"); //a = append
    if (!log) return;

    //ambil waktu skrg
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    fprintf(log, "[%04d-%02d-%02d %02d:%02d:%02d] %s: %s\n",
            tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
            tm.tm_hour, tm.tm_min, tm.tm_sec,
            action, detail);
    fclose(log);
}

//ambil atribut potongan fileny
static int baymax_ambilatribut(const char *path, struct stat *stbuf, struct fuse_file_info *fi) {
    (void) fi; //hindari warning compiler kalo ad parameter yg ga diisi
    memset(stbuf, 0, sizeof(struct stat));

    if (strcmp(path, "/") == 0) { //klo path nya (root)
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
    } else if (strcmp(path + 1, virtual_file) == 0) {
        stbuf->st_mode = S_IFREG | 0644;
        stbuf->st_nlink = 1;
        stbuf->st_size = FRAGMENT_SIZE * 14; // 14 fragments
    } else {
        return -ENOENT;
    }
    return 0;
}

//liat isi direktori (ls)
static int baymax_liatdirektori(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi, enum fuse_readdir_flags flags) {
    (void) offset ; (void) fi ; (void) flags;

    if (strcmp(path, "/") != 0)
        return -ENOENT;

    filler(buf, ".", NULL, 0, 0);
    filler(buf, "..", NULL, 0, 0);
    filler(buf, virtual_file, NULL, 0, 0);
    return 0;
}

//cek file yg dibuka apkh sesuai 
static int baymax_bukafile(const char *path, struct fuse_file_info *fi) {
    if (strcmp(path + 1, virtual_file) != 0)
        return -ENOENT; //enoent = "No such file or directory"

    return 0;
}

// Read file (B, E)
static int baymax_read(const char *path, char *buf, size_t size, off_t offset,
                struct fuse_file_info *fi) {
    (void) fi;

    //cek apkh file yg diminta benar
    if (strcmp(path + 1, virtual_file) != 0)
        return -ENOENT;

    size_t total_read = 0;
    for (int i = 0; i < 14; i++) { //baca 14 file gambarnya
        char frag_path[256];
        snprintf(frag_path, sizeof(frag_path), "%s/%s.%03d", RELICS_PATH, virtual_file, i);

        FILE *fp = fopen(frag_path, "rb"); //buka file pakai fopen
        if (!fp) continue; //klo bagian file (fragmen) gaada, diskip

        char temp[FRAGMENT_SIZE];
        size_t bytes = fread(temp, 1, FRAGMENT_SIZE, fp);
        fclose(fp);

        if ((off_t)(i * FRAGMENT_SIZE + bytes) <= offset) { //tangani offset
            continue;
        }

        int start;
        if (offset > i*FRAGMENT_SIZE){
            start = offset - i * FRAGMENT_SIZE;
        } else {
            start=0;
        }

        int remain = bytes-start;
        if (remain <= 0) continue;

        size_t to_copy;
        if (remain > size-total_read){
            to_copy = size-total_read;
        } else {
            to_copy = remain;
        }
        memcpy(buf + total_read, temp + start, to_copy); //salin data ke buffer
        total_read += to_copy;

        if (total_read >= size) break;
    }

    log_activity("READ", virtual_file); //catet ke log
    return total_read;
}

//buat file 
static int baymax_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
    (void) mode;
    (void) fi;
    return 0; // Nothing now, actual write handled later
}

// Write file (C, E)
static int baymax_write(const char *path, const char *buf, size_t size, off_t offset,
                 struct fuse_file_info *fi) {
    (void) fi;
    char filename[MAX_PATH];
    strcpy(filename, path + 1); //hapus "/" diawal

    int total_parts = (size + FRAGMENT_SIZE - 1) / FRAGMENT_SIZE;

    for (int i=0 ; i<total_parts ; i++) { //hitung juml fragmen yg diperlukan
        char frag_path[MAX_PATH];
        snprintf(frag_path, sizeof(frag_path), "%s/%s.%03d", RELICS_PATH, filename, i);
        FILE *fp = fopen(frag_path, "wb");
        if (!fp) return -EIO;

        size_t to_write = (size - i * FRAGMENT_SIZE > FRAGMENT_SIZE) ? FRAGMENT_SIZE : (size - i * FRAGMENT_SIZE);
        fwrite(buf + i * FRAGMENT_SIZE, 1, to_write, fp);
        fclose(fp);
    }

    char logmsg[512]; //tulis log aktivitas
    snprintf(logmsg, sizeof(logmsg), "%s -> ", filename);
    for (int i=0 ; i<total_parts ; i++) {
        char fragname[MAX_PATH];
        snprintf(fragname, sizeof(fragname), "%s.%03d", filename, i);
        strcat(logmsg, fragname);
        if (i != total_parts - 1) strcat(logmsg, ", ");
    }
    log_activity("WRITE", logmsg);
    return size;
}

//hapus fragmen2 file
static int baymax_unlink(const char *path) {
    char filename[512];
    strcpy(filename, path + 1);

    char logmsg[512];
    logmsg[0] = '\0';

    for (int i = 0; i < MAX_FRAGMENTS; i++) {
        char frag_path[512];
        snprintf(frag_path, sizeof(frag_path), "%s/%s.%03d", RELICS_PATH, filename, i);
        if (access(frag_path, F_OK) == 0) {
            remove(frag_path);
            char fragname[64];
            snprintf(fragname, sizeof(fragname), "%s.%03d", filename, i);
            strcat(logmsg, fragname);
            strcat(logmsg, " ");
        } else {
            break;
        }
    }
    if (strlen(logmsg) > 0)
        log_activity("DELETE", logmsg);

    return 0;
}

static struct fuse_operations baymax_oper = {
    .getattr = baymax_ambilatribut,
    .readdir = baymax_liatdirektori,
    .open    = baymax_bukafile,
    .read    = baymax_read,
    .create  = baymax_create,
    .write   = baymax_write,
    .unlink  = baymax_unlink,
};

int main(int argc, char *argv[]) {
    return fuse_main(argc, argv, &baymax_oper, NULL);
}
