/*
 * threadpool.h
 *  thread pool adapted from 
 *  C++ Concurrency in Action, Anthony Williams
 *  Created on: 2013-07-08
 *      Author: ede cameron
 *      ecameron@gmail.com
 */

#ifndef THREAD_POOL_HPP_
#define THREAD_POOL_HPP_


#include <memory>
#include <queue>
#include <thread>
#include <atomic>
#include <iostream>
#include <future>

#include "thread_safe_queue.hpp"
#include "join_threads.hpp"
#include "port.h"

class thread_pool {

 thread_safe_queue<std::packaged_task<float**() >> ts_queue;
 std::atomic_bool done;

 std::vector<std::thread> threads;
 join_threads joiner;

  unsigned int thread_count;

  void worker_thread()
    {
       while(!done){

	   std::shared_ptr<std::packaged_task<float**() >> task(lfqueue.try_pop());
                if(task){
	           this->thread_function(std::move(*task));}
             else {
                std::this_thread::yield();
             }
          }
      }

public:

thread_pool(unsigned int t):
 
	  done(false),
          joiner(threads),
    	  thread_count(t)
          ts_queue(),

    {
        try
        {
            for(unsigned i=0;i<thread_count;++i)
            {
               threads.push_back(
               std::thread(&thread_pool::worker_thread,this));

            }
        }
        catch(...)
        {
            done=true;
            throw;
        }
    }

     ~thread_pool() {
        
        done=true;

    }


std::future<float**> run_task(std::function<float**()> in_task)
    {

        std::packaged_task<float**()> task(std::bind(in_task));
        std::future<float**> res(task.get_future());
        lfqueue.push(std::move(task));
             return res;
    }


void thread_function(std::packaged_task<float**() > task){
		
                   task();
	}
};

extern thread_pool *pool;

#endif /* THREAD_POOL_HPP_ */



