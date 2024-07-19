CC = gcc
CFLAGS ?= -Wall -O2
LDFLAGS = -lconfig
TARGET = sync_to_baidu
CONFIG_DIR = /etc/sync_to_baidu
CONFIG_FILE = config/config.conf
CRON_JOB_FILE = /etc/cron.d/sync_to_baidu_cron

all: $(TARGET)

$(TARGET): src/sync_to_baidu.c
	$(CC) $(CFLAGS) -o $(TARGET) src/sync_to_baidu.c $(LDFLAGS)

install: $(TARGET)
	# 安装程序和配置文件
	install -m 0755 $(TARGET) /usr/local/bin/$(TARGET)
	mkdir -p $(CONFIG_DIR)
	@if [ ! -f $(CONFIG_DIR)/config.conf ]; then \
		install -m 0644 $(CONFIG_FILE) $(CONFIG_DIR)/config.conf; \
		echo "配置文件 $(CONFIG_DIR)/config.conf 已创建。"; \
	else \
		echo "配置文件 $(CONFIG_DIR)/config.conf 已存在，跳过创建。"; \
	fi

	# 添加 crontab 任务
	#echo "0 */4 * * * /usr/local/bin/$(TARGET)" > $(CRON_JOB_FILE)
	#chmod 644 $(CRON_JOB_FILE)

uninstall:
	rm -f /usr/local/bin/$(TARGET)
	rm -rf $(CONFIG_DIR)
	# 移除 crontab 任务
	rm -f $(CRON_JOB_FILE)

clean:
	rm -f $(TARGET)
