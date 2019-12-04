#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#define MASTER 0

void out(int32_t* m, int size)
{
	for(int i = 0; i < size; i++)
	{
		for(int j = 0; j<size; j++)
			printf("%d ", m[i*size + j]);
		printf("\n");
	}
}

static void Matmul(int rank, int size, int32_t* a, int32_t* b, int32_t* c, int32_t sz)
{
		for(int32_t i = 0; i<sz; i++)
		{
				for (int32_t k = 0; k<sz; k++)
				{
						int32_t r = a[i*sz + k];
						for (int32_t j = 0; j<sz; j++)
								c[i*sz + j] += r*b[k*sz + j];
				}
		}
}

static void bcastblocks(int rank, int size, int32_t* ma, int32_t* mb, MPI_Datatype block, int32_t n, int32_t* a, int32_t* b, int32_t* c)
{
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
			MPI_Send(&ma[start], 1, block, i, 0, MPI_COMM_WORLD);
			MPI_Send(&ma[start], 1, block, i, 1, MPI_COMM_WORLD);
		}
		MPI_Sendrecv(ma, 1, block, MASTER, 0, a, n * n /size, MPI_INT, MASTER, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		MPI_Sendrecv(mb, 1, block, MASTER, 1, b, n * n /size, MPI_INT, MASTER, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	}
	else
	{
		MPI_Recv(a,n*n/size, MPI_INT, MASTER, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		MPI_Recv(b,n*n/size, MPI_INT, MASTER, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	}
}

static void gatherblocks(int rank, int size, MPI_Datatype block, int32_t* mc, int32_t* c, int n)
{
	if(rank != MASTER)
	{
		MPI_Send(c, n * n/size, MPI_INT, MASTER, rank, MPI_COMM_WORLD);
	}
	else
	{
		MPI_Sendrecv(c, n * n/size, MPI_INT, MASTER, 0, mc, 1, block, MASTER, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		int32_t start = 0;
		for(int i = 1; i < size; i++)
		{
			if (i % (int)sqrt(size) == 0)
			{
				start += n * n/size;
			}
			else
			{
				start+=n/sqrt(size);
			}
			MPI_Recv(&mc[start], 1, block, i, i, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		}
	}
}

static void performCannon(int rank, int size, int32_t n, int32_t* a, int32_t* b, int32_t* c)
{
	MPI_Comm cart;
	int dims[2];
	dims[0] = dims[1] = (int)sqrt(size);
	int periods[2];
	periods[0] = periods[1] = 1;
	MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periods, 1, &cart);
	int myrank, leftrank, rightrank, uprank, downrank;
	MPI_Comm_rank(cart, &myrank);
	int coords[2];
	MPI_Cart_coords(cart, myrank, 2, coords);
	MPI_Cart_shift(cart, 0, -1, &rightrank, &leftrank);
	MPI_Cart_shift(cart, 1, -1, &downrank, &uprank);
	int nlocal = n/dims[0];

	for(int i = 0; i < dims[0]; i++)
	{
		Matmul(rank, size, a, b, c, nlocal);
		MPI_Sendrecv_replace(a, nlocal * nlocal, MPI_INT, leftrank, 1, rightrank, 1, cart, MPI_STATUS_IGNORE);
		MPI_Sendrecv_replace(b, nlocal * nlocal, MPI_INT, uprank, 1, downrank, 1, cart, MPI_STATUS_IGNORE);
	}

	MPI_Comm_free(&cart);
}

int main(int argc, char **argv){
	MPI_Init(&argc, &argv);
	int rank, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	int32_t n = 0;
	int32_t* ma = NULL;
	int32_t* mb = NULL;
	int32_t* mc = NULL;
	if (rank == MASTER)
	{
		if (argc != 4)
			return -1;
		FILE* fina = fopen(argv[1], "rb+");
		FILE* finb = fopen(argv[2], "rb+");
		if(!(fina&&finb))
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
		mc = calloc(sizea * sizea, sizeof(int32_t));
		assert(mc);
		check = fread(ma, sizeof(int32_t), sizea * sizea, fina);
		if(!check)
			return -1;
		check = fread(mb, sizeof(int32_t), sizea * sizea, finb);
		if(!check)
			return -1;
		n = sizea;
		fclose(fina);
		fclose(finb);
	}
	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Bcast(&n,1, MPI_INT, MASTER, MPI_COMM_WORLD);
	MPI_Datatype block;
	int count = n/sqrt(size);
	int blocklen = n/sqrt(size);
	int stride = n;
	MPI_Type_vector(count, blocklen, stride, MPI_INT, &block);
	MPI_Type_commit(&block);
	int32_t* a = NULL;
	int32_t* b = NULL;
	int32_t* c = NULL;
	a = calloc(n * n/size, sizeof(int32_t));
	b = calloc(n * n/size, sizeof(int32_t));
	c = calloc(n * n/size, sizeof(int32_t));
	assert(a);
	assert(b);
	assert(c);
	bcastblocks(rank, size, ma, mb, block, n, a, b, c);
	performCannon(rank, size, n, a, b, c);
	gatherblocks(rank, size, block, mc, c, n);
	MPI_Barrier(MPI_COMM_WORLD);
	if(rank == MASTER)
	{
		FILE* fout = fopen(argv[3], "wb+");
		if(!(fout))
			return -1;
		fwrite(&n, sizeof(int32_t), 1, fout);
		fwrite(mc, sizeof(int32_t), n * n, fout);
		free(ma);
		free(mb);
		free(mc);
	}
	free(a);
	free(b);
	free(c);
	return MPI_Finalize();
}
