## Introduction

DataFrames is a pandas DataFrames inspired library for loading and manipulating heterogeneous data.
The basic class is a DataFrame, which implements the functionality that one would expect: you can 
read data from a file into a DataFrame, write a DataFrame to files, select subsets of a DataFrame based
on conditions, access individual columns, add or remove columns, etc. 


## Basic Usages

Reading and writing DataFrames:
```
auto df = DataFrame.ReadCSV("filename.csv");
// ... 
df.ToCSV("fileout.csv");
```
DataFrames can be written to any ostream, e.g.
```
df.ToStream(std::cout);
```
or read from any istream
```
std::ifstream fin("filename.csv");
DataFrame::FromStream(fin);
```

To add columns to a dataframe, use the access operator and set the resulting column equal to a vector.
```
DataFrame df;
df["Basic"] = std::vector<double>{1., 2., 3., 4., 5.};
df["More"] = std::vector<double>{21., 18., 4., 17., 15.};
df["Advanced"] = std::vector<char>{'a', 'c', 'n', 'z', 'e'};
df["Boolean"] = std::vector<bool>{true, false, false, true, true};
```
If you attempt to add a column with the wrong size, this is an error.

You can also add data by creating an entire column with the same value.
```
df["Test"].Set("");
``` 

You can select data with any vector of bools that is the same length as the length of the columns in the DataFrame.
The easy way to create one of these condition vectors (typedef'ed as Indicator) is by comparing a column with a value.
```
auto view = df[(15. <= df["More"]) & (df["Basic"] < 5)];
```
Multiple conditions can be strung together with &, |, or ^, and Indicators can be negated with ~.