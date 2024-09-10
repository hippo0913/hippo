#!/bin/bash

hippoScript="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd -P)"
source ${hippoScript}/hippo_script_base.sh

common_ensure_folder_exists $hippoBin
common_ensure_folder_exists $hippoLog

cmake_bin=$hippoTools/cmake/bin

# cmake
function build_cmake() {
    echo -e $PINFO "\b\c"

    local cmake_folder="cmake-3.27.9-linux-x86_64"
    local cmake_tar_name="${cmake_folder}.tar.gz"
    local cmake_url="https://cmake.org/files/v3.27/${cmake_tar_name}"

    common_ensure_folder_exists $hippoTools
    pushd $hippoTools >/dev/null
    if [ ! -d ${cmake_bin} ]; then
        if [ ! -d $cmake_folder ]; then
            if [ ! -f $cmake_tar_name ]; then
                curl -O ${cmake_url}
                tar -zxf "$cmake_tar_name" -C "$hippoTools"
                rm $cmake_tar_name
            fi
        fi
        ln -s $cmake_folder cmake
    fi
    popd >/dev/null
    echo -e "\b\c"
}

function export_cmake() {
    build_cmake
    export PATH=${cmake_bin}${PATH:+:${PATH}}
    cmake_version=$(cmake --version | grep -oP '(\d+\.\d+\.\d+)')
    echo -e "${PINFO}cmake_version= ${cmake_version}${PNORMAL}"
}

export_cmake

# cuda
CUDA_HOME=/usr/local/cuda
CUDNN_HOME=/usr/local/cuda

function export_hpc() {
    export CUDA_HOME CUDNN_HOME
}

export_hpc
