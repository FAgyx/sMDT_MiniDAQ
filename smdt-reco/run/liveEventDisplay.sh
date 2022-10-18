#!/bin/bash


while [ true ]; 
do
    rm output/liveEventDisplay/*.png
    ./build/liveEventDisplay --conf $1
    status=$?
    echo $status
    if [ $status == 0 ]; then
	mv `ls output/liveEventDisplay/event_id*.png` output/liveEventDisplay/liveimage.png
	cp output/liveEventDisplay/liveimage.png /eos/home-k/kenelson/www/smdt-event-display/img
	cp output/liveEventDisplay/img_date.txt /eos/home-k/kenelson/www/smdt-event-display/img
    else
	echo "not "
    fi
    sleep 5
done

