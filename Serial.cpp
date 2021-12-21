#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <thread>
#include <chrono>

void WriteArray(const std::vector<std::vector<int>> &Data, int gen, std::ofstream &output) {
    output << "Generation " << gen + 1 << ":" << std::endl;
    for (auto &i: Data) {
        for (int j: i) {
            char x = '.';
            if (j == 1)
                x = '*';
            output << x << '\t';
        }
        output << std::endl;
    }
    output << std::endl << std::endl;
}

void ToFile(std::vector<std::vector<std::vector<int>>> Generations, const std::string &fileName) {
    std::ofstream output(fileName);
    for (int i = 0; i < Generations.size(); i++) {
        WriteArray(Generations[i], i, output);
    }
    output.close();
}


int main(int argc, char *argv[]) {

    int N, generations;
    if (argc < 2) {
        std::cout << "No Input File Specified" << std::endl;
        return -1;
    }

    std::fstream file;
    file.open(argv[1]);
    if (!file) {
        std::cout << "Error Opening File" << std::endl;
        return -1;
    }

    if (file.peek() == std::ifstream::traits_type::eof()) {
        std::cout << "Please Check your file" << std::endl;
        return -1;
    }

    file >> N >> generations;
    //std::vector<std::vector<std::vector<int> > > Storage(generations, std::vector<std::vector<int> >(N, std::vector<int>(N)));
    std::vector<std::vector<std::vector<int> > > Storage;
    if (file.peek() == std::ifstream::traits_type::eof() || N == -1 || generations == -1) {
        std::cout << "Please Check your input file" << std::endl;

        return -1;
    }

    if (N < 2 || generations < 1) {
        std::cout << "Board Size cannot Be Less Than 2\nNumber of Generations cannot be less than 1" << std::endl;
        return -1;
    }


    std::string fullname = argv[1];
    std::string testName = fullname.substr(0, fullname.find_last_of('.'));
    testName = testName.substr(testName.find_last_of("/\\") + 1);

    std::vector<std::vector<int>> grid(N, std::vector<int>(N));
    for (int i = 0; i < N; ++i) {
        std::string temp;
        file >> temp;
        for (int j = 0; j < N; ++j) {
            grid[i][j] = temp[j] - '0';
        }
    }
    file.close();

    auto start = std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch());
    int rowSize = N;
    int colSize = N;
    std::vector<int> ROW_Offset = {-1, 0, 1, 0, -1, 1, 1, -1};
    std::vector<int> COL_Offset = {0, 1, 0, -1, 1, 1, -1, -1};
    int DEAD = 0;
    int ALIVE = 1;
    int currentGen = 0;
    int FutureGrid[N][N];
    while (currentGen < generations) {
        //Game Loop
        for (int i = 0; i < rowSize; i++) {
            for (int j = 0; j < colSize; j++) {
                int neighbourAliveCount = 0;
                //Check Neighbours;
                for (int k = 0; k < ROW_Offset.size(); ++k) {
                    int newRow = i + ROW_Offset[k];
                    int newCol = j + COL_Offset[k];
                    if (newRow == -1)
                        newRow = rowSize - 1;
                    else if (newRow == rowSize)
                        newRow = 0;
                    if (newCol == -1)
                        newCol = colSize - 1;
                    else if (newCol == colSize)
                        newCol = 0;
                    neighbourAliveCount += grid[newRow][newCol];
                }

                if (grid[i][j] == ALIVE) {
                    if ((neighbourAliveCount < 2 || neighbourAliveCount > 3)) {
                        FutureGrid[i][j] = DEAD;
                    } else if ((neighbourAliveCount == 2 || neighbourAliveCount == 3)) {
                        FutureGrid[i][j] = ALIVE;
                    } else {
                        FutureGrid[i][j] = grid[i][j];
                    }
                } else {
                    if (neighbourAliveCount == 3) {
                        FutureGrid[i][j] = ALIVE;
                    } else {
                        FutureGrid[i][j] = grid[i][j];
                    }
                }
            }
        }

        //Copy new board to old board
        for (int i = 0; i < rowSize; ++i) {
            for (int j = 0; j < colSize; ++j) {
                grid[i][j] = FutureGrid[i][j];
            }
        }

        //Write to file.
        Storage.push_back(grid);



        //increment generation counter
        currentGen++;
    }


    auto end = std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch());
    auto diff = end - start;
    ToFile(Storage, testName + "_Serial_Output.txt");

    std::ofstream results("Results.txt", std::ios_base::app);
    results << "Serial Time Taken:\t\t" << diff.count() << std::endl;
    std::cout<< testName + "_Serial_Output.txt"<<std::endl;
    return (0);
}
