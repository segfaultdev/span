#include <stddef.h>
#include <stdint.h>
#include <span.h>

#define SP_MAX(x, y) ({size_t _x = (x), _y = (y); (_x > _y ? _x : _y);})

void sp_init(span_t *span, size_t size) {
  int log_size = (sizeof(size_t) * 8) - __builtin_clzll(size - 1) - 1;
  span->size = size;
  
  span->size_64 = span->size_32 = span->size_16 = span->size_8 = 0;
  span->end_64 = span->end_32 = span->end_16 = span->end_8 = 0;
  
  for (int i = 0; log_size >= 0 && i < 8; i++) {
    span->size_8 += 6 * (1ull << log_size);
    span->end_8 += (1ull << log_size);
    
    log_size--;
  }
  
  for (int i = 0; log_size >= 0 && i < 8; i++) {
    span->size_16 += 12 * (1ull << log_size);
    span->end_16 += (1ull << log_size);
    
    log_size--;
  }
  
  for (int i = 0; log_size >= 0 && i < 16; i++) {
    span->size_32 += 24 * (1ull << log_size);
    span->end_32 += (1ull << log_size);
    
    log_size--;
  }
  
  for (int i = 0; log_size >= 0 && i < 32; i++) {
    span->size_64 += 48 * (1ull << log_size);
    span->end_64 += (1ull << log_size);
    
    log_size--;
  }
  
  span->size_32 += span->size_64;
  span->size_16 += span->size_32;
  span->size_8 += span->size_16;
  
  span->end_32 += span->end_64;
  span->end_16 += span->end_32;
  span->end_8 += span->end_16;
  
  span->last_index = (size_t)(-1);
}

size_t sp_get_width(span_t *span, size_t index) {
  return (2ull << (__builtin_clzll(index + 1) - __builtin_clzll(span->size - 1)));
}

sp_node_t sp_read(span_t *span, size_t index) {
  if (index == span->last_index) {
    return span->last_node;
  }
  
  sp_node_t node;
  node.width = sp_get_width(span, index);
  
  if (index < span->end_64) {
    uint64_t *data = (uint64_t *)(span->data) + (index * 6);
    
    for (int i = 0; i < 6; i++) {
      node.data[i] = data[i];
    }
  } else if (index < span->end_32) {
    uint32_t *data = (uint32_t *)(span->data + span->size_64) + ((index - span->end_64) * 6);
    
    for (int i = 0; i < 6; i++) {
      node.data[i] = data[i];
    }
  } else if (index < span->end_16) {
    uint16_t *data = (uint16_t *)(span->data + span->size_32) + ((index - span->end_32) * 6);
    
    for (int i = 0; i < 6; i++) {
      node.data[i] = data[i];
    }
  } else {
    uint8_t *data = (uint8_t *)(span->data + span->size_16) + ((index - span->end_16) * 6);
    
    for (int i = 0; i < 6; i++) {
      node.data[i] = data[i];
    }
  }
  
  if (node.left + node.right >= node.width) {
    node.free = node.span = node.left = node.right = node.width;
  }
  
  if (node.lazy_end < node.lazy_start) {
    node.is_dirty = 0;
  } else {
    node.is_dirty = 1;
    node.lazy_end++;
  }
  
  span->last_index = index;
  return (span->last_node = node);
}

void sp_write(span_t *span, size_t index, sp_node_t node) {
  span->last_index = index;
  span->last_node = node;
  
  if (node.free == node.width) {
    node.free = node.span = node.left = node.right = node.width - 1;
  }
  
  if (node.is_dirty) {
    node.lazy_end--;
  } else {
    node.lazy_start = node.width - 1;
    node.lazy_end = 0;
  }
  
  if (index < span->end_64) {
    uint64_t *data = (uint64_t *)(span->data) + (index * 6);
    
    for (int i = 0; i < 6; i++) {
      data[i] = node.data[i];
    }
  } else if (index < span->end_32) {
    uint32_t *data = (uint32_t *)(span->data + span->size_64) + ((index - span->end_64) * 6);
    
    for (int i = 0; i < 6; i++) {
      data[i] = node.data[i];
    }
  } else if (index < span->end_16) {
    uint16_t *data = (uint16_t *)(span->data + span->size_32) + ((index - span->end_32) * 6);
    
    for (int i = 0; i < 6; i++) {
      data[i] = node.data[i];
    }
  } else {
    uint8_t *data = (uint8_t *)(span->data + span->size_16) + ((index - span->end_16) * 6);
    
    for (int i = 0; i < 6; i++) {
      data[i] = node.data[i];
    }
  }
}

void sp_clear(span_t *span, size_t index) {
  if (2 * index + 2 >= span->end_8) {
    sp_write(span, index, (sp_node_t) {
      .free = 2,
      
      .is_dirty = 0,
      .width = 2,
    });
    
    return;
  }
  
  sp_clear(span, 2 * index + 1);
  sp_node_t left = sp_read(span, 2 * index + 1);
  
  sp_clear(span, 2 * index + 2);
  sp_node_t right = sp_read(span, 2 * index + 2);
  
  sp_write(span, index, (sp_node_t) {
    .free = left.free + right.free,
    .span = SP_MAX(SP_MAX(left.span, right.span), left.right + right.left),
    .left = ((left.free == left.width) ? left.free + right.left : left.left),
    .right = ((right.free == right.width) ? right.free + left.right : right.right),
    
    .is_dirty = 0,
    .width = left.width + right.width,
  });
}

void sp_apply(span_t *span, size_t index) {
  // Apply lazy propagation on each index, non-recursively (TODO)
}
