#!/bin/bash

scriptBase="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd -P)"

function show_help() {
    echo "Usage: $0 [--build] [--debug] [--env] [--help] [--rebuild]."
    echo -e $PDEBUG"    -b | --build        "$PINFO"Compile."$PNORMAL
    echo -e $PDEBUG"    -c | --clean        "$PINFO"Clean build result."$PNORMAL
    echo -e $PDEBUG"    -i | --initialize   "$PINFO"Show this help menu." $PNORMAL
    echo -e $PDEBUG"    -h | --help         "$PINFO"Show this help menu." $PNORMAL
    echo -e $PDEBUG"    -e | --env          "$PINFO"Check env." $PNORMAL
    echo -e $PDEBUG"    -r | --rebuild      "$PINFO"Clean build result and build again." $PNORMAL
    exit 0
}

function main() {
    while [[ $# -gt 0 ]]; do
        case "$1" in
        -b | --build)
            build
            ;;
        -c | --clean)
            cleanup
            ;;
        -i | --initialize)
            initialize
            ;;
        -e | --env)
            env
            ;;
        -h | --help)
            show_help
            ;;
        -r | --rebuild)
            rebuild
            ;;
        *)
            echo -e "${PERROR}Invalid argument: $1${PNORMAL}"
            show_help
            ;;
        esac
        shift
    done
}

main "$@"
