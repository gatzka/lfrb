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

#include <stdint.h>

#include "fff.h"
#include "lfrb.h"
#include "unity.h"

DEFINE_FFF_GLOBALS

void setUp(void)
{
	FFF_RESET_HISTORY();
}

void tearDown(void)
{
}

static void test_init(void)
{
	struct lfrb lfrb;
	uint8_t buffer[8];
	enum lfrb_error err = lfrb_init(&lfrb, sizeof(buffer), buffer);
	TEST_ASSERT_EQUAL_MESSAGE(LFRB_SUCCESS, err, "Initialization of rightly sized buffer failed!");
	TEST_ASSERT_EQUAL_MESSAGE(0, lfrb_dequeue_available(&lfrb), "Data available in freshly initialized ringbuffer!");
	TEST_ASSERT_EQUAL_MESSAGE(sizeof(buffer), lfrb_enqueue_available(&lfrb), "Not enough space available to enqueue data afer initializing ringbuffer!");

	err = lfrb_init(&lfrb, sizeof(buffer) - 1, buffer);
	TEST_ASSERT_EQUAL_MESSAGE(LFRB_NOT_POWER_OF_2, err, "Initialization of non-power-of-2 buffer did not fail!");

	err = lfrb_init(&lfrb, sizeof(buffer), NULL);
	TEST_ASSERT_EQUAL_MESSAGE(LFRB_INVALID_ARGUMENT, err, "Initialization if no buffer memopry given did not fail!");
}

static void test_enqueue(void)
{
	struct lfrb lfrb;
	uint8_t buffer[8];
	enum lfrb_error err = lfrb_init(&lfrb, sizeof(buffer), buffer);
	TEST_ASSERT_EQUAL_MESSAGE(LFRB_SUCCESS, err, "Initialization of rightly sized buffer failed!");
	TEST_ASSERT_TRUE_MESSAGE(lfrb_is_empty(&lfrb), "ringbuffer is not empty after initialization");

	for (unsigned int i = 0; i < sizeof(buffer); i++) {
		size_t num= lfrb_enqueue(&lfrb, 'a');
		TEST_ASSERT_EQUAL_MESSAGE(1, num, "Enqueueing upto size of buffer failed!");
	}

	size_t num = lfrb_enqueue(&lfrb, 'a');
	TEST_ASSERT_EQUAL_MESSAGE(0, num, "Enqueueing more than size of buffer did not failed!");

	TEST_ASSERT_TRUE_MESSAGE(lfrb_is_full(&lfrb), "Buffer is not full after writing enough bytes!");
}

static void test_enqueue_dequeue(void)
{
	uint8_t val;
	struct lfrb lfrb;
	uint8_t buffer[8];
	enum lfrb_error err = lfrb_init(&lfrb, sizeof(buffer), buffer);
	TEST_ASSERT_EQUAL_MESSAGE(LFRB_SUCCESS, err, "Initialization of rightly sized buffer failed!");
	TEST_ASSERT_TRUE_MESSAGE(lfrb_is_empty(&lfrb), "ringbuffer is not empty after initialization");
	size_t num = lfrb_dequeue(&lfrb, &val);
	TEST_ASSERT_EQUAL_MESSAGE(0, num, "Dequeueing from emtpy buffer did not fail!");

	for (unsigned int i = 0; i < 3 * sizeof(buffer); i++) {
		num = lfrb_enqueue(&lfrb, (uint8_t)i);
		TEST_ASSERT_EQUAL_MESSAGE(1, num, "Enqueueing into buffer failed!");
		num = lfrb_dequeue(&lfrb, &val);
		TEST_ASSERT_EQUAL_MESSAGE(1, num, "Dequeueing from buffer failed!");

		TEST_ASSERT_EQUAL_MESSAGE((uint8_t)i, val, "Dequeued value not correct!");
	}
}

static void test_dequeuer_clear(void)
{
	struct lfrb lfrb;
	uint8_t buffer[8];
	enum lfrb_error err = lfrb_init(&lfrb, sizeof(buffer), buffer);
	TEST_ASSERT_EQUAL_MESSAGE(LFRB_SUCCESS, err, "Initialization of rightly sized buffer failed!");

	for (unsigned int i = 0; i < sizeof(buffer); i++) {
		size_t num = lfrb_enqueue(&lfrb, 'a');
		TEST_ASSERT_EQUAL_MESSAGE(1, num, "Enqueueing upto size of buffer failed!");
	}

	TEST_ASSERT_EQUAL_MESSAGE(sizeof(buffer), lfrb_dequeue_available(&lfrb), "Not enough data available in full ringbuffer!");
	TEST_ASSERT_EQUAL_MESSAGE(0, lfrb_enqueue_available(&lfrb), "Data available for writing in full ringbuffer!");
	TEST_ASSERT_TRUE_MESSAGE(lfrb_is_full(&lfrb), "Buffer is not full after writing enough bytes!");

	lfrb_dequeuer_clear(&lfrb);
	TEST_ASSERT_TRUE_MESSAGE(lfrb_is_empty(&lfrb), "Buffer is not empty after clearing!");
	TEST_ASSERT_EQUAL_MESSAGE(0, lfrb_dequeue_available(&lfrb), "Data available in cleared ringbuffer!");
	TEST_ASSERT_EQUAL_MESSAGE(sizeof(buffer), lfrb_enqueue_available(&lfrb), "Not enough space available for writing in cleared ringbuffer!");
}

