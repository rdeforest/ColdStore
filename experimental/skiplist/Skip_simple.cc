#include <iostream>
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdexcept>

#include "SkipList.hh"

template class Qs<int>;
template class Qsh<int>;
template union lptr<int>;
