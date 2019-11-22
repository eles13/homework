#include <mpi.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define MASTER 0

static int seed(const int rank, const int size)
{
	int *seeds = NULL;
	if (rank == MASTER) {
		srand(time(NULL));
		seeds = malloc(size * sizeof(int));
		if (seeds == NULL) {
			return -1;
		}
		for (int i = 0; i < size; i++) {
			seeds[i] = rand();
		}
	}
	int seed;
	MPI_Scatter(seeds, 1, MPI_INTEGER, &seed, 1, MPI_INTEGER, MASTER, MPI_COMM_WORLD);
	if (rank == MASTER) {
		free(seeds);
	}
	srand(seed);
	return 0;
}

static int *gen_numbers(const int n)
{
	int *nums = malloc(n * sizeof(int));
	if (nums == NULL) {
		return NULL;
	}
	for (int i = 0; i < n; i++) {
		nums[i] = rand() % 100500;
	}
	return nums;
}

static int is_sorted(const int *nums, const int n)
{
	for (int i = 0; i < n - 1; i++) {
		if (nums[i] > nums[i + 1]) {
			return 0;
		}
	}
	return 1;
}

static void min_max(const int *nums, const int n, int *min, int *max)
{
	*min = nums[0];
	*max = nums[0];
	for (int i = 1; i < n; i++) {
		if (nums[i] > *max) {
			*max = nums[i];
		}
		if (nums[i] < *min) {
			*min = nums[i];
		}
	}
}

static void swap(int* nums, const int from, const int to)
{
	int temp = nums[from];
	nums[from] = nums[to];
	nums[to] = temp;
}

static int bubble(const int rank, const int size, int* nums)
{
	int sorted = 0;
	int round = 0;
	while(!sorted)
	{
		sorted = 1;
		for(int i = 1; i < size - 1; i += 2)
		{
			if(nums[i] > nums[i + 1])
			{
				round = 1;
				swap(nums, i, i+1);
				sorted = 0;
			}
		}

		for(int i = 0; i < size - 1; i += 2)
		{
			if (nums[i] > nums[i + 1])
			{
				round = 1;
				swap(nums, i, i+1);
				sorted = 0;
			}
		}
	}
	if (round)
		return 1;
	return 0;
}

void out(int* m, int size, int rank)
{
	printf("%d\n", rank);
	for(int i = 0; i < size; i++)
		printf("%d ", m[i]);
	printf("\n");
}

static void sort(const int rank, const int size, int *nums, const int n)
{
	int* results = calloc(size,sizeof(int));
	assert(results);
	if (rank == size - 1)
	{
		int* rest = calloc(n/2, sizeof(int));
		assert(rest);
	}
	int* buf = calloc(n/2, sizeof(int));
	assert(buf);
	int round = 0;
	int response = 1;
	int type = 0;
	int* result = calloc(1,sizeof(int));
	while(1)
	{
		if(type == 0)
		{
			result[0] = bubble(rank, n ,nums);
			MPI_Barrier(MPI_COMM_WORLD);
			MPI_Gather(result, 1, MPI_INTEGER, results, 1, MPI_INTEGER, MASTER, MPI_COMM_WORLD);
			if (rank == 0)
			{
				response = 0;
				for(int i = 0; i < size; i++)
					response += results[i];
				for(int i = 0; i < size; i++)
					results[i] = response;
			}
			MPI_Scatter(results, 1, MPI_INTEGER, &response, 1, MPI_INTEGER, MASTER, MPI_COMM_WORLD);
			if (response == 0)
				break;
		}
		else
		{
			if (rank == size - 1)
			{
					MPI_Send(&nums[n/2], n/2, MPI_INTEGER, 0, 0, MPI_COMM_WORLD);
			}
			else
			{
				MPI_Send(&nums[n/2], n/2, MPI_INTEGER, rank + 1, rank + 1, MPI_COMM_WORLD);
			}
			MPI_Barrier(MPI_COMM_WORLD);
			if (rank != 0)
			{
				MPI_Recv(buf,n/2, MPI_INTEGER, rank - 1, rank, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			}
			else if (rank == 0)
			{
				MPI_Recv(buf, n/2, MPI_INTEGER, size - 1, rank, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			}
			MPI_Barrier(MPI_COMM_WORLD);
			for(int i = n/2; i < n; i++)
			{
				nums[i] = buf[i - n/2];
			}
			result[0] = bubble(rank, n, nums);
			MPI_Barrier(MPI_COMM_WORLD);
			if (rank != 0)
			{
				MPI_Send(nums, n/2, MPI_INTEGER, rank - 1, rank - 1, MPI_COMM_WORLD);
				for(int i = 0; i < n/2; i++)
					nums[i] = nums[i + n/2];
			}
			else
			{
				MPI_Send(&nums[n/2], n/2, MPI_INTEGER, size - 1, size - 1, MPI_COMM_WORLD);

			}
			MPI_Barrier(MPI_COMM_WORLD);
			if(rank != size - 1)
			{
				MPI_Recv(buf, n/2, MPI_INTEGER, rank + 1, rank, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			}
			else
			{
				MPI_Recv(buf, n/2, MPI_INTEGER, 0, rank, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			}
			for(int i = 0; i < n/2; i++)
				nums[n/2 + i] = buf[i];
		}
		round++;
		type = round % 2;
		MPI_Barrier(MPI_COMM_WORLD);
	}
	free(results);
	free(buf);
	free(result);
}

static int check(const int rank, const int size, const int *nums, const int n)
{
	if (!is_sorted(nums, n)) {
		return -1;
	}
	int pair[2];
	min_max(nums, n, pair, &pair[1]);
	int *buf = NULL;
	if (rank == MASTER) {
		buf = malloc(2 * size * sizeof(int));
		if (buf == NULL) {
			return -2;
		}
	}
	MPI_Gather(pair, 2, MPI_INTEGER, buf, 2, MPI_INTEGER, MASTER, MPI_COMM_WORLD);
	if (rank == MASTER) {
		int rc = is_sorted(buf, 2 * size);
		free(buf);
		return rc == 0;
	}
	return 0;
}

int main(int argc, char **argv)
{
	MPI_Init(&argc, &argv);

	if (argc != 2) {
		return -1;
	}
	int n = atoi(argv[1]);
	assert(n > 0);
	int rank, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	int rc = seed(rank, size);
	assert(rc == 0);

	int *nums = gen_numbers(n);
	assert(nums);
	int* buf = calloc(8, sizeof(int));
  assert(buf);
	sort(rank, size, nums, n);
	rc = check(rank, size, nums, n);
	assert(rc == 0);
	free(buf);
	free(nums);
	return MPI_Finalize();
}
