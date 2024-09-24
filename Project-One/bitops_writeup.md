# Bit Operations Writeup
Group: Sankar Gollapudi and Rohan Karamel. Both `bitops.c` and `threads.c` were completed by Rohan Karamel.

## 1. Extracting the Top Bits: get_top_bits

```c
static unsigned int get_top_bits(unsigned int value, int num_bits) {
    return (num_bits > 0 && num_bits <= 32) ? value >> (32 - num_bits) : 0;
}
```
### Purpose:
This function extracts the most significant (top) bits from a 32-bit value.

### Input:
value - The 32-bit unsigned integer from which we want to extract the top bits.
num_bits - The number of top bits to extract.

### Operation:
The function uses a right shift operation (>>) to shift the value by (32 - num_bits) positions. This will effectively discard the lower bits and keep only the top num_bits. For example, if we want the top 4 bits, we will shift the value by 32 - 4 = 28 bits to the right, keeping the top 4 bits intact.

### Conditions:
The function checks that the requested num_bits is within a valid range (greater than 0 and less than or equal to 32). If not, it returns 0.

## 2.Setting a Bit at a Specific Index: set_bit_at_index

```c
static void set_bit_at_index(char *bitmap, int index) {
    bitmap[index / 8] = bitmap[index / 8] |= (1 << index % 8);
}
```

### Purpose:
This function sets a specific bit in a bitmap to 1 at a given index.

### Input:
bitmap - A pointer to the bitmap, which is an array of bytes (char array) representing bits.
index - The index of the bit to set.

### Operation:
The function calculates which byte in the bitmap array contains the bit at the given index by performing integer division (index / 8). Then, it uses a bitwise OR operation (|=) to set the appropriate bit within that byte. The bit to set is determined by 1 << (index % 8), which shifts the value 1 to the correct position within the byte.

## 3. Retrieving a Bit at a Specific Index: get_bit_at_index

```c
static int get_bit_at_index(char *bitmap, int index) {
    return (bitmap[index / 8] & (1 << index % 8)) != 0;
}
```

### Purpose:
This function retrieves the value of a specific bit in the bitmap at the given index (either 0 or 1).

### Input:
bitmap - A pointer to the bitmap, which is an array of bytes (char array) representing bits.
index - The index of the bit to retrieve.

### Operation:
Similar to set_bit_at_index, the function first calculates which byte in the bitmap contains the bit at the given index. It then uses a bitwise AND operation (&) to isolate the specific bit by shifting 1 to the desired position (1 << (index % 8)). The result is then checked to see if the bit is set (!= 0).
