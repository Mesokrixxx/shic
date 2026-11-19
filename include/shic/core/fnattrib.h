#pragma once

#define execOnInit __attribute__((constructor))
#define execOnExit __attribute__((destructor))
