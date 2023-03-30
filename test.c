#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <span.h>

int main(void) {
  span_t span_struct;
  sp_init(&span_struct, 256);
  
  span_t *span = malloc(sizeof(span_t) + span_struct.size_8);
  memcpy(span, &span_struct, sizeof(span_t));
  
  printf("Span needs %llu bytes, with a ratio of 1 to %llu.\n", span->size_8, (span->size << 12) / span->size_8);
  
  printf("64-bit entries span from 0 to %llu.\n", span->end_64);
  printf("32-bit entries span from %llu to %llu.\n", span->end_64, span->end_32);
  printf("16-bit entries span from %llu to %llu.\n", span->end_32, span->end_16);
  printf("8-bit entries span from %llu to %llu.\n", span->end_16, span->end_8);
  
  sp_clear(span, 0);
  
  for (int i = 0; i < 255; i++) {
    sp_node_t node = sp_read(span, i);
    printf("%d: (", i);
    
    for (int j = 0; j < 6; j++) {
      if (j) {
        putchar(' ');
      }
      
      printf("%llu", node.data[j]);
    }
    
    printf(")\n");
  }
  
  return 0;
}
