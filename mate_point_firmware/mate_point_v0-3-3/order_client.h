#pragma once

#include <stddef.h>

bool order_create(char *order_id, size_t order_id_len,
                  char *external_ref, size_t external_ref_len);
bool order_cancel(const char *order_id);
