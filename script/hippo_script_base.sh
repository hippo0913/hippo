#!/bin/bash

# script env
PNORMAL='\e[0m'   # 重置文本颜色为默认颜色，表示 NORMAL
PDEBUG='\e[35m'   # 洋红色表示 DEBUG
PINFO='\e[36m'    # 青色表示 INFORMATION
PWARNING='\e[33m' # 黄色表示 WARNING
PERROR='\e[31m'   # 红色表示 ERROR

hippoScript=$(
    cd $(dirname ${BASH_SOURCE:-$0})
    pwd
)

hippoBase=$hippoScript/../
hippoBase=$(realpath "$hippoBase")
hippoBin=$hippoBase/bin
hippoCode=$hippoBase/code
hippoLog=$hippoBase/log
hippoTools=$hippoBase/tools

echo -e $PDEBUG "\b\c"
echo "hippoBase = $hippoBase"
echo "hippoBin = $hippoBin"
echo "hippoCode = $hippoCode"
echo "hippoScript = $hippoScript"
echo "hippoLog = $hippoLog"
echo "hippoTools = $hippoTools"
echo -e $PNORMAL "\b\c"

function common_ensure_folder_exists() {
    if [ ! -d "$1" ]; then
        mkdir -p $1
    fi
}

function check_file_exists() {
    if [ ! -f "$1" ]; then
        echo -e "${PERROR}Error: $1 does not exist.${PNORMAL}"
        exit 1
    fi
}

function array_to_string() {
    local -n array=$1
    local string=""
    for element in "${array[@]}"; do
        string+="$element "
    done
    echo "$string"
}

function find_code_includes() {
    find "$@" -type f -name "*.hpp" \
        -or -name "*.h"
}

function find_code_srcs() {
    find "$@" -type f -name "*.cpp" \
        -or -name "*.cc" \
        -or -name "*.c"
}

function find_proto_srcs() {
    find "$@" -type f -name "*.proto"
}

function find_py_srcs() {
    find "$@" -type f -name "*.py"
}
