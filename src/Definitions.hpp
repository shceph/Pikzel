#pragma once

#ifndef NDEBUG
#define MY_ASSERT(x) assert(x)
#else
#define MY_ASSERT(x) ((void)(x))
#endif // _DEBUG
