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

2. 安装依赖项：

### 在 Ubuntu 上安装依赖项

    ```bash
    sudo apt-get update
    sudo apt-get install -y build-essential libconfig-dev python3 python3-pip
    pip3 install bypy
    ```

### 在 macOS 上安装依赖项

    ```bash
    brew update
    brew install libconfig python
    pip3 install bypy
    ```

### 在 Windows 上安装依赖项

    ```bash
    choco install mingw -y
    choco install python -y
    pip install bypy
    ```

3. 在 `SyncToBaidu` 目录下编译程序：

    ```bash
    make
    ```

4. 安装程序和配置文件：

    ```bash
    sudo make install
    ```

    这将会：
    - 将编译后的可执行文件安装到 `/usr/local/bin/sync_to_baidu`
    - 创建配置文件目录 `/etc/sync_to_baidu`
    - 将默认的配置文件复制到 `/etc/sync_to_baidu/config.conf`
    - 添加一个 `crontab` 任务，每4小时运行一次 `sync_to_baidu`

## 配置

默认的配置文件位于 `/etc/sync_to_baidu/config.conf`。你可以编辑这个文件来配置本地同步目录和远程百度云目录。

示例配置文件：

```ini
Paths = {
    sync_dir = "/path/to/syncDir"
    remote_dir = "/remote/path/on/baiduyun"
}
```

## 使用
程序会每4小时自动运行一次，扫描 syncDir 目录并将文件上传到百度云。你可以查看日志文件 /var/log/sync_to_baidu.log 以获取同步的详细信息。

## 卸载

如果需要卸载程序，可以运行以下命令：

```bash
sudo make uninstall
```

## 命令行参数

- -h 或 --help：显示帮助信息并退出
- -s 或 --schedule TIME：设置 crontab 计划任务时间（例如，"0 */4 * * *" 表示每4小时运行一次）

```bash
sync_to_baidu --help
sync_to_baidu --schedule "0 */4 * * *"
```
