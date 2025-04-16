#pragma once
#include <type_traits>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <vector>
#include <atomic>

#if __cplusplus >= 201703L && !defined(__cpp_lib_invoke)
namespace std {
    template <typename F, typename... Args>
    using invoke_result_t = typename result_of<F(Args...)>::type;
}
#endif

class thread_pool {
public:
    explicit thread_pool(size_t threads = std::thread::hardware_concurrency()) : stop(false) {
        for (size_t i = 0; i < threads; ++i) {
            workers.emplace_back([this] {
                while (true) {
                    std::function<void()> task;

                    {
                        std::unique_lock lock{ this->queue_mutex };
                        this->condition.wait(lock, [this] { return this->stop || !this->tasks.empty(); });
                        if (this->stop && this->tasks.empty()) return;

                        task = std::move(this->tasks.front());
                        this->tasks.pop();
                    }

                    task();
                }
            });
        }
    }

    template<class F>
    auto enqueue(F&& f) -> std::future<std::invoke_result_t<F>> {
        using return_type = std::invoke_result_t<F>;

        auto task = std::make_shared<std::packaged_task<return_type()>>(std::forward<F>(f));
        std::future<return_type> res = task->get_future();

        {
            std::unique_lock lock(queue_mutex);
            if (stop) throw std::runtime_error("enqueue on stopped thread_pool");
            tasks.emplace([task]() { (*task)(); });
        }

        condition.notify_one();
        return res;
    }

    ~thread_pool() {
        {
            std::unique_lock lock(queue_mutex);
            stop = true;
        }

        condition.notify_all();
        for (auto& worker : workers) {
            worker.join();
        }
    }

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;

    std::mutex queue_mutex;
    std::condition_variable condition;
    std::atomic<bool> stop;
};