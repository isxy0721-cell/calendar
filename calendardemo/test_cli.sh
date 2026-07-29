#!/usr/bin/env bash
# Linux CLI task test utility.
# Usage:
#   ./test_cli.sh register <program> <username> <password>
#   ./test_cli.sh add1000  <program> <username> <password> [date]
#   ./test_cli.sh show     <program> <username> <password> [date]
#   ./test_cli.sh update   <program> <username> <password> <task-id> <name> <start-time> [priority] [category] [remind-time]
#   ./test_cli.sh delete   <program> <username> <password> <task-id>
# Date format: yyyy-MM-dd. Time format: yyyy-MM-ddTHH:mm.

set -euo pipefail

usage() {
    sed -n '2,10p' "$0"
    exit "${1:-0}"
}

[[ $# -ge 1 ]] || usage 1
action="$1"
shift

case "$action" in
    help|--help|-h) usage ;;
esac

if [[ $# -lt 3 ]]; then
    echo "ERROR: expected <program> <username> <password>." >&2
    usage 1
fi

binary="$1"
username="$2"
password="$3"
shift 3

if [[ "$binary" == */* ]]; then
    binary="$(realpath "$binary")"
else
    binary="$(command -v "$binary" || true)"
fi

if [[ ! -x "$binary" ]]; then
    echo "ERROR: myschedule executable not found: $binary" >&2
    exit 1
fi

# TaskManager saves data relative to the process working directory. By default,
# use the executable directory, so build/myschedule uses build/data.
data_dir="${MYSCHEDULE_DATA_DIR:-$(dirname "$binary")}"
if [[ ! -d "$data_dir" ]]; then
    echo "ERROR: data working directory does not exist: $data_dir" >&2
    exit 1
fi
cd "$data_dir"
echo "Using data directory: $(pwd)/data"

case "$action" in
    register)
        [[ $# -eq 0 ]] || usage 1
        "$binary" register "$username" "$password"
        ;;

    add1000)
        [[ $# -le 1 ]] || usage 1
        task_date="${1:-$(date -d '+1 day' +%F)}"
        date -d "$task_date" +%F >/dev/null 2>&1 || {
            echo "ERROR: invalid date: $task_date" >&2
            exit 1
        }
        if ! login_check="$("$binary" "$username" "$password" showtask "$task_date" 2>&1)"; then
            echo "ERROR: login check failed before adding tasks." >&2
            echo "Run this script in the same working directory that contains the program's data/ folder." >&2
            printf '%s\n' "$login_check" >&2
            exit 1
        fi
        echo "Adding 1000 tasks for $username on $task_date..."
        echo "  Added 0/1000 tasks"
        for index in $(seq 0 999); do
            start_time="$(date -d "$task_date 00:00 + $index minutes" +%Y-%m-%dT%H:%M)"
            remind_time="$(date -d "$start_time - 5 minutes" +%Y-%m-%dT%H:%M)"
            task_name="bulk_task_$(printf '%04d' "$index")"
            if ! add_output="$("$binary" "$username" "$password" addtask "$task_name" "$start_time" medium study "$remind_time" 2>&1)"; then
                echo "ERROR: add task failed at item $index ($task_name)." >&2
                printf '%s\n' "$add_output" >&2
                exit 1
            fi
            if [[ $(((index + 1) % 50)) -eq 0 ]]; then
                echo "  Added $((index + 1))/1000 tasks"
            fi
        done
        echo "DONE: 1000 tasks added."
        ;;

    show)
        [[ $# -le 1 ]] || usage 1
        task_date="${1:-$(date -d '+1 day' +%F)}"
        "$binary" "$username" "$password" showtask "$task_date"
        ;;

    update)
        [[ $# -ge 3 && $# -le 6 ]] || usage 1
        task_id="$1"
        task_name="$2"
        start_time="$3"
        priority="${4:-medium}"
        category="${5:-life}"
        remind_time="${6:-$(date -d "$start_time - 5 minutes" +%Y-%m-%dT%H:%M)}"
        "$binary" "$username" "$password" updatetask "$task_id" "$task_name" "$start_time" "$priority" "$category" "$remind_time"
        ;;

    delete)
        [[ $# -eq 1 ]] || usage 1
        "$binary" "$username" "$password" deltask "$1"
        ;;

    *)
        echo "ERROR: unknown action: $action" >&2
        usage 1
        ;;
esac
