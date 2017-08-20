/*
   Copyright 2017, Mike Krinkin <krinkin.m.u@gmail.com>

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <block/block.h>
#include <myfs.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>

#include <assert.h>
#include <stdio.h>
#include <errno.h>


static void generate_trans(struct myfs_trans *trans)
{
	char data[256];

	for (size_t i = 0; i != sizeof(data)/sizeof(data[0]); ++i)
		data[i] = rand();
	myfs_trans_append(trans, 0, data, sizeof(data));
}

static int run_test(struct myfs *myfs)
{
	const size_t COUNT = 1000000;
	for (size_t i = 0; i != COUNT; ++i) {
		struct myfs_trans trans;

		myfs_trans_setup(&trans);
		generate_trans(&trans);
		myfs_trans_submit(myfs, &trans);
		assert(!myfs_trans_wait(&trans));
		myfs_trans_release(&trans);
	}
	return 0;
}

static void *worker(void *arg)
{
	struct myfs *myfs = arg;
	myfs_trans_worker(myfs);
	return NULL;
}

static int ignore(struct myfs *myfs, uint32_t type, const void *data,
			size_t size)
{
	(void) myfs;
	(void) type;
	(void) data;
	(void) size;
	return 0;
}

static struct myfs_trans_apply trans_apply = {
	.apply = &ignore
};

static void start_trans_worker(struct myfs *myfs)
{
	assert((myfs->log_data = malloc(MYFS_MAX_WAL_SIZE)));
	assert(!pthread_mutex_init(&myfs->trans_mtx, NULL));
	assert(!pthread_cond_init(&myfs->trans_cv, NULL));
	atomic_store_explicit(&myfs->trans, NULL, memory_order_relaxed);
	myfs->done = 0;
	myfs->trans_apply = &trans_apply;
	assert(!pthread_create(&myfs->trans_worker, NULL, &worker, myfs));
}

static void stop_trans_worker(struct myfs *myfs)
{
	assert(!pthread_mutex_lock(&myfs->trans_mtx));
	myfs->done = 1;
	assert(!pthread_cond_signal(&myfs->trans_cv));
	assert(!pthread_mutex_unlock(&myfs->trans_mtx));
	assert(!pthread_join(myfs->trans_worker, NULL));
	assert(!pthread_mutex_destroy(&myfs->trans_mtx));
	assert(!pthread_cond_destroy(&myfs->trans_cv));
	free(myfs->log_data);
}

static const char TEST_NAME[] = "test.bin";

int main()
{
	const int fd = open(TEST_NAME, O_RDWR | O_CREAT | O_TRUNC,
				S_IRUSR | S_IWUSR);

	if (fd < 0) {
		perror("failed to create test file");
		return 1;
	}

	struct sync_bdev bdev;
	struct myfs myfs;
	int ret = 0;

	sync_bdev_setup(&bdev, fd);
	myfs.bdev = &bdev.bdev;
	myfs.page_size = 4096;
	myfs.next_offs = 0;

	memset(&myfs.log, 0, sizeof(myfs.log));

	start_trans_worker(&myfs);
	ret = run_test(&myfs);
	stop_trans_worker(&myfs);

	if (ret)
		fprintf(stderr, "tests failed\n");
	else
		unlink(TEST_NAME);
	close(fd);

	return ret ? 1 : 0;
}
