#include <stddef.h>
#include <span.h>

ssize_t sp_find(span_t *span, size_t index, size_t size) {
  if (span->nodes[index].max_free == span->nodes[index].size) {
    return index;
  } else if (span->nodes[index].max_free < size) {
    return -1;
  }
  
  if (span->nodes[2 * index + 1].max_free >= size) {
    return sp_find(span, 2 * index + 1, size);
  } else {
    return sp_find(span, 2 * index + 2, size);
  }
}

size_t sp_index_by_offset(span_t *span, size_t index, size_t offset) {
  if (!span->nodes[index].max_free || span->nodes[index].max_free == span->nodes[index].size) {
    return 0;
  }
  
  if (offset >= span->nodes[2 * index + 1].size) {
    return sp_index_by_offset(span, 2 * index + 2, offset - span->nodes[2 * index + 1].size);
  } else {
    return sp_index_by_offset(span, 2 * index + 1, offset);
  }
}

size_t sp_offset_by_index(span_t *span, size_t index) {
  
}

ssize_t sp_previous(span_t *span, size_t index) {
  size_t previous = index;
  
  while (previous) {
    size_t parent = (previous - 1) / 2;
    
    if (previous == 2 * parent + 2) {
      previous--;
      break;
    }
    
    previous = parent;
  }
  
  while (!span->nodes[previous].max_free || span->nodes[previous].max_free == span->nodes[previous].size) {
    previous = 2 * previous + 2;
  }
  
  if (previous == index) {
    return -1;
  }
  
  return previous;
}

ssize_t sp_next(span_t *span, size_t index) {
  size_t next = index;
  
  while (next) {
    size_t parent = (next - 1) / 2;
    
    if (next == 2 * parent + 1) {
      next++;
      break;
    }
    
    next = parent;
  }
  
  while (!span->nodes[next].max_free || span->nodes[next].max_free == span->nodes[next].size) {
    next = 2 * next + 1;
  }
  
  if (next == index) {
    return -1;
  }
  
  return next;
}

void sp_mark_used(span_t *span, size_t index, size_t size) {
  if (span->nodes[index].size == size) {
    span->nodes[index].max_size = 0;
    return;
  }
  
  span->nodes[index].max_size = span->nodes[index].size - size;
  
  span->nodes[2 * index + 1].max_size = 0, span->nodes[2 * index + 1].size = size;
  span->nodes[2 * index + 2].max_size = span->nodes[2 * index + 2].size = span->nodes[index].max_size;
  
  ssize_t next = sp_next(span, index);
  
  if (next >= 0 && span->nodes[next].max_size) {
    sp_merge(span, index, next);
  }
}

void sp_mark_free(span_t *span, size_t index) {
  
}
