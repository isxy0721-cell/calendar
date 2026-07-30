# 我的日程管理系统（MySchedule）

一个基于 **Qt 5 / C++17** 的日程管理课程项目。程序同时提供图形界面和命令行接口，任务和账户数据均保存在本地 JSON 文件中，不使用数据库。

## 功能概览

- 账户注册与登录；口令只保存 SHA-256 哈希值，不保存明文。
- 任务包含：唯一 ID、名称、启动时间、提醒时间、优先级、分类和备注。
- 优先级默认“中”，分类默认“生活”，未指定提醒时间时默认在启动前 5 分钟提醒。
- 启动时间全局唯一，因此“任务名称 + 启动时间”也唯一。
- 每次新增、修改、删除任务后立即保存到本地 JSON 文件。
- 登录后加载任务到内存；按日期和启动时间排序显示。
- GUI 支持按当天任务名称搜索、弹窗添加任务、备注、删除任务和语音输入任务名称。
- 后台 `QThread` 每秒检查提醒任务；GUI 弹窗提醒，命令行 `run` 模式打印提醒。
- 内置固定 WAV 提醒音，使用 Qt5 `QSoundEffect` 播放。
- 支持 Windows 和 Linux 语音输入：Windows 使用系统 `System.Speech`，Linux 使用 C++ Vosk C API。
- 命令行支持注册、增、删、查、改和交互式 `run` 模式。

## 环境要求

- Qt 5（组件：`Widgets`、`Multimedia`）
- C++17 编译器
- CMake 3.16+，或 Qt qmake

Linux 的可选语音识别功能还需要：

- `arecord`（通常在 `alsa-utils` 包中）
- Vosk 动态库 `libvosk.so`
- 已解压的 Vosk 语音模型

## 构建

### CMake

```bash
cmake -S . -B build
cmake --build build
```

生成的可执行程序通常位于 `build/myschedule`（Windows 为 `build/myschedule.exe`）。

### qmake

```bash
qmake calendardemo.pro
make
```

Windows MinGW 环境可使用：

```powershell
qmake calendardemo.pro
mingw32-make
```

## 启动 GUI

不带参数运行即可启动图形界面：

```bash
./myschedule
```

主界面用于查看选定日期的日程。右上角的“🔍 搜索”会展开搜索框，按任务名称实时筛选当天任务；“＋ 添加任务”会打开任务编辑窗口。

语音输入通过“开始语音输入”和“结束并识别”控制时长。每次识别结果会追加到已有任务名称后，且自动移除语音结果中的空白字符。

## 命令行使用

查看完整帮助：

```bash
./myschedule --help
```

### 注册

```bash
./myschedule register user1 password
```

### 新增任务

```bash
./myschedule user1 password addtask "学习Qt" 2030-01-01T19:30 high study 2030-01-01T19:25
```

参数格式：

```text
addtask <任务名> <启动时间> [优先级] [分类] [提醒时间]
```

- 时间格式：`yyyy-MM-ddTHH:mm`，例如 `2030-01-01T19:30`
- 优先级：`high`、`medium`（默认）、`low`
- 分类：`study`、`entertainment`、`life`（默认）

### 查询某日任务

```bash
./myschedule user1 password showtask 2030-01-01
```

### 更新任务

先使用 `showtask` 获取任务 ID，再执行：

```bash
./myschedule user1 password updatetask <任务ID> "复习Qt" 2030-01-01T20:00 medium study
```

### 删除任务

```bash
./myschedule user1 password deltask <任务ID>
```

### 交互式 Shell

```bash
./myschedule run
```

也可以直接提供账户：

```bash
./myschedule run user1 password
```

Shell 中可输入：

```text
addtask "完成作业" 2030-01-01T20:00 high study
showtask 2030-01-01
updatetask <任务ID> "完成Qt作业" 2030-01-01T20:30 high study
deltask <任务ID>
help
exit
```

## 数据文件

数据保存在**程序启动时的当前工作目录**：

```text
data/
├── users.json
└── tasks/
    └── <用户名UTF-8十六进制>.json
```

`users.json` 存储用户名与 `passwordHash`；任务 JSON 存储完整任务属性和备注。

## Linux 语音输入配置（可选）

安装录音工具，并设置 Vosk 库和模型路径：

```bash
sudo apt install alsa-utils
export MYSCHEDULE_VOSK_LIBRARY=/绝对路径/libvosk.so
export MYSCHEDULE_VOSK_MODEL=/绝对路径/解压后的Vosk模型目录
```

若未设置这些环境变量，其他日程功能仍可正常使用；仅 Linux 语音输入会给出缺少依赖的提示。

## 命令行压力测试

独立测试脚本位于 [tests/cli_stress_test.sh](tests/cli_stress_test.sh)，覆盖账户注册、批量新增、查询、更新和删除。

默认新增 1000 条任务：

```bash
chmod +x tests/cli_stress_test.sh
./tests/cli_stress_test.sh /绝对路径/myschedule --count 1000
```

脚本默认在临时目录运行，结束后自动删除测试数据。需要保留 JSON 文件检查时：

```bash
./tests/cli_stress_test.sh /绝对路径/myschedule --count 1000 --keep-data
```

此时脚本会在当前目录创建 `stress-test-data-时间戳/`，其中包含 `data/users.json` 和 `data/tasks/`。

## 项目结构

```text
.
├── main.cpp                 # GUI / 命令行入口分发
├── mainwindow.*             # Qt 图形界面
├── cli.*                    # 命令行接口
├── task.*                   # 任务数据模型与 JSON 序列化
├── taskmanager.*            # 任务加载、保存、增删改查
├── usermanager.*            # 账户与口令哈希
├── reminderworker.*         # 后台提醒线程
├── vosktranscriber.*        # Linux C++ Vosk 识别后端
├── assets/                  # 内置提醒音
├── resources.qrc            # Qt 资源配置
└── tests/cli_stress_test.sh # Linux 压力测试脚本
```
