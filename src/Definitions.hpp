#pragma once

#ifndef NDEBUG
#define MY_ASSERT(x) assert(x)
#else
#define MY_ASSERT(x) {}
#endif // _DEBUG
