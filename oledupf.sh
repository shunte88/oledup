#!/bin/sh

logfile=/tmp/oledupf.log
sudo rm -f $logfile
sudo touch $logfile
sudo chmod 777 $logfile

exec 3>&2
log() {
  datestring=`date +'%Y-%m-%d %H:%M:%S'`
  # Expand escaped characters, wrap at 70 chars, indent wrapped lines
  logtext=$(echo "$datestring $1" | fold -w70 -s | sed '2~1s/^/  /')
  echo "$logtext" >> $logfile
  echo "$logtext" >&3
}

log "checking network..."
until netup=$( ifconfig -s | grep -E "^(wlan|eth).*BMRU$" )
do
  log "Waiting on network ..."
  sleep 1
done

log "$netup"
cd /home/pi/oledup/
./sysinfoled >> $logfile 2>&1
# sits and waits
