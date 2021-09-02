/*
MIT License
Copyright (c) 2013 Peter Bjorklund
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#include <swamp-typeinfo/fnv.h>

#include <tiny-libc/tiny_libc.h>

static const swtiStringHash FNV_PRIME = 16777619u;
static const swtiStringHash OFFSET_BASIS = 2166136261u;

// Similar to FNV-1a https://en.wikipedia.org/wiki/Fowler-Noll-Vo_hash_function

swtiStringHash swtiHash(const char* str)
{
	const size_t length = tc_strlen(str) + 1;
	swtiStringHash hash = OFFSET_BASIS;
	for (size_t i = 0; i < length; ++i) {
		hash ^= (uint8_t) *str++;
		hash *= FNV_PRIME;
	}
	return hash;
}
