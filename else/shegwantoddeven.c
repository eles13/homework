#include <mpi.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define MASTER 0

static int cmp(const void *_a, const void *_b)
{
	return *((int *)_a) - *((int *)_b);
}

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

void out(int* m, int size, int rank)
{
	printf("%d\n", rank);
	for(int i = 0; i < size; i++)
		printf("%d ", m[i]);
	printf("\n");
}

static void merge(int* m1, int* m2, const int size, int* res)
{
  int s1 = 0;
  int s2 = 0;
  for(int i = 0; i < size * 2; i++)
  {
    if(((s2 == size) || (m1[s1] <= m2[s2])) && (s1 != size))
    {
      res[i] = m1[s1++];
    }
    else if(s2 != size)
    {
      res[i] = m2[s2++];
    }
  }
}

static void odd_sort(const int rank, const int size, int* nums, const int n, int* recbuf, int* result)
{
  if ((rank % 2 == 0) && (rank != 0))
  {
    MPI_Send(nums, n, MPI_INTEGER, rank - 1, 0, MPI_COMM_WORLD);
  }
  if ((rank % 2 == 1) && (rank != size - 1))
  {
    MPI_Recv(recbuf, n, MPI_INTEGER, rank + 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    merge(nums, recbuf, n, result);
    MPI_Send(&result[n], n, MPI_INTEGER, rank + 1, 1, MPI_COMM_WORLD);
    for(int i = 0; i < size; i++)
    {
      nums[i] = result[i];
    }
  }
  if ((rank % 2 == 0) && (rank != 0))
  {
    MPI_Recv(nums, n, MPI_INTEGER, rank - 1, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  }
}

static void even_sort(const int rank, const int size, int* nums, const int n, int* recbuf, int* result)
{
  if ((rank % 2 == 1))
  {
    MPI_Send(nums, n, MPI_INTEGER, rank - 1, 0, MPI_COMM_WORLD);
  }
  if ((rank % 2 == 0) && (rank != size - 1))
  {
    MPI_Recv(recbuf, n, MPI_INTEGER, rank + 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    merge(nums, recbuf, n, result);
    MPI_Send(&result[n], n, MPI_INTEGER, rank + 1, 1, MPI_COMM_WORLD);
    for(int i = 0; i < n; i++)
    {
      nums[i] = result[i];
    }
  }
  if ((rank % 2 == 1))
  {
    MPI_Recv(nums, n, MPI_INTEGER, rank - 1, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  }
}

static int check(const int rank, const int size, const int *nums, const int n)
{
  int rc;
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
		rc = is_sorted(buf, 2 * size);
		free(buf);
    return rc == 0;
	}
	return 0;
}

static void sort(const int rank, const int size, int *nums, const int n)
{
  int * recbuf = calloc(n, sizeof(int));
  int * result = calloc(n*2, sizeof(int));
  qsort(nums, n, sizeof(int), cmp);
  int rc = 1;
  while (rc)
  {
    odd_sort(rank, size, nums, n, recbuf, result);
    MPI_Barrier(MPI_COMM_WORLD);
    even_sort(rank, size, nums, n, recbuf, result);
    MPI_Barrier(MPI_COMM_WORLD);
    rc = check(rank, size, nums, n);
    MPI_Bcast(&rc, 1, MPI_INTEGER, MASTER, MPI_COMM_WORLD);
  }
  free(recbuf);
  free(result);
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
