#pragma once

#include <stddef.h>

bool order_create(char *order_id, size_t order_id_len,
                  char *external_ref, size_t external_ref_len,
                  char *product_desc, size_t product_desc_len,
                  char *price_display, size_t price_display_len);
bool order_cancel(const char *order_id);
