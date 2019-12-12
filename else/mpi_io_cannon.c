#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#define MASTER 0

void out(int rank, int size, int32_t* m)
{
	printf("rank %d\n", rank);
	for (int  i = 0; i < size; i++)
	{
		for (int j = 0; j< size; j++)
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

static void performCannon(int rank, int size, int32_t n, int32_t* a, int32_t* b, int32_t* c)
{
	MPI_Comm cart;

	int dims[2];
	dims[0] = dims[1] = (int)sqrt(size);
	int periods[2];
	periods[0] = periods[1] = 1;
	MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periods, 1, &cart);

	int myrank, leftrank, rightrank, uprank, downrank, shiftsource, shiftdest;
	MPI_Comm_rank(cart, &myrank);
	int coords[2];

	MPI_Cart_coords(cart, myrank, 2, coords);

	int nlocal = n/dims[0];
	MPI_Cart_shift(cart, 1, -coords[0], &shiftsource, &shiftdest);

	MPI_Sendrecv_replace(a, nlocal * nlocal, MPI_INT, shiftdest, 1, shiftsource, 1, cart, MPI_STATUS_IGNORE);
	MPI_Cart_shift(cart, 0, -coords[1], &shiftsource, &shiftdest);
	MPI_Sendrecv_replace(b, nlocal * nlocal, MPI_INT, shiftdest, 1, shiftsource, 1, cart, MPI_STATUS_IGNORE);
	MPI_Cart_shift(cart, 1, -1, &rightrank, &leftrank);
	MPI_Cart_shift(cart, 0, -1, &downrank, &uprank);
	for(int i = 0; i < dims[0]; i++)
	{
		Matmul(rank, size, a, b, c, nlocal);
		MPI_Barrier(cart);
		MPI_Sendrecv_replace(a, nlocal * nlocal, MPI_INT, leftrank, 1, rightrank, 1, cart, MPI_STATUS_IGNORE);
		MPI_Barrier(cart);
		MPI_Sendrecv_replace(b, nlocal * nlocal, MPI_INT, uprank, 1, downrank, 1, cart, MPI_STATUS_IGNORE);
		MPI_Barrier(cart);
	}
	MPI_Comm_free(&cart);
}

int main(int argc, char **argv){
	MPI_Init(&argc, &argv);
	int rank, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	int32_t n = 0;
	MPI_File fa, fb, fc;
	MPI_Offset offset = 0;
	MPI_File_open(MPI_COMM_WORLD, argv[1], MPI_MODE_RDONLY, MPI_INFO_NULL, &fa);
	MPI_File_open(MPI_COMM_WORLD, argv[2], MPI_MODE_RDONLY, MPI_INFO_NULL, &fb);
	MPI_File_read_at_all(fa, offset, &n, 1, MPI_INT, MPI_STATUS_IGNORE);
	MPI_Datatype block;
	int count = n/sqrt(size);
	int blocklen = n/sqrt(size);
	int stride = n;
	MPI_Type_vector(count, blocklen, stride, MPI_INT, &block);
	MPI_Type_commit(&block);
	offset = 1;
	int goffset = n * n/sqrt(size) - ((int)sqrt(size)) * n/sqrt(size);
	offset += rank * n/sqrt(size) + rank /(int)sqrt(size) * goffset;
	offset*=4;
	goffset = offset;
	MPI_File_set_view(fa, offset, MPI_INT, block, "native", MPI_INFO_NULL);
	MPI_File_set_view(fb, offset, MPI_INT, block, "native", MPI_INFO_NULL);
	int32_t* a = NULL;
	int32_t* b = NULL;
	int32_t* c = NULL;
	a = calloc(n * n/size, sizeof(int32_t));
	b = calloc(n * n/size, sizeof(int32_t));
	c = calloc(n * n/size, sizeof(int32_t));
	assert(a);
	assert(b);
	assert(c);
	for(int i = 0; i < count; i++)
	{
		MPI_File_read(fa, &a[i * count], blocklen, MPI_INT, MPI_STATUS_IGNORE);
		MPI_File_read(fb, &b[i * count], blocklen, MPI_INT, MPI_STATUS_IGNORE);
		offset += n*4;
		MPI_File_set_view(fa, offset, MPI_INT, MPI_INT, "native", MPI_INFO_NULL);
		MPI_File_set_view(fb, offset, MPI_INT, MPI_INT, "native", MPI_INFO_NULL);
	}
	MPI_File_close(&fa);
	MPI_File_close(&fb);
	double start = MPI_Wtime();
	performCannon(rank, size, n, a, b, c);
	start = MPI_Wtime() - start;
	MPI_File_open(MPI_COMM_WORLD, argv[3], MPI_MODE_WRONLY|MPI_MODE_CREATE, MPI_INFO_NULL, &fc);
	if(rank == 0)
	{
		MPI_File_write(fc, &n, 1, MPI_INT, MPI_STATUS_IGNORE);
	}
	MPI_File_set_view(fc, goffset, MPI_INT, block, "native", MPI_INFO_NULL);
	for(int i = 0; i < count; i++)
	{
		MPI_File_write(fc, &c[i * count], blocklen, MPI_INT, MPI_STATUS_IGNORE);
		goffset += n*4;
		MPI_File_set_view(fc, goffset, MPI_INT, MPI_INT, "native", MPI_INFO_NULL);
	}
	MPI_Barrier(MPI_COMM_WORLD);
	MPI_File_close(&fc);
	free(a);
	free(b);
	free(c);
	return MPI_Finalize();
}
