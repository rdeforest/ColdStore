#include <iostream>
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdexcept>

#include "SkipList.hh"

const int sampleSize = 10000;
const int nr_threads = 10;
const int nrPasses = 1000;

SkipList<int> *l;

void throwing(char *err)
{
    throw runtime_error(err);
}

void addone(int *keys, int k, int &num)
{
    SkipEl<int> *probe;
    keys[k] = num++;
    while (!l->Insert(keys[k])) {
        cerr << "warning: inserted duplicate\n";
        num = keys[k] = random();
    }
    if (!l->Search(keys[k])) {
        throwing("Insert Failed to find!!");
    }
}

void prime(int *keys, int &count)
{
    for(int k=0; k < sampleSize; k++) {
        addone(keys, k, count);
    }
}

void shuffle(int *keys, int &count)
{
    int k = random() % sampleSize;
    if (!l->Search(keys[k])) {
        throwing("Insert Failed to find!!");
    }
    if (!l->Delete(keys[k]))
        throwing("Failed to delete");
    addone(keys, k, count);
}

void *exercise(void *)
{
    int keys[sampleSize];
    int counter = random();
    try {
        prime(keys, counter);
        for (int i = nrPasses; i; i--) {
            shuffle(keys, counter);
            if ((counter % 100) && sched_yield()) {
                throwing("yield failed\n");
            }
        }
    } catch (std::exception &e) {
        std::cerr << "exception: "
                  << e.what()
                  << std::endl;
    }
    return NULL;
}

main()
{
    l = new SkipList<int>(8);
    pthread_t threads[nr_threads];
    
    for (int i = 0; i < nr_threads; i++) {
        int pid = pthread_create(threads + i, NULL,  exercise, NULL);
        if (pid < 0) {
            cerr << "pid: " << pid 
                 << " thread: " << threads[i]
                 << '\n';
        }
    }
    for (int kid = 0; kid < nr_threads; kid++) {
        int status;
        wait(&status);
    }
}

#include "SkipList.th"
template class SkipEl<int>;
template class SkipList<int>;
template union lptr<int>;

