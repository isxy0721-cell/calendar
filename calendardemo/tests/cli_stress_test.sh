#!/usr/bin/env bash
# Linux 命令行压力测试：注册、批量新增、查询、更新、删除。
# 用法：./tests/cli_stress_test.sh /绝对路径/myschedule [--count 1000] [--keep-data]
set -euo pipefail

APP="${1:-./myschedule}"
shift $(( $# > 0 ? 1 : 0 ))
COUNT=1000
KEEP_DATA=false

while (( $# > 0 )); do
    case "$1" in
        --count) COUNT="$2"; shift 2 ;;
        --keep-data) KEEP_DATA=true; shift ;;
        *) echo "未知参数：$1" >&2; exit 2 ;;
    esac
done

if ! [[ "$COUNT" =~ ^[1-9][0-9]*$ ]]; then
    echo "--count 必须是正整数" >&2
    exit 2
fi
if ! APP="$(realpath "$APP")" || [[ ! -x "$APP" ]]; then
    echo "找不到可执行程序：$APP" >&2
    exit 2
fi

WORK_DIR="$(mktemp -d -t myschedule-stress.XXXXXX)"
if [[ "$KEEP_DATA" != true ]]; then
    trap 'rm -rf "$WORK_DIR"' EXIT
fi
cd "$WORK_DIR"

USER_NAME="stress_user"
PASSWORD="stress_password"
BASE_TIME="2030-01-01 08:00"

echo "[1/5] 注册临时账户并准备 $COUNT 条任务"
"$APP" register "$USER_NAME" "$PASSWORD" >/dev/null

echo "[2/5] 批量新增任务（每 100 条显示一次进度）"
task_index=0
while [ "$task_index" -lt "$COUNT" ]; do
    start="$(date -d "$BASE_TIME + $task_index minutes" '+%Y-%m-%dT%H:%M')"
    remind="$(date -d "$BASE_TIME + $task_index minutes - 5 minutes" '+%Y-%m-%dT%H:%M')"
    "$APP" "$USER_NAME" "$PASSWORD" addtask "stress-task-$(printf '%04d' "$task_index")" \
        "$start" medium study "$remind" >/dev/null
    if [ $(((task_index + 1) % 100)) -eq 0 ] || [ $((task_index + 1)) -eq "$COUNT" ]; then
        echo "  已新增 $((task_index + 1))/$COUNT"
    fi
    task_index=$((task_index + 1))
done

echo "[3/5] 查询首日任务"
QUERY_OUTPUT="$("$APP" "$USER_NAME" "$PASSWORD" showtask 2030-01-01)"
FIRST_ID="$(printf '%s\n' "$QUERY_OUTPUT" | awk 'NR == 3 {print $1}')"
if [[ ! "$FIRST_ID" =~ ^[0-9a-fA-F-]{36}$ ]]; then
    echo "无法从查询结果取得任务 ID：" >&2
    printf '%s\n' "$QUERY_OUTPUT" >&2
    exit 1
fi

echo "[4/5] 更新首条任务"
"$APP" "$USER_NAME" "$PASSWORD" updatetask "$FIRST_ID" "stress-task-updated" \
    2030-01-02T08:00 high life 2030-01-02T07:55 >/dev/null

echo "[5/5] 删除已更新任务并验证"
"$APP" "$USER_NAME" "$PASSWORD" deltask "$FIRST_ID" >/dev/null
VERIFY_OUTPUT="$("$APP" "$USER_NAME" "$PASSWORD" showtask 2030-01-02)"
if printf '%s\n' "$VERIFY_OUTPUT" | grep -Fq "stress-task-updated"; then
    echo "删除验证失败：任务仍然存在" >&2
    exit 1
fi

echo "测试通过：已完成注册、$COUNT 条新增、查询、更新和删除。"
if [[ "$KEEP_DATA" == true ]]; then
    echo "测试数据保留在：$WORK_DIR"
fi
