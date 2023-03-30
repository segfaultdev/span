#ifndef __SPAN_H__
#define __SPAN_H__

#include <stddef.h>

typedef struct sp_node_t sp_node_t;
typedef struct span_t span_t;

struct sp_node_t {
  size_t size, max_free;
};

struct span_t {
  sp_node_t nodes[];
};

ssize_t sp_find(span_t *span, size_t index, size_t size);

size_t sp_index_by_offset(span_t *span, size_t index, size_t offset);
size_t sp_offset_by_index(span_t *span, size_t index);

ssize_t sp_previous(span_t *span, size_t index);
ssize_t sp_next(span_t *span, size_t index);

void sp_merge()

void sp_mark_used(span_t *span, size_t index, size_t size);
void sp_mark_free(span_t *span, size_t index);

#endif
