/** 
 * gdiopt.cpp
 * Copyright (c) 2014-2015 SWAT
 */

#include <string>
#include <vector>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include "gdiopt.h"

int bin2iso(const char *source, const char *target) {

    int seek_header, seek_ecc, sector_size, ret;
    long i, source_length;
    char buf[2352];
    const unsigned char SYNC_HEADER[12] =
            {0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0};

    FILE *fpSource, *fpTarget;

    fpSource = fopen(source, "rb");
    fpTarget = fopen(target, "wb");

    if ((fpSource == nullptr) || (fpTarget == nullptr)) {
        return -1;
    }

    ret = fread(buf, sizeof(char), 16, fpSource);
    if (memcmp(SYNC_HEADER, buf, 12) != 0) {
        seek_header = 8;
        seek_ecc = 280;
        sector_size = 2336;
    } else {
        switch (buf[15]) {
            case 2: {
                seek_header = 24;    // Mode2/2352
                seek_ecc = 280;
                sector_size = 2352;
                break;
            }

            case 1: {
                seek_header = 16;    // Mode1/2352
                seek_ecc = 288;
                sector_size = 2352;
                break;
            }

            default: {
                fclose(fpTarget);
                fclose(fpSource);
                return -1;
            }
        }
    }

    fseek(fpSource, 0L, SEEK_END);
    source_length = ftell(fpSource) / sector_size;
    fseek(fpSource, 0L, SEEK_SET);

    int count = 0;
    for (i = 0; i < source_length; i++) {
        fseek(fpSource, seek_header, SEEK_CUR);
        ret = fread(buf, sizeof(char), 2048, fpSource);
        fwrite(buf, sizeof(char), 2048, fpTarget);
        fseek(fpSource, seek_ecc, SEEK_CUR);
        count++;
        if (count == source_length / 5) {
            printf("#");
            fflush(stdout);
            count = 0;
        }
    }

    fclose(fpTarget);
    fclose(fpSource);

    return 0;
}

int convert_gdi(const GdiFile &file) {

    FILE *fr, *fw;
    int i, rc, track_no, track_count;
    unsigned long start_lba, flags, sector_size, offset;
    char fn_old[256], fn_new[256], full_fn_old[1024], full_fn_new[1024];
    std::string out = file.dirname + "/opt.gdi";

    fr = fopen(file.path.c_str(), "r");
    if (!fr) {
        printf(" can't open for read: %s", file.path.c_str());
        return -1;
    }

    fw = fopen(out.c_str(), "w+");
    if (!fw) {
        printf(" can't open for write: %s", out.c_str());
        fclose(fr);
        return -1;
    }

    rc = fscanf(fr, "%d", &track_count);
    if (rc == 1) {
        fprintf(fw, "%d\n", track_count);

        for (i = 0; i < track_count; i++) {
            start_lba = flags = sector_size = offset = 0;
            memset(fn_new, 0, sizeof(fn_new));
            memset(fn_old, 0, sizeof(fn_old));

            rc = fscanf(fr, "%d %ld %ld %ld %s %ld",
                        &track_no, &start_lba, &flags,
                        &sector_size, fn_old, &offset);

            if (sector_size == 2048)
                continue;

            if (flags == 4) {
                if (sector_size != 2048) {
                    int len = strlen(fn_old);
                    strncpy(fn_new, fn_old, sizeof(fn_new));
                    fn_new[len - 3] = 'i';
                    fn_new[len - 2] = 's';
                    fn_new[len - 1] = 'o';
                    fn_new[len] = '\0';
                    sector_size = 2048;

                    printf(" %s ", fn_new);

                    snprintf(full_fn_old, 1024, "%s/%s", file.dirname.c_str(), fn_old);
                    snprintf(full_fn_new, 1024, "%s/%s", file.dirname.c_str(), fn_new);

                    if (bin2iso(full_fn_old, full_fn_new) < 0) {
                        printf(" error ");
                        fclose(fr);
                        fclose(fw);
                        return -1;
                    }

                    unlink(full_fn_old);
                }

            }

            fprintf(fw, "%d %ld %ld %ld %s %ld\n",
                    track_no, start_lba, flags, sector_size,
                    (flags == 4 ? fn_new : fn_old), offset);
        }
    }

    fclose(fr);
    fclose(fw);

    unlink(file.path.c_str());
    rename(out.c_str(), file.path.c_str());
    printf(" > done.");

    return 0;
}

bool exist(const std::string &file) {
    struct stat st{};
    return (stat(file.c_str(), &st) == 0);
}

bool endsWith(const std::string &value, const std::string &ending) {
    if (ending.size() > value.size()) return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

std::vector<GdiFile> getGdiList(const std::string &path) {

    std::vector<GdiFile> files;
    struct dirent *ent;
    struct stat st{};
    DIR *dir;

    if (!path.empty()) {
        if ((dir = opendir(path.c_str())) != nullptr) {
            while ((ent = readdir(dir)) != nullptr) {

                // skip "hidden" files
                if (ent->d_name[0] == '.') {
                    continue;
                }

                GdiFile file = {ent->d_name, path + "/" + ent->d_name, path};
                if (stat(file.path.c_str(), &st) == 0) {
                    if (S_ISDIR(st.st_mode)) {
                        std::vector<GdiFile> subFiles = getGdiList(file.path);
                        for (const auto &x : subFiles) {
                            files.push_back(x);
                        }
                    } else {
                        if (endsWith(file.name, ".gdi") && !exist(file.dirname + "/track01.iso")) {
                            files.push_back(file);
                        }
                    }
                }
            }
            closedir(dir);
        }
    }

    return files;
}

int main(int argc, char *argv[]) {
#if 1
    if (argc < 2) {
        printf("GDI optimizer v0.3 by SWAT & megavolt85 & cpasjuste\n");
        printf("Usage: %s input_folder\n", argv[0]);
        return 0;
    }

    std::vector<GdiFile> files = getGdiList(argv[1]);
#else
    std::vector<GdiFile> files = getGdiList("/home/cpasjuste/Téléchargements/test/");
#endif
    int size = files.size();
    int count = 1;

    printf("gdiopt: found %zu games to optimize...\n", files.size());
    for (const auto &file : files) {
        printf("[%i/%i] optimizing: %s ... ", count, size, file.name.c_str());
        convert_gdi(file);
        printf("\n");
        count++;
    }

    return 0;
}
