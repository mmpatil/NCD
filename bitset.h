#include <math.h>
/**
 * returns the value of the indexth bit in a 32 bit bitset
 * starting at buff
 * @param buff a pointer to the beginning of a bitset
 * @param index the index of the bit to be queried
 * @param size	the number of items in the bitset
 * @return the value of the indexth bit in the bitset (o or 1)
 */
int get_bs_32(uint32_t *buff, int index, size_t size)
{
	if(index > size - 1)
		return -1;
	int offset = index / 32;
	uint32_t* ptr = (uint32_t *) buff;
	ptr += offset;
	uint32_t mask = 1 << (index % 32);
	return *ptr & mask;
}


/**
 * creates a bitset of num_items using 32 bit integers
 * @param num_items the number of items in the bitset
 * @return a pointer to the beggining of the bitset
 */
uint32_t* make_bs_32(int num_items)
{
	int len = (int) ceil(num_items / 32.0);
	return (uint32_t *) calloc(len, sizeof(uint32_t));
}

/**
 * clears the bit at index
 * @param buff pointer to the beginning of the bitset
 * @param index the bit to be cleared
 * @param size the size of the bitset
 * @return 0 for success, -1 for failure/error
 */
int clear_bs_32(uint32_t* buff, int index, size_t size)
{
	if(index > size - 1)
		return -1;
	int offset = index / 32;
	uint32_t* ptr = (uint32_t*) buff;
	ptr += offset;
	uint32_t mask = 1 << (index % 32);
	*ptr &= ~mask;
	return 0;
}

/**
 * sets the bit at index to 1
 * @param buff pointer to the beginning of the bitset
 * @param index the bit to be cleared
 * @param size the size of the bitset
 * @return 0 for success, -1 for failure/error
 */
int set_bs_32(uint32_t* buff, int index, size_t size)
{
	if(index > size - 1)
		return -1;
	int offset = index / 32;
	uint32_t* ptr = (uint32_t*) buff;
	ptr += offset;
	uint32_t mask = 1 << ((index % 32) - 1);
	*ptr |= mask;
	return 0;
}
