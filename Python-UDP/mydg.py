#!/usr/bin/python
# -*- coding: utf-8 -*-

import socket;

UDP_IP = "127.0.0.1"
UDP_PORT = 1025
sock = socket.socket(socket.AF_INET,socket.SOCK_DGRAM) 
sock.bind((UDP_IP, UDP_PORT))
while True:
  data, addr = sock.recvfrom(1024) 
  k = data.index(chr(0))
  print data[0:k]

#
#
