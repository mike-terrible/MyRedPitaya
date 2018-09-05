#!/usr/bin/python
# -*- coding: utf-8 -*-

import socket;

UDP_IP = "192.168.12.2"
UDP_PORT = 1026
sock = socket.socket(socket.AF_INET,socket.SOCK_DGRAM) 
sock.bind((UDP_IP, UDP_PORT))
while True:
  data, addr = sock.recvfrom(128) 
  k = data.index(chr(0))
  print data[0:k]

#
#
