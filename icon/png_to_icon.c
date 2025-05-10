#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main(int argc, char **argv) {
	int w, h, unused;
	uint8_t *array = stbi_load(argv[1], &w, &h, &unused, 2);
	
	if (strrchr(argv[1], '/') != NULL) {
		argv[1] = strrchr(argv[1], '/') + 1;
	}
	
	char *size = strrchr(argv[1], '_');
	
	size++;
	const int icon_w = strtol(size, &size, 10);
	
	size++;
	const int icon_h = strtol(size, &size, 10);
	
	*size = '\x00';
	
	uint16_t *data = malloc((((icon_w + 15) & ~15) >> 3) * icon_h);
	int i = 0;
	
	for (int y = 0; y < icon_h; y++) {
		uint16_t d = 0;
		int j = 0;

		for (int x = 0; x < ((icon_w + 15) & ~15); x++) {
			uint16_t b = 0;

			if (x < icon_w) {
				const int u = ((int)(array[(x + y * w) * 2 + 0]) * (int)(array[(x + y * w) * 2 + 1])) / 256;
				b = ((u >= 96) ? 1 : 0);
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
	
	char path[100];
	snprintf(path, 100, "icon_%s.c", argv[1]);
	
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
	fprintf(file, "const span_icon_t icon_%s = (span_icon_t) {\n", argv[1]);
	fprintf(file, "\t.type = SPAN_ICON_MASK,\n");
	fprintf(file, "\t.array = array,\n");
	fprintf(file, "\t.w = %d,\n", icon_w);
	fprintf(file, "\t.h = %d,\n", icon_h);
	fprintf(file, "};\n");
	
	fclose(file);
	
	printf("extern const span_icon_t icon_%s;\n", argv[1]);
	return 0;
}
