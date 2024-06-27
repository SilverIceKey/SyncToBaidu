# SyncToBaidu

SyncToBaidu 是一个使用`bypy`同步本地目录到百度云的工具。该工具会扫描指定的本地目录，并将其中的文件上传到百度云，同时保持目录结构的同步。

## 特性

- 递归扫描本地目录
- 使用 `bypy` 工具上传文件到百度云
- 通过配置文件配置本地和远程目录路径
- 每4小时自动同步一次
- 可通过命令行参数修改 `crontab` 时间表

## 安装

1. 克隆或下载该仓库。

2. 在 `SyncToBaidu` 目录下编译程序：

    ```bash
    make
    ```

3. 安装程序和配置文件：

    ```bash
    sudo make install
    ```

    这将会：
    - 安装 `libconfig` 和 `bypy`
    - 将编译后的可执行文件安装到 `/usr/local/bin/sync_to_baidu`
    - 创建配置文件目录 `/etc/sync_to_baidu`
    - 将默认的配置文件复制到 `/etc/sync_to_baidu/config.conf`
    - 添加一个 `crontab` 任务，每4小时运行一次 `sync_to_baidu`

## 配置

默认的配置文件位于 `/etc/sync_to_baidu/config.conf`。你可以编辑这个文件来配置本地同步目录和远程百度云目录。

示例配置文件：

```ini
[Paths]
sync_dir = /path/to/syncDir
remote_dir = /remote/path/on/baiduyun

