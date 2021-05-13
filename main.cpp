#include <iostream>

#include "DataFrame.h"
using namespace dataframe;

int main() {
    DataFrame df;
    df["Basic"] = std::vector<double>{1., 2., 3., 4., 5.};
    df["More"] = std::vector<double>{21., 18., 4., 17., 0.};
    df["Advanced"] = std::vector<char>{'a', 'c', 'n', 'z', 'e'};
    df["Boolean"] = std::vector<bool>{true, false, false, true, true};
    df["Test"].Set("");
    try {
        std::cout << "Try to add a column of the wrong size. Should fail:\n";
        df["BadSize"] = std::vector<double>{ 3. };
    }
    catch (...) {
        std::cout << " >> Could not create column, sizes mismatch.\n\n";
    }
    // The call df["BadSize"] created a None column.
    std::cout << "Trying to add the column of the wrong size will have added a NONE column. Drop this:\n";
    std::cout << " >> Dropped " << df.DropNones() << " NONE column.\n";

    std::cout << "\nPrint DataFrame:\n";
    df.ToStream(std::cout);
    std::cout << "\nSelect subset of df where Boolean is False, and set Test to \"FFF\".\n";
    std::cout << "Then, select subsets and add columns to them, showing that adding columns to"
                 "\nanother DataFrame doesn't add columns to the original data frame.\n\n";
    auto view = df[df["Boolean"] == false];
    view["Test"].Set("FFF");
    view["NewAddition"].Set(1);
    auto small_view = view[view["More"] == 4];
    auto small_view_2 = small_view;
    small_view["OneMore"].Set("K");
    df.ToStream(std::cout);
    std::cout << "------\n";
    view.ToStream(std::cout);
    std::cout << "------\n";
    small_view.ToStream(std::cout);
    std::cout << "------\n";
    small_view_2.ToStream(std::cout);

    auto save1 = df["Basic"];
    auto save2 = df["Basic"];
    df["Basic"] = std::vector<double>{3., 2., 1., 4., 5.};
    save2 = std::vector<std::string>{"hello", "what's", "up?", "how", "you?" };

    // Test appending a DataFrame to another DataFrame.
    std::cout << "\nTest appending a DataFrame to another DataFrame:\n";
    DataFrame df2;
    df2["Basic"] = std::vector<std::string> { "punky", "bean" };
    df2["Boolean"] = std::vector<bool> { true, true };
    df2.Append(df);
    df2.ToStream(std::cout);

    // Test appending some data to a DataFrame.
    std::cout << "\nTest appending a row to a DataFrame. Successfully added? ";
    std::cout << std::boolalpha << df2.Append("another", false) << "!\nResult:\n";
    df2.ToStream(std::cout);


    std::cout << "\n\nTest on Sacramento Real Estate data.\n\n";

    auto df_realestate = DataFrame::ReadCSV("../TestData/sacramento_real_estate.csv");
    auto col_names = df_realestate.Columns();
    auto col_dtypes = df_realestate.DTypes();
    std::cout << "Columns:\n";
    for (std::size_t i = 0; i < col_names.size(); ++i) {
        std::cout << "\t\"" << col_names[i] << "\", dtype = " << col_dtypes[i] << "\n";
    }
    std::cout << "\n";

    std::cout << "Look the number of bedrooms in a house.\n";
    std::size_t total = 0;
    auto num_beds = df_realestate["beds"].Unique<int>();
    for (int beds : num_beds) {
        auto df_bed = df_realestate[beds == df_realestate["beds"]];
        std::cout << "\tThere are " << df_bed.NumRows() << " houses with " << beds << " bedrooms.\n";
        total += df_bed.NumRows();
    }
    std::cout << "Total count, " << total << " houses, should get " << df_realestate.NumRows() << ".\n";

    auto df_zero = df_realestate[(df_realestate["beds"] == 3) & (df_realestate["type"] == "Condo")];
    std::cout << "\nProperties with three bedrooms that are condos:\n";
    df_zero.ToStream(std::cout);

    // You can cast a column to a vector of a specified type. The vector will be empty if the conversion failed.
    auto data_float = df["More"].CastToVector<float>();
    auto data_int = df["More"].CastToVector<int>();
    auto data_bool = df["More"].CastToVector<bool>();
    auto data_string = df["More"].CastToVector<std::string>();
    auto data_char = df["More"].CastToVector<char>();
    auto data_long = df["More"].CastToVector<long>();
    auto data_double = df["More"].CastToVector<double>();

    // If you append a dataframe to an empty dataframe, the first dataframe should just become the appended
    // dataframe.
    DataFrame starting_df;
    starting_df.Append(df);
    starting_df.Append(df);

    auto more = starting_df["More"].GetConcrete<double>();
    auto more_vec = starting_df["More"].CastToVector<int>();
    auto x = more[4];
    return 0;
}
