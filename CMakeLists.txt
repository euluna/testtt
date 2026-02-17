cmake_minimum_required(VERSION 4.1)
project(synchronizationProjectSolution_advanced C)

set(CMAKE_C_STANDARD 11)

add_executable(synchronizationProjectSolution_advanced
        test.c
        mybarrier.c
        mymutex.c
        mycondvar.c
        mysemaphore.c
        spinlock.c)
