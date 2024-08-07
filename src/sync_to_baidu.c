#include <libconfig.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define CONFIG_FILE "/etc/sync_to_baidu/config.conf"
#define LOG_FILE "/var/log/sync_to_baidu.log"
#define CRON_JOB_FILE "/etc/cron.d/sync_to_baidu_cron"

char sync_dir[256];
char remote_dir[256];

// 检查目录是否为空
int is_directory_empty(const char *path) {
    int n = 0;
    struct dirent *d;
    DIR *dir = opendir(path);
    if (dir == NULL) {
        return 1; // 如果目录无法打开，则视为空目录
    }
    while ((d = readdir(dir)) != NULL) {
        if (++n > 2) {
            closedir(dir);
            return 0; // 目录中有文件或文件夹
        }
    }
    closedir(dir);
    return 1; // 目录为空
}

// 递归扫描目录并同步文件到百度云
void sync_directory(const char *local_path, const char *remote_path) {
    DIR *dir;
    struct dirent *entry;

    if (!(dir = opendir(local_path))) {
        perror("无法打开目录");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR) {
            char new_local_path[1024];
            char new_remote_path[1024];

            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }

            snprintf(new_local_path, sizeof(new_local_path), "%s/%s", local_path, entry->d_name);
            snprintf(new_remote_path, sizeof(new_remote_path), "%s/%s", remote_path, entry->d_name);

            // 递归同步子目录
            sync_directory(new_local_path, new_remote_path);
        } else {
            char local_file_path[1024];
            char remote_file_path[1024];
            char command[4096];  // 增加缓冲区大小

            snprintf(local_file_path, sizeof(local_file_path), "%s/%s", local_path, entry->d_name);
            snprintf(remote_file_path, sizeof(remote_file_path), "%s/%s", remote_path, entry->d_name);

            // 使用 bypy 上传文件到百度云
            snprintf(command, sizeof(command), "bypy upload %s %s", local_file_path, remote_file_path);
            (void)system(command);  // 显式忽略返回值

            // 记录日志
            FILE *log = fopen(LOG_FILE, "a");
            if (log) {
                fprintf(log, "上传 %s 到 %s\n", local_file_path, remote_file_path);
                fclose(log);
            }
        }
    }
    closedir(dir);
}

void load_config(const char *config_file) {
    config_t cfg;
    config_init(&cfg);

    if (!config_read_file(&cfg, config_file)) {
        fprintf(stderr, "读取配置文件 %s 出错:%d - %s\n", config_file, config_error_line(&cfg), config_error_text(&cfg));
        config_destroy(&cfg);
        exit(EXIT_FAILURE);
    }

    const char *sync_dir_temp;
    const char *remote_dir_temp;

    if (config_lookup_string(&cfg, "Paths.sync_dir", &sync_dir_temp)) {
        printf("从配置文件中读取的同步目录: %s\n", sync_dir_temp);
        strcpy(sync_dir, sync_dir_temp);
    } else {
        fprintf(stderr, "配置文件中没有 'sync_dir' 设置。\n");
    }

    if (config_lookup_string(&cfg, "Paths.remote_dir", &remote_dir_temp)) {
        printf("从配置文件中读取的远程目录: %s\n", remote_dir_temp);
        strcpy(remote_dir, remote_dir_temp);
    } else {
        fprintf(stderr, "配置文件中没有 'remote_dir' 设置。\n");
    }

    config_destroy(&cfg);
}

void set_cron_schedule(const char *schedule) {
    // 临时文件用于存储现有的和新的 cron 任务
    FILE *temp_cron = fopen("/tmp/temp_cron", "w");
    if (!temp_cron) {
        perror("无法创建临时 cron 文件");
        exit(EXIT_FAILURE);
    }

    // 使用 crontab -l 命令获取当前用户的 cron 任务
    FILE *current_cron = popen("crontab -l", "r");
    if (!current_cron) {
        perror("无法读取当前的 cron 任务");
        fclose(temp_cron);
        exit(EXIT_FAILURE);
    }

    char line[256];
    int task_found = 0;
    while (fgets(line, sizeof(line), current_cron)) {
        // 检查是否存在相同的任务
        if (strstr(line, "/usr/local/bin/sync_to_baidu") == NULL) {
            fputs(line, temp_cron); // 不是目标任务，写入临时文件
        } else {
            // 是目标任务，更新为新的时间表
            fprintf(temp_cron, "%s /usr/local/bin/sync_to_baidu\n", schedule);
            task_found = 1;
        }
    }
    pclose(current_cron);

    // 如果没有找到相同的任务，追加新任务
    if (!task_found) {
        fprintf(temp_cron, "%s /usr/local/bin/sync_to_baidu\n", schedule);
    }

    fclose(temp_cron);

    // 使用 crontab 命令将临时文件设置为当前用户的 Crontab
    if (system("crontab /tmp/temp_cron") == -1) {
        perror("无法设置 cron 任务");
        exit(EXIT_FAILURE);
    }

    // 删除临时文件
    if (remove("/tmp/temp_cron") != 0) {
        perror("无法删除临时 cron 文件");
    }

    printf("Cron 计划任务已更新为: %s\n", schedule);
}

void print_help() {
    printf("用法: sync_to_baidu [选项]\n");
    printf("选项:\n");
    printf("  -h, --help           显示此帮助信息并退出\n");
    printf("  -s, --schedule TIME  设置 crontab 计划任务时间 (例如, \"0 */4 * * *\" 表示每4小时运行一次)\n");
}

int main(int argc, char *argv[]) {
    if (argc > 1) {
        if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
            print_help();
            return 0;
        } else if (strcmp(argv[1], "-s") == 0 || strcmp(argv[1], "--schedule") == 0) {
            if (argc != 3) {
                fprintf(stderr, "错误: 未指定计划任务时间\n");
                print_help();
                return 1;
            }
            set_cron_schedule(argv[2]);
            return 0;
        } else {
            fprintf(stderr, "错误: 未知选项 %s\n", argv[1]);
            print_help();
            return 1;
        }
    }

    load_config(CONFIG_FILE);

    if (is_directory_empty(sync_dir)) {
        printf("目录 %s 为空，没有文件需要同步。\n", sync_dir);
        return 0;
    }

    sync_directory(sync_dir, remote_dir);
    return 0;
}
