#!/bin/bash
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/OSDiagnosticSystem/

#if [[ "$(sudo systemctl status sshd.service 2>&1 |  grep 'active (running)')" == "" ]]; then  echo "sshd no runing...reload"; sudo service ssh start | sudo service sshd start;  else echo "sshd running"; sudo service ssh restart | sudo service sshd restart;  fi;

exec /opt/OSDiagnosticSystem/OSDiagnosticSystem -e
