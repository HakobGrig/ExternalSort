#ifndef _UTILS_EX_SORT_
#define _UTILS_EX_SORT_

bool is_big_endian() {
    union {
		uint32_t i;
		char c[4];
    } bint = {0x01020304};

    return bint.c[0] == 1;
}

template <typename T>
void FixUnaligned(T& element, uint32_t realbytecount) {
	if (is_big_endian()) {
		element >>= 8*(sizeof(T)-realbytecount);
	} else {
		element <<= 8*(sizeof(T)-realbytecount);
		element >>= 8*(sizeof(T)-realbytecount);
	}
}

#endif // _UTILS_EX_SORT_
