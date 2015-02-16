#include <math.h>

int get_bs_32(uint32_t *buff, int index, size_t size)
{
	if(index > size-1)
		return -1;
	int offset = index/32;
	uint32_t* ptr = (uint32_t *)buff;
	ptr += offset;
	uint32_t mask = 1 << (index % 32);
	return *ptr & mask;
}

uint32_t* make_bs_32(int num_items)
{
	int len = (int) ceil(num_items/32.0);
	return (uint32_t *)calloc(len, sizeof(uint32_t));
}

int clear_bs_32(uint32_t* buff, int index, size_t size)
{
	if(index > size-1)
		return -1;
	int offset = index/32;
	uint32_t* ptr = (uint32_t*)buff;
	ptr += offset;
	uint32_t mask = 1 << (index % 32);
	*ptr &= ~mask;
	return 0;
}


int set_bs_32(uint32_t* buff, int index, size_t size)
{
	if(index > size-1)
		return -1;
	int offset = index/32;
	uint32_t* ptr = (uint32_t*)buff;
	ptr += offset;
	uint32_t mask = 1 << ((index % 32)-1);
	*ptr |= mask;
	return 0;
}
