#ifndef __SPAN_H__
#define __SPAN_H__

#include <stddef.h>
#include <stdint.h>

typedef struct sp_node_t sp_node_t;
typedef struct span_t span_t;

struct sp_node_t {
  union {
    struct {
      size_t free, span, left, right;
      size_t lazy_start, lazy_end;
    };
    
    size_t data[6];
  };
  
  int is_dirty;
  size_t width;
};

struct span_t {
  size_t size;
  
  size_t size_64, size_32, size_16, size_8;
  size_t end_64, end_32, end_16, end_8;
  
  uint8_t data[];
};

void   sp_init(span_t *span, size_t size);
size_t sp_get_width(span_t *span, size_t index);

sp_node_t sp_read(span_t *span, size_t index);
void      sp_write(span_t *span, size_t index, sp_node_t node);

int  sp_check(span_t *span, size_t index, size_t offset);
void sp_clear(span_t *span, size_t index);
void sp_apply(span_t *span, size_t index);
void sp_mark(span_t *span, size_t start, size_t end);

#endif
