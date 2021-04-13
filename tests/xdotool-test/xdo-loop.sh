#!/bin/bash
old_pid=""
hidraw="../hidraw-test/anykey-hidraw-test"
device="/dev/hidraw2"

set_layer() {
  case $1 in
    "telegram-desktop")
    ;&
    "thunderbird")
    ;&
    "firefox")
    LAYER="tluafed"
    ;;
    "hexchat")
    ;&
    *)
    LAYER="default"
    ;;
  esac
  echo "got window $1, setting layer $LAYER"
  $hidraw -D $device -C set-layer -l $LAYER
}

while read -r pid
do
	proc=$(ls -l /proc/$pid/exe | rev | cut -d "/" -f1 | rev)
	[ "$pid" != "$old_pid" ] && set_layer $proc
	old_pid=$pid
done <  <(xdotool search --desktop --onlyvisible --class . behave %@ focus getwindowpid)
