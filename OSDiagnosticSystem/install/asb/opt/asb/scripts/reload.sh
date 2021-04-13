#!/bin/bash

host="localhost"; cnt=0; res=""; while IFS=  read -r line_par; do if [ "$cnt" -gt  0 ]; then  res+=", ";  fi;  res+=$line_par; cnt=$[$cnt + 1]; done <<< $(sudo ald-admin host-list); if [ "$cnt" -gt  0 ]; then host=$res; fi; sudo sed -i 's/#hosts/'"$host"'/g' /opt/asb/scripts/templates >/dev/null 2>/dev/null

if [[ -e /opt/asb/scripts/config ]]; then  sudo /bin/bash /opt/asb/scripts/config; fi;

sudo rm /var/cache/debconf/*.dat;
