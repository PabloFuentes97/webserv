#!/usr/bin/env python3

import os, cgi, shutil

def copy_file_to_directory(file_path, destination_directory):

	if not os.access(file_path, os.R_OK):
		exit (2)

	file_name = os.path.basename(file_path)

	destination_path = os.path.join(destination_directory, file_name)

	try:
		shutil.copy2(file_path, destination_path)
		print(f"File copied successfully to {destination_path}")
	except Exception as e:
		print(f"Error copying file: {e}")
		exit (1)

absolute_path = os.environ.get('PATH_INFO', '')
destination_directory = os.getcwd() + "/SERVER2/cgi_upload"

copy_file_to_directory(absolute_path, destination_directory)

