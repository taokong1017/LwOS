import argparse

def convert_ascii_art(ascii_art_input, output_file):
	lines = ascii_art_input.split('\n')
	processed_lines = []

	if len(lines) > 2:
		lines = lines[1:-1]

	for line in lines:
		orgin = line
		line = '	"' + line
		line = line.replace('\\', '\\\\')
		if orgin == lines[-1]:
			line += '\\n";'
		else:
			line += '\\n"\n'
		processed_lines.append(line)

	full_ascii_art = ''.join(processed_lines)
	full_ascii_art = '#include <stdio.h>\n\nconst char *logo =\n' + full_ascii_art
	full_ascii_art += "\n\n void logo_show() { printf(\"%s\", logo); }\n"

	with open(output_file, 'w') as f:
		f.write(full_ascii_art)

# ASCII字符图
ascii_art = '''
  _        _____      _____  _    _  ______  _       _
 | |      / ____|    / ____|| |  | ||  ____|| |     | |
 | |     | (___     | (___  | |__| || |__   | |     | |
 | |      \___ \     \___ \ |  __  ||  __|  | |     | |
 | |____  ____) |    ____) || |  | || |____ | |____ | |____
 |______||_____/    |_____/ |_|  |_||______||______||______|
'''

def main():
	parser = argparse.ArgumentParser(description='Remove the first and last lines from a file.')
	parser.add_argument('--output', '-o', type=str, default=None, help='The output C file.')

	args = parser.parse_args()
	output_file = args.output

	convert_ascii_art(ascii_art, output_file)

if __name__ == "__main__":
	main()
