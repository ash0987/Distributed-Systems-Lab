#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>


template <typename T>
class ThreadSafeQueue
{
private:

    std::queue<T> queue_;

    std::mutex mutex_;

    std::condition_variable cv_;

    bool shutdown_ = false;


public:

    // Add item to queue
    void push(T item)
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);

            // Assumption:
            // push() should not be called after shutdown().
            // Reader thread finishes producing before shutdown()
            // is signaled.
            if (shutdown_)
            {
                return;
            }

            queue_.push(std::move(item));
        }


        // Wake one waiting worker
        cv_.notify_one();
    }



    // Remove item from queue
    // Returns false when shutdown happened and queue is empty
    bool pop(T& out)
    {
        std::unique_lock<std::mutex> lock(mutex_);


        cv_.wait(
            lock,
            [this]()
            {
                return !queue_.empty() || shutdown_;
            }
        );


        // No more work available
        if (queue_.empty() && shutdown_)
        {
            return false;
        }


        out = std::move(queue_.front());

        queue_.pop();


        return true;
    }



    // Signal workers that no more items will arrive
    void shutdown()
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);

            shutdown_ = true;
        }


        // Wake all workers waiting on empty queue
        cv_.notify_all();
    }
};