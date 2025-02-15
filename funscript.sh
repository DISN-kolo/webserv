#!/bin/bash

mkdir -p /tmp/var/www/
if [ ! -f /tmp/var/www/filesforserver ]; then
	ln -s /home/akozin/code/webserv/filesforserver /tmp/var/www/filesforserver
fi
rm filesforserver/funscript.sh
