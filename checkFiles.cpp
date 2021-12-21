#include <algorithm>
#include <iterator>
#include <string>
#include <fstream>
#include <iostream>

template<typename InputIterator1, typename InputIterator2>
bool
range_equal(InputIterator1 first1, InputIterator1 last1,
            InputIterator2 first2, InputIterator2 last2) {
    while (first1 != last1 && first2 != last2) {
        if (*first1 != *first2) return false;
        ++first1;
        ++first2;
    }
    return (first1 == last1) && (first2 == last2);
}

bool compare_files(const std::string &filename1, const std::string &filename2) {
    std::ifstream file1(filename1);
    std::ifstream file2(filename2);

    std::istreambuf_iterator<char> begin1(file1);
    std::istreambuf_iterator<char> begin2(file2);

    std::istreambuf_iterator<char> end;

    return range_equal(begin1, end, begin2, end);
}


int main(int argc, char *argv[]) {
    if(compare_files(argv[1], argv[2])){
        std::cout << "Test Case Passed" << std::endl;
    }else{
        std::cout << "Test Case Failed" << std::endl;
    }

    return 0;
}
