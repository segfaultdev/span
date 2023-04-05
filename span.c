#include <stddef.h>
#include <stdint.h>
#include <span.h>

#define SP_MIN(x, y) ({size_t _x = (x), _y = (y); (_x < _y ? _x : _y);})
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
}

size_t sp_get_width(span_t *span, size_t index) {
  return (2ull << (__builtin_clzll(index + 1) - __builtin_clzll(span->size - 1)));
}

sp_node_t sp_read(span_t *span, size_t index) {
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
  
  return node;
}

void sp_write(span_t *span, size_t index, sp_node_t node) {
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

int sp_check(span_t *span, size_t index, size_t offset) {
  sp_node_t node = sp_read(span, index);
  
  if (offset < node.left + 1) {
    return (offset < node.left);
  } else if (offset >= node.width - node.right - 1) {
    return (offset >= node.width - node.right);
  }
  
  if (offset < node.width / 2) {
    return sp_check(span, 2 * index + 1, offset);
  } else {
    return sp_check(span, 2 * index + 2, offset - node.width / 2);
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
  
  size_t node_left = ((left.free == left.width) ? left.free + right.left : left.left);
  size_t node_right = ((right.free == right.width) ? right.free + left.right : right.right);
  
  sp_write(span, index, (sp_node_t) {
    .free = left.free + right.free,
    .span = SP_MAX(SP_MAX(SP_MAX(left.span, right.span), left.right + right.left), SP_MAX(node_left, node_right)),
    .left = node_left,
    .right = node_right,
    
    .is_dirty = 0,
    .width = left.width + right.width,
  });
}

void sp_apply(span_t *span, size_t index) {
  sp_node_t node = sp_read(span, index);
  if (!node.is_dirty) return;
  
  sp_node_t left = sp_read(span, 2 * index + 1);
  sp_node_t right = sp_read(span, 2 * index + 2);
  
  if (node.lazy_start < node.width / 2) {
    if (left.is_dirty) {
      sp_apply(span, 2 * index + 1);
      left = sp_read(span, 2 * index + 1);
    }
    
    left.lazy_start = node.lazy_start;
    left.lazy_end = SP_MIN(node.lazy_end, left.width);
    
    sp_write(span, 2 * index + 1, left);
  }
  
  if (node.lazy_end >= node.width / 2) {
    if (right.is_dirty) {
      sp_apply(span, 2 * index + 2);
      right = sp_read(span, 2 * index + 2);
    }
    
    right.lazy_start = SP_MAX(node.lazy_start, right.width) - right.width;
    right.lazy_end = node.lazy_end;
    
    sp_write(span, 2 * index + 2, right);
  }
  
  int clear = 1;
  
  if (node.lazy_start < node.left) {
    clear = 0;
    
    // Must end inside left edge, as lazy spans are guaranteed to have the same state in all bits prior to updating
    
    node.free -= (node.lazy_end - node.lazy_start);
    node.left = node.lazy_start;
    
    if (node.right == node.width) {
      node.right -= node.lazy_end;
      node.span = SP_MAX(node.left, node.right);
    }
    
    if (left.span == left.left && left.free < left.width) {
      sp_apply(span, 2 * index + 1);
      left = sp_read(span, 2 * index + 1);
    } else {
      left.free -= SP_MIN((node.lazy_end - node.lazy_start), left.width);
      left.left = SP_MIN(node.lazy_start, left.width);
      
      if (left.right == left.width) {
        left.right -= SP_MIN(left.lazy_end, left.width);
        left.span = SP_MAX(left.left, left.right);
      }
    }
    
    if (right.span == right.left && right.free < right.width) {
      sp_apply(span, 2 * index + 2);
      right = sp_read(span, 2 * index + 2);
    } else {
      if (node.lazy_end > left.width) {
        right.free -= (node.lazy_end - left.width);
        right.left = 0;
        
        if (right.right == right.width) {
          right.right -= (node.lazy_end - left.width);
          right.span = SP_MAX(right.left, right.right);
        }
      }
    }
  }
  
  // Fill:
  // - Left (DONE)
  // - Center
  // - Right
  // - Other (have to apply no matter what)
  
  // node.span = SP_MAX(SP_MAX(SP_MAX(left.span, right.span), left.right + right.left), SP_MAX(node.left, node.right));
  node.is_dirty = 0;
  
  sp_write(span, index, node);
}

void sp_mark(span_t *span, size_t start, size_t end) {
  sp_apply(span, 0);
  sp_node_t node = sp_read(span, 0);
  
  node.lazy_start = start;
  node.lazy_end = end;
  
  node.is_dirty = 1;
  sp_write(span, 0, node);
}
