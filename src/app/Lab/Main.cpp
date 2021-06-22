#include "Common.h"
#include "StringConvert.h"
#include "StringFormat.h"
#include "Timer.h"
#include "Log.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <random>
#include <vector>
#include <algorithm>
#include <thread>

namespace fs = std::filesystem;

namespace
{
    using NumbersTemplate = std::vector<uint32>;

    NumbersTemplate _numbers;
    NumbersTemplate _numbers1;
    NumbersTemplate _numbers2;

    constexpr auto FILE_PATH = "RandomNumbers.txt";
    constexpr auto numbersCount = 5000000;
    constexpr auto numbersMin = 1;
    constexpr auto numbersMax = 2000;
}

inline auto GetTimeDiff(SystemTimePoint timePoint)
{
    using namespace std::chrono;
    // Get End Time
    auto end = system_clock::now();
    return duration_cast<Microseconds>(end - timePoint).count();
}

void GenerateFile()
{
    // Path to file
    fs::path path = fs::path(FILE_PATH);
    std::ofstream file(path);

    // Get start time
    auto startTime = std::chrono::system_clock::now();
    std::random_device random_device; // Source of entropy
    std::mt19937 generator(random_device()); // ГСЧ

    file << Warhead::ToString(numbersCount) + "\n"; // Print numbersCount in file

    for (int i = 0; i < numbersCount; i++)
    {
        std::uniform_int_distribution<> distribution(numbersMin, numbersMax);
        auto number = distribution(generator);
        file << Warhead::ToString(number) + " ";
    }

    file.close();

    fmt::print("# -- File created in {}\n", Warhead::Time::ToTimeString<Microseconds>(GetTimeDiff(startTime), TimeOutput::Microseconds));
}

void GetNumbers()
{
    auto fileText = Warhead::File::GetFileText(FILE_PATH);
    auto found = fileText.find_first_of('\n');

    if (found != std::string::npos)
        fileText = fileText.substr(found + 2LL);
    else
    {
        fmt::print("> In file {} no found array\n", FILE_PATH);
        return;
    }

    for (auto str : Warhead::Tokenize(fileText, ' ', false))
    {
        auto number = Warhead::StringTo<uint32>(str);
        if (!number)
        {
            fmt::print("> Number {} is incorrect!", str);
            continue;
        }

        _numbers.emplace_back(*number);
    }

    uint32 count = 0;

    for (auto const& itr : _numbers)
    {
        count++;

        if (count >= numbersCount / 2 + 1)
            break;

        _numbers1.emplace_back(itr);
    }

    for (size_t i = numbersCount / 2; i < _numbers.size(); i++)
    {
        _numbers2.emplace_back(_numbers.at(i));
    }
}

void CheckFile()
{
    auto resultMinIndex = std::distance(_numbers.begin(), std::min_element(_numbers.begin(), _numbers.end()));
    int minElement = _numbers.at(resultMinIndex);
    int minCount = std::count(_numbers.begin(), _numbers.end(), minElement);
    fmt::print("> Min element at: {}. Count: {}\n", minElement, minCount);
}

void CheckFile1()
{
    auto resultMinIndex = std::distance(_numbers1.begin(), std::min_element(_numbers1.begin(), _numbers1.end()));
    int minElement = _numbers1.at(resultMinIndex);
    int minCount = std::count(_numbers1.begin(), _numbers1.end(), minElement);
    fmt::print("> Min element at: {}. Count: {}\n", minElement, minCount);
}

void CheckFile2()
{
    auto resultMinIndex = std::distance(_numbers2.begin(), std::min_element(_numbers2.begin(), _numbers2.end()));
    int minElement = _numbers2.at(resultMinIndex);
    int minCount = std::count(_numbers2.begin(), _numbers2.end(), minElement);
    fmt::print("> Min element at: {}. Count: {}\n", minElement, minCount);
}

int main()
{
    GenerateFile();
    GetNumbers();

    // Get start time
    auto startTime = std::chrono::system_clock::now();
    CheckFile(); //1 thread
    uint64 time1 = GetTimeDiff(startTime);
    fmt::print("# CheckFile done with 1 thread in {}\n", Warhead::Time::ToTimeString<Microseconds>(time1, TimeOutput::Microseconds));

    startTime = std::chrono::system_clock::now();
    std::thread thread1(CheckFile1);
    std::thread thread2(CheckFile2);

    thread1.join();
    thread2.join();

    uint64 time2 = GetTimeDiff(startTime);

    fmt::print("# CheckFile done with 2 thread in {}\n", Warhead::Time::ToTimeString<Microseconds>(time2, TimeOutput::Microseconds));

    if (time1 > time2)
        fmt::print("> 2 threads faster 1 in: {}. \n", float(time1) / float(time2));
    else if (time2 > time1)
        fmt::print("> 1 thread faster 2 in: {}. \n", float(time2) / float(time1));

    return 0;
}
