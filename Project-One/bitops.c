/*
 * @author
 * Rohan Karamel (RAK218)
 * Sankar Gollapudi (SAG341)
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define NUM_TOP_BITS 4           // Number of top bits to extract from an address
#define BITMAP_SIZE 4            // Size of the bitmap in bytes
#define SET_BIT_INDEX 17         // Index to set in the bitmap
#define GET_BIT_INDEX 17         // Index to get in the bitmap

static unsigned int myaddress = 4026544704;

/*
 * Function to extract the top `num_bits` bits from an unsigned integer value.
 * @param value: The input value to extract the bits from.
 * @param num_bits: The number of top bits to extract.
 * @return: The extracted top bits as an unsigned integer.
 */
static unsigned int get_top_bits(unsigned int value,  int num_bits)
{
    // Ensure num_bits is within the valid range (1 to 32), otherwise return 0
    return (num_bits > 0 && num_bits <= 32) ? value >> (32 - num_bits) : 0;
}

/*
 * Function to set a bit at the specified index in a bitmap.
 * @param bitmap: The input bitmap (an array of bytes).
 * @param index: The index of the bit to set.
 */
static void set_bit_at_index(char *bitmap, int index)
{
    // Set the bit at the given index in the bitmap
    bitmap[index / 8] = bitmap[index / 8] |= (1 << index % 8);
}

/*
 * Function to get the value of a bit at the specified index in a bitmap.
 * @param bitmap: The input bitmap (an array of bytes).
 * @param index: The index of the bit to retrieve.
 * @return: 1 if the bit is set, 0 otherwise.
 */
static int get_bit_at_index(char *bitmap, int index)
{
    // Retrieve the value of the bit at the given index in the bitmap
    return (bitmap[index / 8] & (1 << index % 8)) != 0;
}

int main () {
    /* 
     * Function 1: Finding value of top order (outer) bits Now let’s say we
     * need to extract just the top (outer) 4 bits (1111), which is decimal 15  
     */
    unsigned int outer_bits_value = get_top_bits(myaddress, NUM_TOP_BITS);
    printf("Function 1: Outer bits value %u \n", outer_bits_value);
    printf("\n");

    /* 
     * Function 2 and 3: Checking if a bit is set or not
     */
    char *bitmap = (char *)malloc(BITMAP_SIZE);  //We can store 32 bits (4*8-bit per character)
    memset(bitmap,0, BITMAP_SIZE); //clear everything
    
    /* 
     * Let's try to set the bit 
     */
    set_bit_at_index(bitmap, SET_BIT_INDEX);
    
    /* 
     * Let's try to read bit
     */
    printf("Function 3: The value at %dth location %d\n", 
            GET_BIT_INDEX, get_bit_at_index(bitmap, GET_BIT_INDEX));
            
    return 0;
}