static void test_enqueuer_clear(void)
{
	struct lfrb lfrb;
	uint8_t buffer[8];
	enum lfrb_error err = lfrb_init(&lfrb, sizeof(buffer), buffer);
	TEST_ASSERT_EQUAL_MESSAGE(LFRB_SUCCESS, err, "Initialization of rightly sized buffer failed!");

	for (unsigned int i = 0; i < sizeof(buffer); i++) {
		size_t num = lfrb_enqueue(&lfrb, 'a');
		TEST_ASSERT_EQUAL_MESSAGE(1, num, "Enqueueing upto size of buffer failed!");
	}

	TEST_ASSERT_EQUAL_MESSAGE(sizeof(buffer), lfrb_dequeue_available(&lfrb), "Not enough data available in full ringbuffer!");
	TEST_ASSERT_EQUAL_MESSAGE(0, lfrb_enqueue_available(&lfrb), "Data available for writing in full ringbuffer!");
	TEST_ASSERT_TRUE_MESSAGE(lfrb_is_full(&lfrb), "Buffer is not full after writing enough bytes!");

	lfrb_enqueuer_clear(&lfrb);
	TEST_ASSERT_TRUE_MESSAGE(lfrb_is_empty(&lfrb), "Buffer is not empty after clearing!");
	TEST_ASSERT_EQUAL_MESSAGE(0, lfrb_dequeue_available(&lfrb), "Data available in cleared ringbuffer!");
	TEST_ASSERT_EQUAL_MESSAGE(sizeof(buffer), lfrb_enqueue_available(&lfrb), "Not enough space available for writing in cleared ringbuffer!");
}

static void test_enqueue_block(void)
{
	struct lfrb lfrb;
	uint8_t buffer[8];
	enum lfrb_error err = lfrb_init(&lfrb, sizeof(buffer), buffer);
	TEST_ASSERT_EQUAL_MESSAGE(LFRB_SUCCESS, err, "Initialization of rightly sized buffer failed!");

	uint8_t block[] = {1, 2, 3};
	size_t written = lfrb_enqueue_buffer(&lfrb, block, sizeof(block));
	TEST_ASSERT_EQUAL_MESSAGE(sizeof(block), written, "Not enough data written into empty ringbuffer!");

	for (unsigned int i = 0; i < sizeof(block); i++) {
		uint8_t val;
		size_t num = lfrb_dequeue(&lfrb, &val);
		TEST_ASSERT_EQUAL_MESSAGE(num, 1, "Dequeueing failed!");
		TEST_ASSERT_EQUAL_MESSAGE(block[i], val, "dequeuing values from block enqueue do not match!");
	}
}

static void test_enqueue_big_block(void)
{
	struct lfrb lfrb;
	uint8_t buffer[4];
	enum lfrb_error err = lfrb_init(&lfrb, sizeof(buffer), buffer);
	TEST_ASSERT_EQUAL_MESSAGE(LFRB_SUCCESS, err, "Initialization of rightly sized buffer failed!");

	uint8_t block[] = {1, 2, 3, 4, 5};
	size_t written = lfrb_enqueue_buffer(&lfrb, block, sizeof(block));
	TEST_ASSERT_EQUAL_MESSAGE(lfrb_size(&lfrb), written, "Not enough or too much data written into empty ringbuffer!");

	for (unsigned int i = 0; i < written; i++) {
		uint8_t val;
		size_t num = lfrb_dequeue(&lfrb, &val);
		TEST_ASSERT_EQUAL_MESSAGE(1, num, "Dequeueing failed!");
		TEST_ASSERT_EQUAL_MESSAGE(block[i], val, "dequeuing values from block enqueue do not match!");
	}
}

static void test_enqueue_big_block_with_wrap_around(void)
{
	uint8_t val;

	struct lfrb lfrb;
	uint8_t buffer[4];
	enum lfrb_error err = lfrb_init(&lfrb, sizeof(buffer), buffer);
	TEST_ASSERT_EQUAL_MESSAGE(LFRB_SUCCESS, err, "Initialization of rightly sized buffer failed!");

	for (unsigned int i = 0; i < 20; i++) {
		lfrb_enqueue(&lfrb, 'a');
		lfrb_dequeue(&lfrb, &val);

		uint8_t block[] = {1, 2, 3, 4, 5};
		size_t written = lfrb_enqueue_buffer(&lfrb, block, sizeof(block));
		TEST_ASSERT_EQUAL_MESSAGE(lfrb_size(&lfrb), written, "Not enough or too much data written into empty ringbuffer!");

		for (unsigned int j = 0; j < written; j++) {
			size_t num = lfrb_dequeue(&lfrb, &val);
			TEST_ASSERT_EQUAL_MESSAGE(1, num, "Dequeueing failed!");
			TEST_ASSERT_EQUAL_MESSAGE(block[j], val, "dequeuing values from block enqueue do not match!");
		}
	}
}

int main(void)
{
	UNITY_BEGIN();
	RUN_TEST(test_init);
	RUN_TEST(test_enqueue);
	RUN_TEST(test_enqueue_dequeue);
	RUN_TEST(test_dequeuer_clear);
	RUN_TEST(test_enqueuer_clear);
	RUN_TEST(test_enqueue_block);
	RUN_TEST(test_enqueue_big_block);
	RUN_TEST(test_enqueue_big_block_with_wrap_around);
	return UNITY_END();
}
