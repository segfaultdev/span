#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main(int argc, char **argv) {
	int w, h, unused;
	uint8_t *array = stbi_load(argv[1], &w, &h, &unused, 1);
	
	if (strrchr(argv[1], '/') != NULL) {
		argv[1] = strrchr(argv[1], '/') + 1;
	}
	
	char *size = strrchr(argv[1], '_');
	
	size++;
	const int char_w = strtol(size, &size, 10);
	
	size++;
	const int char_h = strtol(size, &size, 10);
	
	*size = '\x00';
	
	const int char_a = w / char_w;
	const int char_b = h / char_h;
	
	uint16_t *data = malloc((((char_w + 15) & ~15) >> 3) * char_a * h);
	int i = 0;
	
	for (int q = 0; q < char_b; q++) {
		for (int p = 0; p < char_a; p++) {
			for (int y = 0; y < char_h; y++) {
				uint16_t d = 0;
				int j = 0;
				
				for (int x = 0; x < ((char_w + 15) & ~15); x++) {
					const int u = x + p * char_w;
					const int v = y + q * char_h;
					
					uint16_t b = 0;
					
					if (x < char_w) {
						b = ((array[u + v * w] >= 128) ? 1 : 0);
					}
					
					d |= (b << j);
					j++;
					
					if (j >= 16) {
						data[i] = d;
						i++;
						
						d = 0;
						j = 0;
					}
				}
			}
		}
	}
	
	int char_start;
	
	if (char_a * char_b <= 16) {
		char_start = 48;
	} else if (char_a * char_b <= 96) {
		char_start = 32;
	} else {
		char_start = 0;
	}
	
	char path[100];
	snprintf(path, 100, "font_%s.c", argv[1]);
	
	FILE *file = fopen(path, "w");
	
	fprintf(file, "#include <stdint.h>\n");
	fprintf(file, "#include <span.h>\n\n");
	fprintf(file, "static const uint16_t array[] = {");
	
	for (int j = 0; j < i; j++) {
		if ((j & 15) == 0) {
			fprintf(file, "\n\t");
		} else {
			fprintf(file, " ");
		}
		
		fprintf(file, "0x%04X,", data[j]);
	}
	
	fprintf(file, "\n};\n\n");
	fprintf(file, "const span_font_t font_%s = (span_font_t) {\n", argv[1]);
	fprintf(file, "\t.array = array,\n");
	fprintf(file, "\t.w = %d,\n", char_w);
	fprintf(file, "\t.h = %d,\n", char_h);
	fprintf(file, "\t.start = %d,\n", char_start);
	fprintf(file, "\t.n = %d,\n", char_a * char_b);
	fprintf(file, "};\n");
	
	fclose(file);
	
	printf("extern const span_font_t font_%s;\n", argv[1]);
	return 0;
}
