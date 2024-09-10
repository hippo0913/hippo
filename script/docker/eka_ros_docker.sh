#!/bin/bash

# script env
PBOLD='\e[1m'     # 字体加粗
PNO_COLOR='\e[0m' # 重置文本颜色为默认颜色，表示 NORMAL
PBLUE='\e[34m'    # 洋红色表示 DEBUG
PTEAL='\e[36m'    # 青色表示 INFORMATION
PYELLOW='\e[33m'  # 黄色表示 WARNING
PRED='\e[31m'     # 红色表示 ERROR

# Exit immediately if a command exits with a non-zero status
set -e

# custom
DOCKER_SERVER=harbor.gpal.dev
DOCKER_CONTAINER_NAME=eka_ros_docker
DOCKER_IMAGE="${DOCKER_SERVER}/devops/${DOCKER_CONTAINER_NAME}"
DOCKER_TAG=V1

function _usage() {
    echo -e "\n${PRED}Usage${PNO_COLOR}:
        .${PBOLD}/eka_ros_docker.sh${PNO_COLOR} [OPTION]"
    echo -e "\n${PRED}Options${PNO_COLOR}:
        ${PBLUE}-h, --help[options]${PNO_COLOR}:    Show this usage message and exit."
    echo -e "\n${PRED}Commands:${PNO_COLOR}:
        ${PBLUE}account --help[options]${PNO_COLOR}:  Show account Usage.
        ${PBLUE}image --help[options]${PNO_COLOR}:  Show image Usage.
        ${PBLUE}docker --help[options]${PNO_COLOR}: Show docker Usage.
        ${PTEAL}Call \`$0 <command> \` for more detailed usage.${PNO_COLOR}
        "
    exit 0
}

function _account_usage() {
    echo -e "\n${PRED}Usage${PNO_COLOR}:
        .${PBOLD}/eka_ros_docker.sh account${PNO_COLOR} [OPTION]"
    echo -e "\n${PRED}Options${PNO_COLOR}:
        ${PBLUE}-h, --help[options]${PNO_COLOR}:    Show this usage message and exit."
    echo -e "\n${PRED}Commands:${PNO_COLOR}:
        ${PBLUE}login${PNO_COLOR}: Login ${DOCKER_SERVER}.
        ${PBLUE}login${PNO_COLOR}: Loout ${DOCKER_SERVER}.
        "
    exit 0
}

function manager_account() {
    if [ "$#" -eq 0 ]; then
        _account_usage
    fi

    case "$1" in
    login)
        local exec_cmd="docker login ${DOCKER_SERVER}"
        echo -e "\n${PYELLOW}Login gpal docker server, exec ${PRED}${exec_cmd}${PYELLOW} below${PNO_COLOR}.\n"
        eval ${exec_cmd}
        ;;
    logout)
        local exec_cmd="docker logout"
        echo -e "\n${PYELLOW}Logout gpal docker server, exec ${PRED}${exec_cmd}${PYELLOW} below${PNO_COLOR}.\n"
        eval ${exec_cmd}
        ;;
    -h | --help)
        _account_usage
        ;;
    *)
        echo -e "\n${PYELLOW}Invalid argument: $1.${PNO_COLOR}\n"
        _account_usage
        ;;
    esac
}

function _image_usage() {
    echo -e "\n${PRED}Usage${PNO_COLOR}:
        .${PBOLD}/eka_ros_docker.sh image${PNO_COLOR} [OPTION]"
    echo -e "\n${PRED}Options${PNO_COLOR}:
        ${PBLUE}-h, --help[options]${PNO_COLOR}:    Show this usage message and exit."
    echo -e "\n${PRED}Commands:${PNO_COLOR}:
        ${PBLUE}list${PNO_COLOR}:   Show all docker images on machine.
        ${PBLUE}pull${PNO_COLOR}:   Pull docker image from ${DOCKER_SERVER}.
        ${PBLUE}remove${PNO_COLOR}: Remove docker image.
        ${PBLUE}load${PNO_COLOR}:   Load docker image.
        "
    exit 0
}

function manage_images() {
    case "$1" in
    list)
        local exec_cmd="docker images"
        echo -e "\n${PTEAL}Show all docker images, exec ${PRED}${exec_cmd}${PTEAL} below${PNO_COLOR}.\n"
        eval "${exec_cmd}"
        ;;
    pull)
        if [ -z "$2" ]; then
            echo -e "${PRED}Please specify an image to pull !!!${PNO_COLOR}"
        else
            local exec_cmd="docker pull $2"
            echo -e "${PTEAL}Pulling Docker Image: $2 exec ${PRED}${exec_cmd}${PTEAL} below${PNO_COLOR}.\n"
            eval "${exec_cmd}"
        fi
        ;;
    load)
        if [ -z "$2" ]; then
            echo -e "${PRED}Please specify an image to load !!!${PNO_COLOR}"
        else
            local exec_cmd="docker load -i $2"
            echo -e "${PTEAL}Loading Docker Image: $2 exec ${PRED}${exec_cmd}${PTEAL} below${PNO_COLOR}.\n"
            eval "${exec_cmd}"
        fi
        ;;
    remove)
        if [ -z "$2" ]; then
            echo -e "${PRED}Please specify an image to remove !!!${PNO_COLOR}"
        else
            local exec_cmd="docker rmi $2"
            echo -e "${PTEAL}Removing docker image: $2 exec ${PRED}${exec_cmd}${PTEAL} below${PNO_COLOR}.\n"
            eval "${exec_cmd}"
        fi
        ;;
    -h | --help)
        _image_usage
        ;;
    *)
        echo -e "${PYELLOW}Invalid docker image argument: $1${PNO_COLOR}."
        _image_usage
        ;;
    esac
}

function _get_docker_image_id() {
    local image_id=$(docker images --format "{{.Repository}}:{{.Tag}} {{.ID}}" | grep "^$DOCKER_IMAGE:$DOCKER_TAG " | awk '{print $2}')

    if [ -z "$image_id" ]; then
        echo -e "${PRED}Image not found: $DOCKER_IMAGE:$DOCKER_TAG${PNO_COLOR}"
    else
        echo "$image_id"
    fi
}

function _get_docker_container_id() {
    local image_id=$(get_docker_image_id)

    mapfile -t container_ids < <(docker ps -a --filter "ancestor=$image_id" --format "{{.ID}}")

    if [ ${#container_ids[@]} -ne 1 ]; then
        exit 1
    else
        echo "${container_ids[0]}"
    fi
}

function _container_usage() {
    echo -e "\n${PRED}Usage${PNO_COLOR}:
        .${PBOLD}/eka_ros_docker.sh container${PNO_COLOR} [OPTION]"
    echo -e "\n${PRED}Options${PNO_COLOR}:
        ${PBLUE}-h, --help[options]${PNO_COLOR}:    Show this usage message and exit."
    echo -e "\n${PRED}Commands:${PNO_COLOR}:
        ${PBLUE}list${PNO_COLOR}:   Show all docker container on machine.
        "
    exit 0
}

function manage_containers() {
    case "$1" in
    list)
        local exec_cmd="docker ps -a"
        echo -e "\n${PTEAL}Show all docker containers, exec ${PRED}${exec_cmd}${PTEAL} below${PNO_COLOR}.\n"
        eval "${exec_cmd}"
        ;;
    run)
        local exec_cmd="docker run -itd \
            -h ${DOCKER_CONTAINER_NAME} \
            -w /work \
            -v $(pwd):/work \
            --network=host \
            --shm-size=\"1g\" \
            --privileged \
            --name ${DOCKER_CONTAINER_NAME} \
            ${DOCKER_CONTAINER_NAME}:${DOCKER_TAG} /bin/bash"
        echo -e "${PTEAL}Run Docker container: $2 exec ${PRED}${exec_cmd}${PTEAL} below${PNO_COLOR}.\n"
        eval "${exec_cmd}"
        ;;
    remove)
        local container_id = $(_get_docker_container_id)
        local exec_cmd="docker rm ${container_id}"
        echo -e "${PTEAL}Removing docker container: exec ${PRED}${exec_cmd}${PTEAL} below${PNO_COLOR}.\n"
        eval "${exec_cmd}"
        ;;
    exec)
        local container_id = $(_get_docker_container_id)
        local exec_cmd="docker exec -it ${container_id} /bin/bash"
        echo -e "${PTEAL}Executing docker container: exec ${PRED}${exec_cmd}${PTEAL} below${PNO_COLOR}.\n"
        eval "${exec_cmd}"
        ;;
    start)
        local container_id = $(_get_docker_container_id)
        local exec_cmd="docker start ${container_id}"
        echo -e "${PTEAL}Staring docker container: exec ${PRED}${exec_cmd}${PTEAL} below${PNO_COLOR}.\n"
        eval "${exec_cmd}"
        ;;
    status)
        local container_id = $(_get_docker_container_id)
        local exec_cmd="docker inspect --format='{{.State.Status}}' ${container_id}"
        echo -e "${PTEAL}Quering docker container status: exec ${PRED}${exec_cmd}${PTEAL} below${PNO_COLOR}.\n"
        eval "${exec_cmd}"
        ;;
    stop)
        local container_id = $(_get_docker_container_id)
        local exec_cmd="docker stop ${container_id}"
        echo -e "${PTEAL}Stoping docker container: exec ${PRED}${exec_cmd}${PTEAL} below${PNO_COLOR}.\n"
        eval "${exec_cmd}"
        ;;
    -h | --help)
        _container_usage
        ;;
    *)
        echo -e "${PYELLOW}Invalid docker container argument: $1${PNO_COLOR}."
        _container_usage
        ;;
    esac
}

function main() {
    if [ "$#" -eq 0 ]; then
        _usage
        exit 0
    fi

    local cmd="$1"
    shift
    case "$cmd" in
    account)
        manager_account "$@"
        ;;
    image)
        manage_images "$@"
        ;;
    container)
        manage_containers "$@"
        ;;
    -h | --help)
        _usage
        ;;
    *)
        _usage
        ;;
    esac
}

main "$@"
