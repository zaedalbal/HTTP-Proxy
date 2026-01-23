#!/usr/bin/env bash

set -euo pipefail

print_help()
{
    cat <<EOF
Usage:
  $0 <file> add <host>
  $0 <file> remove <host>
  $0 <file> check <host>
  $0 <file> list
  $0 --help

Description:
  Utility for managing blacklist hosts in TOML format.

Arguments:
  <file>    Path to blacklisted_hosts.toml
  <host>    Domain name (e.g. example.com)

Commands:
  add       Add host to blacklist
  remove    Remove host from blacklist
  check     Check if host exists in blacklist
  list      List all blacklisted hosts
EOF
}

usage()
{
    print_help
    exit 1
}

for arg in "$@"
do
    case "$arg" in
        --help|-h)
            print_help
            exit 0
            ;;
    esac
done

if [[ $# -lt 2 ]]
then
    usage
fi

BLACKLIST_FILE="$1"
COMMAND="$2"

ensure_file_exists()
{
    if [[ ! -f "$BLACKLIST_FILE" ]]
    then
        cat <<EOF > "$BLACKLIST_FILE"
[blacklist]
hosts = [
]
EOF
    fi
}

host_exists()
{
    local host="$1"
    grep -q "\"$host\"" "$BLACKLIST_FILE"
}

add_host()
{
    local host="$1"

    if host_exists "$host"
    then
        echo "Host already exists: $host"
        exit 0
    fi

    sed -i "/hosts = \[/,/]/ {
        /]/ i\    \"$host\",
    }" "$BLACKLIST_FILE"

    echo "Added host: $host"
}

remove_host()
{
    local host="$1"

    if ! host_exists "$host"
    then
        echo "Host not found: $host"
        exit 1
    fi

    sed -i "/\"$host\"/d" "$BLACKLIST_FILE"
    echo "Removed host: $host"
}

check_host()
{
    local host="$1"

    if host_exists "$host"
    then
        echo "Host is blacklisted: $host"
        exit 0
    else
        echo "Host is NOT blacklisted: $host"
        exit 1
    fi
}

list_hosts()
{
    awk '
    BEGIN { in_hosts = 0 }
    /^\s*hosts\s*=\s*\[/ { in_hosts = 1; next }
    in_hosts && /^\s*\]/ { exit }
    in_hosts {
        gsub(/^[ \t]*"/, "");    # убрать начало кавычки и пробелы
        gsub(/"[ \t]*,?[ \t]*$/, ""); # убрать конец кавычки и запятую
        if (length($0) > 0)
            print $0
    }' "$BLACKLIST_FILE"
}

ensure_file_exists

case "$COMMAND" in
    add)
        if [[ $# -ne 3 ]]
        then
            usage
        fi
        add_host "$3"
        ;;
    remove)
        if [[ $# -ne 3 ]]
        then
            usage
        fi
        remove_host "$3"
        ;;
    check)
        if [[ $# -ne 3 ]]
        then
            usage
        fi
        check_host "$3"
        ;;
    list)
        if [[ $# -ne 2 ]]
        then
            usage
        fi
        list_hosts
        ;;
    *)
        usage
        ;;
esac
