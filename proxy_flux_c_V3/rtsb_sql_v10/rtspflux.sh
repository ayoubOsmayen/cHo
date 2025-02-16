#!/bin/bash

if [ "$#" -ne 6 ]; then
    echo "Usage: $0 <rtsp_source> <mount_point> <destination_port> <login> <password> <overlay_text>"
    exit 1
fi

RTSP_SOURCE=$1
MOUNT_POINT=$2
DEST_PORT=$3
LOGIN=$4
PASSWORD=$5
OVERLAY_TEXT=$6

./gstream_project $RTSP_SOURCE $MOUNT_POINT $DEST_PORT $LOGIN $PASSWORD $OVERLAY_TEXT
