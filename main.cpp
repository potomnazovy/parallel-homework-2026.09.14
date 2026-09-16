#include <iostream>
#include <chrono>
#include <cstddef>
#include <vector>
#include <string>
#include <future>
#include <functional>
#include <algorithm>
#include "clicker.hpp"

using data_t = std::vector< unsigned long long >;
using value_t = data_t::value_type;

value_t worker(size_t lower_ind, size_t upper_ind, const data_t& values)
{
  value_t sum = 0;
  for (size_t i = lower_ind; i < upper_ind; ++i)
  {
    sum += values[i];
  }

  return sum;
}

int main(int argc, char* argv[])
{
  if (argc != 2)
  {
    std::cerr << "Too much" << '\n';
    return 1;
  }

  unsigned int threads = 0;
  try
  {
    threads = std::stoul(argv[1]);
    if (threads == 0)
    {
      throw std::invalid_argument("Num of threads can not be 0");
    }
  }
  catch (const std::exception& e)
  {
    std::cerr << "Invalid input" << e.what() << '\n';
    return 1;
  }

  std::cout << "Starting with " << threads << " threads...\n";

  constexpr size_t size = 1'000'000'000;
  double init = 0, total = 0;

  std::vector< double > time;
  data_t values(size, 1);

  value_t sum = 0;

  for (size_t j = 0; j < 5; ++j)
  {
    sum = 0;
    {
      mtt::Clicker cl;

      init = cl.millisec();

      std::vector< std::future< value_t > > result;
      result.reserve(threads);

      size_t chunk = size / threads;

      for (size_t i = 0; i < threads; ++i)
      {
        if (i == threads - 1)
        {
          result.emplace_back(std::async(
            std::launch::async, worker, (threads - 1) * chunk, size, std::cref(values)));
        }
        else
        {
          result.emplace_back(std::async(
            std::launch::async, worker, i * chunk, (i + 1) * chunk, std::cref(values)));
        }
      }

      for (size_t i = 0; i < result.size(); ++i)
      {
        sum += result[i].get();
      }

      total = cl.millisec();
      time.push_back(total - init);
    }
  }

  std::sort(time.begin(), time.end());

  std::cout << "Median for " << threads << " threads is " << time[2] << '\n';
  std::cout << "Total sum: " << sum << '\n';
  return 0;
}
