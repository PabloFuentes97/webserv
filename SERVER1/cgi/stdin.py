#!/usr/bin/dev python3

import ios, cgi, os, sys

content_length = int(os.environ.get("CONTENT_LENGTH", 0))

data = sys.stdin.read(content_length)

print(f"<html><body>{data}</body></html>")