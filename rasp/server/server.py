#!/usr/bin/python3

#python2

#import BaseHTTPServer
#import CGIHTTPServer

#python3

import http.server 
import http.cookies
import cgi

#import cgitb; cgitb.enable()
    

server = http.server.HTTPServer
handler = http.server.CGIHTTPRequestHandler
server_address = ("", 8080)
handler.cgi_directories = ["/cgi-bin"]

httpd = server(server_address, handler)
httpd.serve_forever()
