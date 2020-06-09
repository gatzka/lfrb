/*
 * SPDX-License-Identifier: MIT
 *
 * The MIT License (MIT)
 *
 * Copyright (c) <2020> <Stephan Gatzka>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "lfrb.h"

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

//static memory_order index_acquire_barrier = memory_order_acquire;
//static memory_order index_release_barrier = memory_order_release;

static memory_order index_acquire_barrier = memory_order_relaxed;
static memory_order index_release_barrier = memory_order_relaxed;

enum lfrb_error lfrb_init(struct lfrb *lfrb, size_t size, uint8_t *mem)
{
	lfrb->mask = size - 1;
	if ((lfrb->mask & size) != 0) {
		return LFRB_NOT_POWER_OF_2;
	}

	if (mem == NULL) {
		return LFRB_INVALID_ARGUMENT;
	}

	lfrb->mem = mem;
	atomic_init(&lfrb->read_index, 0);
	atomic_init(&lfrb->write_index, 0);

	return LFRB_SUCCESS;
}

size_t lfrb_enqueue(struct lfrb *lfrb, uint8_t value)
{
	size_t tmp_write_index = atomic_load_explicit(&lfrb->write_index, memory_order_relaxed);
	if ((tmp_write_index - atomic_load_explicit(&lfrb->read_index, index_acquire_barrier)) == lfrb_size(lfrb)) {
		return 0;
	} else {
		lfrb->mem[tmp_write_index++ & lfrb->mask] = value;
		atomic_signal_fence(memory_order_release);
		atomic_store_explicit(&lfrb->write_index, tmp_write_index, index_release_barrier);
	}

	return 1;
}

size_t lfrb_enqueue_buffer(struct lfrb *lfrb, const uint8_t *buff, size_t count)
{
	size_t tmp_write_index = atomic_load_explicit(&lfrb->write_index, memory_order_relaxed);
	size_t to_write = count;

	size_t available = lfrb_enqueue_available(lfrb);

	if (available < count) {
		to_write = available;
	}

	size_t first_chunk = MIN(lfrb_size(lfrb) - (tmp_write_index & lfrb->mask), to_write);
	memcpy(&lfrb->mem[tmp_write_index  & lfrb->mask], buff, first_chunk);
	tmp_write_index += first_chunk;

	size_t second_chunk = to_write - first_chunk;
	memcpy(&lfrb->mem[tmp_write_index  & lfrb->mask], &buff[first_chunk], second_chunk);
	tmp_write_index += second_chunk;

	atomic_signal_fence(memory_order_release);
	atomic_store_explicit(&lfrb->write_index, tmp_write_index, index_release_barrier);

	return to_write;
}

size_t lfrb_dequeue(struct lfrb *lfrb, uint8_t *value)
{
	size_t tmp_read_index = atomic_load_explicit(&lfrb->read_index, memory_order_relaxed);

	if (tmp_read_index == atomic_load_explicit(&lfrb->write_index, memory_order_relaxed)) {
		return 0;
	} else {
		*value = lfrb->mem[tmp_read_index & lfrb->mask];
		atomic_store_explicit(&lfrb->read_index, ++tmp_read_index, index_release_barrier);
	}

	return 1;
}

size_t lfrb_dequeue_available(const struct lfrb *lfrb)
{
	return atomic_load_explicit(&lfrb->write_index, index_acquire_barrier) - atomic_load_explicit(&lfrb->read_index, memory_order_relaxed);
}

size_t lfrb_enqueue_available(const struct lfrb *lfrb)
{
	return lfrb_size(lfrb) - (atomic_load_explicit(&lfrb->write_index, memory_order_relaxed) - atomic_load_explicit(&lfrb->read_index, index_acquire_barrier));
}

size_t lfrb_size(const struct lfrb *lfrb)
{
	return (lfrb->mask + 1);
}

bool lfrb_is_full(const struct lfrb *lfrb)
{
	return lfrb_enqueue_available(lfrb) == 0;
}

bool lfrb_is_empty(const struct lfrb *lfrb)
{
	return lfrb_dequeue_available(lfrb) == 0;
}

void lfrb_enqueuer_clear(struct lfrb *lfrb)
{
	atomic_store_explicit(&lfrb->write_index, atomic_load_explicit(&lfrb->read_index, memory_order_relaxed), memory_order_relaxed);
}

void lfrb_dequeuer_clear(struct lfrb *lfrb)
{
	atomic_store_explicit(&lfrb->read_index, atomic_load_explicit(&lfrb->write_index, memory_order_relaxed), memory_order_relaxed);
}
