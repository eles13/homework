#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#define MASTER 0

static void bcastblocks(int rank, int size, int32_t** ma, int32_t** mb, MPI_Datatype block, int32_t n, int32_t** a, int32_t** b)
{
	a = calloc(n * n/size, sizeof(int32_t));
	b = calloc(n * n/size, sizeof(int32_t));
	assert(a);
	assert(b);
	if (rank == MASTER)
	{
		int32_t start = 0;
		for(int i = 1; i < size; i++)
		{
			if(i % (int)sqrt(size) == 0)
			{
				start += n * n/sqrt(size);
			}
			else
			{
				start+=n/sqrt(size);
			}
			MPI_Send(ma[start], 1, block, i, 0, MPI_COMM_WORLD);
			MPI_Send(ma[start], 1, block, i, 1, MPI_COMM_WORLD);
		}
		MPI_Sendrecv(ma, 1, block, MASTER, 0, a, n * n /size, MPI_INTEGER, MASTER, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		MPI_Sendrecv(mb, 1, block, MASTER, 1, b, n * n /size, MPI_INTEGER, MASTER, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	}
	else
	{
		MPI_Recv(a,n*n/size, MPI_INT, MASTER, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		MPI_Recv(b,n*n/size, MPI_INT, MASTER, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	}
}

static void performCannon(int rank, int size, int32_t n, int32_t** a, int32_t** b)
{
	MPI_Comm cart;
	int dims[2];
	dims[0] = dims[1] = (int)sqrt(size);
	int periods[2];
	periods[0] = periods[1] = 1;
	MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periods, 1, &cart);
	int myrank;
	MPI_Comm_rank(cart, &myrank);
	int coords[2];
	MPI_Cart_coords(cart, myrank, 2, coords);
}

int main(int argc, char **argv){
	MPI_Init(&argc, &argv);
	int rank, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	int32_t n;
	int32_t** ma;
	int32_t** mb;
	if (rank == MASTER)
	{
		if (argc != 4)
			return -1;
		FILE* fina = fopen(argv[1], "rb+");
		FILE* finb = fopen(argv[2], "rb+");
		FILE* fout = fopen(argv[3], "a");
		if(!(fina&&finb&&fout))
			return -1;
		int32_t sizea, sizeb;
		int check;
		check = fread(&sizea, sizeof(int32_t), 1, fina);
		if (!check)
			return -1;
		check = fread(&sizeb, sizeof(int32_t), 1, finb);
		if (!check)
			return -1;
		if (sizea != sizeb)
			return -1;
		ma = calloc(sizea * sizea, sizeof(int32_t));
		assert(ma);
		mb = calloc(sizea * sizea, sizeof(int32_t));
		assert(mb);
		check = fread(ma, sizeof(int32_t), sizea * sizea, fina);
		if(!check)
			return -1;
		check = fread(mb, sizeof(int32_t), sizea * sizea, finb);
		if(!check)
			return -1;
		n = sizea;
	}
	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Bcast(&n,1, MPI_INT, MASTER, MPI_COMM_WORLD);
	MPI_Datatype block;
	int count = n/sqrt(size);
	int blocklen = n/sqrt(size);
	int stride = n;
	MPI_Type_vector(count, blocklen, stride, MPI_INT, &block);
	MPI_Type_commit(&block);
	int32_t** a;
	int32_t** b;
	bcastblocks(rank, size, ma, mb, block, n, a, b);





	return MPI_Finalize();
}
