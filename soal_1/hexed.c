#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <curl/curl.h>

#define MAX_PATH 1024
#define LOG_FILE "conversion.log"
#define IMAGE_DIR "image"

void create_dir_if_not_exists(const c   har *dir)
{
    struct stat st = {0};
    if (stat(dir, &st) == -1)
    {
        mkdir(dir, 0755);
    }
}

void get_timestamp(char *timestamp)
{
    time_t now;
    struct tm *tm_info;

    time(&now);
    tm_info = localtime(&now);

    strftime(timestamp, 20, "%Y-%m-%d_%H:%M:%S", tm_info);
}

void get_log_timestamp(char *timestamp)
{
    time_t now;
    struct tm *tm_info;

    time(&now);
    tm_info = localtime(&now);

    strftime(timestamp, 20, "[%Y-%m-%d][%H:%M:%S]", tm_info);
}

size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    return fwrite(ptr, size, nmemb, stream);
}

int download_file(const char *url, const char *output_filename)
{
    CURL *curl;
    FILE *fp;
    CURLcode res;

    curl = curl_easy_init();
    if (!curl)
    {
        fprintf(stderr, "Failed to initialize curl\n");
        return -1;
    }

    fp = fopen(output_filename, "wb");
    if (!fp)
    {
        fprintf(stderr, "Failed to open file %s\n", output_filename);
        curl_easy_cleanup(curl);
        return -1;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    res = curl_easy_perform(curl);
    if (res != CURLE_OK)
    {
        fprintf(stderr, "Download failed: %s\n", curl_easy_strerror(res));
        fclose(fp);
        curl_easy_cleanup(curl);
        return -1;
    }

    fclose(fp);
    curl_easy_cleanup(curl);
    return 0;
}

int unzip_file(const char *zip_filename)
{
    char command[MAX_PATH * 2];
    snprintf(command, sizeof(command), "unzip -o %s", zip_filename);
    return system(command);
}

int hex_to_bin(const char *hex_str, unsigned char **bin_data, size_t *bin_size)
{
    size_t hex_len = strlen(hex_str);
    if (hex_len % 2 != 0)
    {
        fprintf(stderr, "Invalid hex string length\n");
        return -1;
    }

    *bin_size = hex_len / 2;
    *bin_data = malloc(*bin_size);
    if (!*bin_data)
    {
        fprintf(stderr, "Memory allocation failed\n");
        return -1;
    }

    for (size_t i = 0; i < *bin_size; i++)
    {
        sscanf(hex_str + 2 * i, "%2hhx", &(*bin_data)[i]);
    }

    return 0;
}

int convert_hex_to_image(const char *input_filename, const char *output_filename)
{
    FILE *input, *output;
    char *hex_str = NULL;
    size_t len = 0;
    unsigned char *bin_data = NULL;
    size_t bin_size = 0;

    
    input = fopen(input_filename, "r");
    if (!input)
    {
        fprintf(stderr, "Failed to open input file %s\n", input_filename);
        return -1;
    }

    if (getline(&hex_str, &len, input) == -1)
    {
        fprintf(stderr, "Failed to read hex string from %s\n", input_filename);
        fclose(input);
        return -1;
    }

    fclose(input);

    hex_str[strcspn(hex_str, "\n")] = 0;

    if (hex_to_bin(hex_str, &bin_data, &bin_size))
    {
        free(hex_str);
        return -1;
    }

    free(hex_str);

    output = fopen(output_filename, "wb");
    if (!output)
    {
        fprintf(stderr, "Failed to open output file %s\n", output_filename);
        free(bin_data);
        return -1;
    }

    fwrite(bin_data, 1, bin_size, output);
    fclose(output);
    free(bin_data);

    return 0;
}

void log_conversion(const char *input_filename, const char *output_filename)
{
    FILE *log;
    char timestamp[20];
    char input_basename[MAX_PATH];
    char output_basename[MAX_PATH];

    strncpy(input_basename, input_filename, MAX_PATH);
    char *last_slash = strrchr(input_basename, '/');
    if (last_slash)
    {
        strcpy(input_basename, last_slash + 1);
    }

    strncpy(output_basename, output_filename, MAX_PATH);
    last_slash = strrchr(output_basename, '/');
    if (last_slash)
    {
        strcpy(output_basename, last_slash + 1);
    }

    get_log_timestamp(timestamp);

    log = fopen(LOG_FILE, "a");
    if (!log)
    {
        fprintf(stderr, "Failed to open log file\n");
        return;
    }

    fprintf(log, "%s: Successfully converted hexadecimal text %s to %s.\n",
            timestamp, input_basename, output_basename);
    fclose(log);
}

void process_hex_files_in_dir(const char *dir_path)
{
    DIR *dir;
    struct dirent *ent;
    char timestamp[20];
    char input_path[MAX_PATH];
    char output_filename[MAX_PATH];
    char base_name[256];
    const char *output_dir = IMAGE_DIR;

    create_dir_if_not_exists(output_dir);

    if ((dir = opendir(dir_path)) != NULL)
    {
        while ((ent = readdir(dir)) != NULL)
        {
            char *ext = strrchr(ent->d_name, '.');
            if (!ext || strcmp(ext, ".txt") != 0)
                continue;

            snprintf(input_path, MAX_PATH, "%s/%s", dir_path, ent->d_name);

            strncpy(base_name, ent->d_name, sizeof(base_name) - 1);
            base_name[sizeof(base_name) - 1] = '\0';
            char *dot = strrchr(base_name, '.');
            if (dot)
                *dot = '\0';

            get_timestamp(timestamp);

            snprintf(output_filename, MAX_PATH, "%s/%s_image_%s.png", output_dir, base_name, timestamp);

            if (convert_hex_to_image(input_path, output_filename) == 0)
            {
                log_conversion(input_path, output_filename);
                printf("Converted %s to %s\n", input_path, output_filename);
            }
            else
            {
                fprintf(stderr, "Failed to convert %s\n", input_path);
            }
        }
        closedir(dir);
    }
    else
    {
        perror("Could not open directory");
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s <download_url>\n", argv[0]);
        return 1;
    }

    const char *download_url = argv[1];
    const char *zip_filename = "download.zip";

    curl_global_init(CURL_GLOBAL_DEFAULT);

    printf("Downloading %s...\n", download_url);
    if (download_file(download_url, zip_filename) != 0)
    {
        curl_global_cleanup();
        return 1;
    }

    printf("Unzipping %s...\n", zip_filename);
    if (unzip_file(zip_filename) != 0)
    {
        fprintf(stderr, "Failed to unzip file\n");
        curl_global_cleanup();
        return 1;
    }

    if (unlink(zip_filename) == 0)
    {
        printf("Deleted ZIP file: %s\n", zip_filename);
    }
    else
    {
        perror("Failed to delete ZIP file");
    }

    printf("Processing hex files...\n");
    process_hex_files_in_dir("anomali");

    curl_global_cleanup();

    printf("Done!\n");
    return 0;
}