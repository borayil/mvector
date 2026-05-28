header-only, minimalistic vector implementation to lock heap allocated memory in RAM and also zero out the sensitive data where needed.

```c++
#include "mvector.hpp"
mvector<double> high_secret_doubles;
```

Calls the POSIX standard [`mlock`](https://man7.org/linux/man-pages/man2/mlock.2.html) to prevent the vector internal memory from being swapped out from RAM

Instead of `std::fill_n`, 
[`explicit_memset`](https://en.cppreference.com/c/string/byte/memset) could be used.