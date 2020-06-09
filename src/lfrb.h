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

#ifndef LFRB_H
#define LFRB_H

#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "lfrb_export.h"

#ifdef __cplusplus
extern "C" {
#endif

enum lfrb_error {
	LFRB_SUCCESS = 0,           /*!< No error occured. */
	LFRB_NOT_POWER_OF_2 = -1,   /*!< The ringbuffer size is not power of 2. */
	LFRB_INVALID_ARGUMENT = -2, /*!< An argument is not correct. */
};

struct lfrb {
	size_t mask;
	uint8_t *mem;
	atomic_size_t read_index;
	atomic_size_t write_index;
};
/**
 * @brief Initializes the lock free ringbuffer.
 * 
 * @param[in] lfrb The ringbuffer to be initialized.
 * @param size The size of the rinfbuffer in bytes. Must be a power of 2.
 * @param mem  The memory to operate on
 * @return ::LFRB_SUCCESS on success.
 */
LFRB_EXPORT enum lfrb_error lfrb_init(struct lfrb *lfrb, size_t size, uint8_t *mem);

/**
 * @brief Enqueues a byte into the ringbuffer.
 * 
 * @param[in] lfrb The ringbuffer where the byte should be enqueued to.
 * @param value The value to be enqueued
 * @return 1 on success
 * @return 0 if not enough space available.
 */
size_t lfrb_enqueue(struct lfrb *lfrb, uint8_t value);

/**
 * @brief Enqueues multiple bytes into the ringbuffer.
 * 
 * @param[in] lfrb The ringbuffer where the bytes should be enqueued to.
 * @param buff Pointer to buffer with data to be inserted from.
 * @param count Number of elements to write from the given buffer
 * @return The number of bytes actually enqueued. Could be zero!
 */
LFRB_EXPORT size_t lfrb_enqueue_buffer(struct lfrb *lfrb, uint8_t *buff, size_t count);

/**
 * @brief Dequeues a byte from the ringbuffer.
 * 
 * @param[in] lfrb The ringbuffer where the byte should be dequeued from.
 * @param[out] value Pointer where the value should be stored to.
 * @return 1 on success
 * @return 0 if no data is available.
 */
size_t lfrb_dequeue(struct lfrb *lfrb, uint8_t *value);

/**
 * @brief Check how many elements can be read from the buffer.
 * 
 * @param[in] lfrb The buffer that should be asked.
 * @return The number of elements that can be read.
 */
LFRB_EXPORT size_t lfrb_dequeue_available(const struct lfrb *lfrb);

/**
 * @brief Check how many elements can be written to the buffer.
 * 
 * @param[in] lfrb The buffer that should be asked.
 * @return The number of elements that can be written.
 */
LFRB_EXPORT size_t lfrb_enqueue_available(const struct lfrb *lfrb);

/**
 * @brief Check if the buffer is full.
 * 
 * @param[in] lfrb The buffer that should be asked.
 * @return true if the buffer is full.
 * @return false if the buffer is not full.
 */
LFRB_EXPORT bool lfrb_is_full(const struct lfrb *lfrb);

/**
 * @brief Check if the buffer is empty.
 * 
 * @param[in] lfrb The buffer that should be asked.
 * @return true if the buffer is empty.
 * @return false if the buffer is not empty.
 */
LFRB_EXPORT bool lfrb_is_empty(const struct lfrb *lfrb);

/**
 * @brief Clear the buffer from the enqueuer side
 * 
 * @param[in] lfrb The buffer to be cleared.
 */
LFRB_EXPORT void lfrb_enqueuer_clear(struct lfrb *lfrb);

/**
 * @brief Clear the buffer from the dequeuer side
 * 
 * @param[in] lfrb The buffer to be cleared.
 */
LFRB_EXPORT void lfrb_dequeuer_clear(struct lfrb *lfrb);

/**
 * @brief Gives the size of the ringbuffer.
 * 
 * @param[in] lfrb The ringbuffer to be asked.
 * @return The size in bytes of the ringbuffer.
 */
LFRB_EXPORT size_t lfrb_size(const struct lfrb *lfrb);

#ifdef __cplusplus
}
#endif

#endif
