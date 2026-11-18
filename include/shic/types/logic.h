#pragma once

#define type_isPtr(x) (__builtin_classify_type(x) == 5)
