import sys
import argparse
import json
import os
import re
from collections import OrderedDict
from elftools.elf.elffile import ELFFile
from elftools.elf.sections import SymbolTableSection
import elftools.common.exceptions

SZ = 'size'
SRC = 'sources'
section_regex = re.compile(r'.([A-Za-z0-9_]*)_(data|bss)')
section_template = """
.{name} ALIGN({align_size}):
{{
	. = ALIGN({align_size});
	__{name}_data_start = .;	/* define a global symbol at app data start */
	*(.{name}_data)				/* app data sections */
	. = ALIGN(0x1000);
	__{name}_data_end = .;		/* define a global symbol at app data end */

	__{name}_bss_start = .;		/* define a global symbol at app bss start */
	*(.{name}_bss)				/* app bss sections */
	. = ALIGN(0x1000);
	__{name}_bss_end = .;		/* define a global symbol at app bss end */
}}
"""

def parse_args():
	global args
	parser = argparse.ArgumentParser(
		description=__doc__,
		formatter_class=argparse.RawDescriptionHelpFormatter, allow_abbrev=False)
	parser.add_argument("-d", "--directory", required=True, default=None, help="Build directory")
	parser.add_argument("-v", "--verbose", required=False, default=0, help="Verbose Output")
	parser.add_argument("-o", "--output", required=True, help="Output ld file")
	parser.add_argument("-s", "--size", required=False, default=0x1000, help="Page size")
	args = parser.parse_args()


def find_obj_file_partitions(filename, partitions):
	with open(filename, 'rb') as f:
		try:
			full_lib = ELFFile(f)
		except elftools.common.exceptions.ELFError as e:
			exit(f"Error: {filename}: {e}")

		if not full_lib:
			sys.exit("Error parsing file: " + filename)

		sections = [x for x in full_lib.iter_sections()]
		for section in sections:
			m = section_regex.match(section.name)
			if not m:
				continue

			partition_name = m.groups()[0]
			if partition_name not in partitions:
				partitions[partition_name] = {SZ: section.header.sh_size}

				if args.verbose:
					partitions[partition_name][SRC] = filename

			else:
				partitions[partition_name][SZ] += section.header.sh_size

	return partitions


def parse_obj_files(partitions):
	# Iterate over all object files to find partitions
	for dirpath, _, files in os.walk(args.directory):
		for filename in files:
			if re.match(r".*\.o$", filename):
				fullname = os.path.join(dirpath, filename)
				fsize = os.path.getsize(fullname)
				if fsize != 0:
					find_obj_file_partitions(fullname, partitions)
	if args.verbose:
		print(partitions)


def generate_linker(file, partitions):
	content = ""

	if len(partitions) > 0:
		for partition, item in partitions.items():
			content += section_template.format(name=partition, align_size=args.size)

	with open(file, "w") as f:
		f.write(content)

def main():
	parse_args()
	partitions = {}

	if args.directory is None:
		return
	
	parse_obj_files(partitions)
	generate_linker(args.output, partitions)


if __name__ == '__main__':
	main()
