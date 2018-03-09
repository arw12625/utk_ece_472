#include <stdlib.h>
#include <stddef.h>


ptrdiff_t maxVal(ptrdiff_t a, ptrdiff_t b) {
	return (a < b) ? b : a;
}

ptrdiff_t minVal(ptrdiff_t a, ptrdiff_t b) {
	return (a < b) ? a : b;
}