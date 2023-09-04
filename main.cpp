#include <vector>
#include <mutex>
#include <system_error>
#include <iostream>
#include <memory>
#include <thread>
#include <random>

using namespace std; 


class Philosopher;


class Table
{
public:

    Table();

    void serve();

public:
    static const int N = 5;
    std::vector<Philosopher> philosophers;
    std::vector<std::unique_ptr<std::mutex>> forks;
    std::vector<std::unique_ptr<std::thread>> threads;
};



class Philosopher
{
public:

    Philosopher(Table &table, int index);

    void operator()();

    void think();

    void eat();

private:
    Table& _table;
    int _index;
};


Philosopher::Philosopher(Table &table, int index): _table(table), _index(index)
{

}

void Philosopher::operator()()
{

    while (true)
    {
        think();
        eat();
    }

}

void Philosopher::think()
{

        unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
        std::default_random_engine generator(seed);
        std::uniform_int_distribution<int> distribution(1, 10000);

        cout << "Philosopher " << _index << " thinking.\n";

        // Think.
        std::this_thread::sleep_for(std::chrono::milliseconds{distribution(generator)});

}

void Philosopher::eat()
{

    // Acquire mutexes.
    int left = (_index + Table::N - 1) % Table::N;
    //cout << (_index - 1) << " modulo " << Table::N << " is " << left << endl;
    auto & left_mutex = *(_table.forks[left]);

    int right = (_index + Table::N + 1) % Table::N;
    //cout << (_index + 1) << " modulo " << Table::N << " is " << right << endl;
    auto & right_mutex = *(_table.forks[right]);

    std::scoped_lock lck { left_mutex, right_mutex };

    cout << "Philosopher " << _index << " eating.\n";

    // Eat.
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator(seed);
    std::uniform_int_distribution<int> distribution(1, 10000);

    std::this_thread::sleep_for(std::chrono::milliseconds{distribution(generator)});

    // Release mutexes.

}








Table::Table()
{
    philosophers.reserve(N);
    for(int i = 0; i < N; i++)
    {
        philosophers.emplace_back(std::move(Philosopher(*this, i)));
    }

    forks.reserve(N);
    for(int i = 0; i < N; i++)
    {
        //forks.push_back(std::make_unique<std::mutex>());
        // Is this any better?
        forks.emplace_back(std::move(std::make_unique<std::mutex>()));
    }
}

void Table::serve()
{

    for(int i = 0; i < N; i++)
    {
        //threads.push_back(std::make_unique<std::thread>(philosophers[i]));
        threads.emplace_back(std::move(std::make_unique<std::thread>(philosophers[i])));
    }

    for(int i = 0; i < N; i++)
    {
        threads[i]->join();
    }
}


int main()
{

    try
    {
        Table table;
        table.serve();
    }
    catch (std::system_error & e)
    {
        std::cout << "System error: " << e.code().message() << std::endl;
    }
    catch (std::exception & e)
    {
        std::cout << "Exception: " << e.what() << std::endl;
    }
    catch (...)
    {
        std::cout << "unknown exception\n";
    }

    return 0;
}