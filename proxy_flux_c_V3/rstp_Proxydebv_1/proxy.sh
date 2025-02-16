#!/bin/bash

if [ "$#" -ne 5 ]; then
    echo "Usage: $0 <rtsp_source> <mount_point> <destination_port> <login> <password>"
    exit 1
fi

RTSP_SOURCE=$1
MOUNT_POINT=$2
DEST_PORT=$3
LOGIN=$4
PASSWORD=$5

./serv_rtsp $RTSP_SOURCE $MOUNT_POINT $DEST_PORT $LOGIN $PASSWORD
