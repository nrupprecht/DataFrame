#include <iostream>

#include "DataFrame.h"

double f(int, const std::vector<int>&) {
    return 0.;
}


int main() {
    DataFrame df;
    df["Basic"] = std::vector<double>{1., 2., 3.};
    df["More"] = std::vector<double>{2., 1., 4.};
    df["Advanced"] = std::vector<char>{'a', 'b', 'c'};
    df["Boolean"] = std::vector<bool>{true, false, false};
    df["Test"] = std::vector<std::string>{ "True", "False", "True" };
    try {
        df["BadSize"] = std::vector<double>{ 3. };
    }
    catch (...) {
        std::cout << "Could not create column, sizes mismatch.\n\n";
    }

    auto id1 = df["Basic"] < 2.5;

    auto id2 = df["Basic"] < df["More"];

    auto save1 = df["Basic"];
    auto save2 = df["Basic"];

    df["Basic"] = std::vector<double>{3., 2., 1.};

    save2 = std::vector<std::string>{"hello", "what's", "up?"};

    auto concrete1 = save1.GetConcrete<std::string>();
    int i = 0;
    for (const auto& str : concrete1) {
        std::cout << i << ") " << str << "\n";
        ++i;
    }
    std::cout << "\n";

    auto concrete2 = df["Boolean"].GetConcrete<bool>();
    i = 0;
    for (const auto& x : concrete2) {
        std::cout << i << ") " << std::boolalpha << (x == 1) << "\n";
        ++i;
    }
    std::cout << "\n";

    df.ToCSV("../Test.csv");

    df.ToStream(std::cout);

    auto df_in = DataFrame::ReadCSV("../Data.csv");
    std::cout << "\n\nRead dataframe from file:\n---\n";
    df_in.ToStream(std::cout);
    std::cout << "---\n";

    // Test appending a DataFrame to another DataFrame.
    DataFrame df2;
    df2["Basic"] = std::vector<std::string> { "punky", "bean" };
    df2["Boolean"] = std::vector<bool> { true, true };
    df2.Append(df);
    df2.ToStream(std::cout);

    // Test appending some data to a DataFrame.
    std::cout << "\nCan we add to df2? ";
    std::cout << std::boolalpha << df2.Append("another", false) << "!\nResult:\n";
    df2.ToStream(std::cout);

    // std::cout << "\nTesting Type Details:\n";
    // std::cout << TypeDetails<std::vector<std::pair<const volatile int* const* const, float const>>>().description << "\n";
    // std::cout << TypeDetails<decltype(&f)>().description << "\n";
    return 0;
}
