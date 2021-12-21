#include <iostream>
#include <mpi.h>
#include <string>
#include <fstream>
#include <vector>
#include <thread>
#include <cmath>


void WriteArray(std::vector<int> Data, int gen, std::ofstream &output) {
    output << "Generation " << gen + 1 << ":" << std::endl;
    int idx = 0;
    int N = (int) sqrt((int) Data.size());
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N && idx < N * N; ++j) {
            char q = '.';
            if (Data[idx++] == 1)
                q = '*';
            output << q << '\t';
        }
        output << std::endl;
    }
    output << std::endl << std::endl;
}

void ToFile(std::vector<std::vector<int>> Generations, const std::string &fileName) {
    std::ofstream output(fileName);
    for (int i = 0; i < Generations.size(); i++) {
        WriteArray(Generations[i], i, output);
    }
    output.close();
}


int main(int argc, char *argv[]) {

    double myTime;

    int size, rank;
    int N = -1, generations = -1, sliceSize;
    std::vector<int> grid;
    int info[3];
    MPI_Init(&argc, &argv);

    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);


    std::string fullname = argv[1];
    size_t lastindex = fullname.find_last_of('.');
    std::string testName = fullname.substr(0, lastindex);
    testName = testName.substr(testName.find_last_of("/\\") + 1);
    std::vector<std::vector<int>> Storage;

    int next, prev;
    //set up next and prev ranks
    if (rank == 0) {
        prev = size - 1;
    } else {
        prev = rank - 1;
    }
    if (rank == size - 1) {
        next = 0;
    } else {
        next = rank + 1;
    }


    if (rank == 0) {
        //Does initial setup only on first rank
        if (argc < 2) {
            std::cout << "No Input File Specified" << std::endl;
            MPI_Abort(MPI_COMM_WORLD, 1);
            MPI_Finalize();
            return -1;
        }

        //open file to read from
        std::fstream file;
        file.open(argv[1]);
        if (!file) {
            std::cout << "Error Opening File" << std::endl;
            MPI_Abort(MPI_COMM_WORLD, 1);
            MPI_Finalize();
            return -1;
        }


        file >> N >> generations;
        if (file.peek() == std::ifstream::traits_type::eof() || N == -1 || generations == -1) {
            std::cout << "Please Check your input file" << std::endl;
            MPI_Abort(MPI_COMM_WORLD, 1);
            MPI_Finalize();
            return -1;
        }

        if (N < 2 || generations < 1) {
            std::cout << "Board Size cannot Be Less Than 2\nNumber of Generations cannot be less than 1"
                      << std::endl;
            MPI_Abort(MPI_COMM_WORLD, 1);
            MPI_Finalize();
            return -1;
        }


        if (N % size != 0) {
            std::cout << "Size of Board Must Be Divisible By Number of Processors " << std::endl;
            MPI_Abort(MPI_COMM_WORLD, 1);
            MPI_Finalize();
            return -1;
        }


        sliceSize = N / size;
        int gridSize = N * N;

        for (int i = 0; i < gridSize; ++i) {
            char temp;
            file >> temp;
            grid.push_back(temp - '0');
        }

        info[0] = N;
        info[1] = sliceSize;
        info[2] = generations;

    }

    MPI_Barrier(MPI_COMM_WORLD);
    myTime = MPI_Wtime();


    MPI_Bcast(info, 3, MPI_INT, 0, MPI_COMM_WORLD);
    N = info[0];
    sliceSize = info[1];
    generations = info[2];
    int mySlice[sliceSize][N];

    MPI_Scatter(grid.data(), sliceSize * N, MPI_INT, &(mySlice[0][0]), sliceSize * N, MPI_INT, 0, MPI_COMM_WORLD);


    std::vector<int> ROW_Offset = {-1, 0, 1, 0, -1, 1, 1, -1};
    std::vector<int> COL_Offset = {0, 1, 0, -1, 1, 1, -1, -1};
    int DEAD = 0, ALIVE = 1, currentGen = 0;
    int fromDown[N];
    int fromUp[N];
    int futureSlice[sliceSize][N];


    while (currentGen < generations) {
        //simultaneously send and receive data for the bottom and top rows.
        MPI_Sendrecv(&mySlice[sliceSize - 1][0], N, MPI_INT, next, 1, fromDown, N, MPI_INT, prev, 1, MPI_COMM_WORLD,
                     MPI_STATUS_IGNORE);
        MPI_Sendrecv(&mySlice[0][0], N, MPI_INT, prev, 1, fromUp, N, MPI_INT, next, 1, MPI_COMM_WORLD,
                     MPI_STATUS_IGNORE);


        //Game Loop
        for (int i = 0; i < sliceSize; i++) {
            for (int j = 0; j < N; j++) {
                int neighbourAliveCount = 0;
                //Check Neighbours;
                for (int k = 0; k < ROW_Offset.size(); ++k) {
                    int newRow = i + ROW_Offset[k];
                    int newCol = j + COL_Offset[k];
                    int recv;
                    if (newCol == -1)
                        newCol = N - 1;
                    else if (newCol == N)
                        newCol = 0;

                    if (newRow == -1) {
                        recv = fromDown[newCol];
                        neighbourAliveCount += recv;
                    } else if (newRow == sliceSize) {
                        recv = fromUp[newCol];
                        neighbourAliveCount += recv;
                    } else {
                        neighbourAliveCount += mySlice[newRow][newCol];
                    }
                }


                //Game Conditions
                if (mySlice[i][j] == ALIVE) {
                    if ((neighbourAliveCount < 2 || neighbourAliveCount > 3)) {
                        futureSlice[i][j] = DEAD;
                    } else if ((neighbourAliveCount == 2 || neighbourAliveCount == 3)) {
                        futureSlice[i][j] = ALIVE;
                    } else {
                        futureSlice[i][j] = mySlice[i][j];
                    }
                } else {
                    if (neighbourAliveCount == 3)
                        futureSlice[i][j] = ALIVE;
                    else
                        futureSlice[i][j] = mySlice[i][j];
                }
            }
        }


        //Copy new board to old board
        for (int i = 0; i < sliceSize; ++i) {
            for (int j = 0; j < N; ++j) {
                mySlice[i][j] = futureSlice[i][j];
            }
        }


        MPI_Gather(&(mySlice[0][0]), sliceSize * N, MPI_INT, grid.data(), sliceSize * N, MPI_INT, 0, MPI_COMM_WORLD);

        if (rank == 0) {
            Storage.push_back(grid);
        }


        //increment generation counter
        currentGen++;
    }

    myTime = MPI_Wtime() - myTime;
    double maxTime;
    MPI_Reduce(&myTime, &maxTime, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        ToFile(Storage, testName + "_Parallel_Output.txt");
        std::ofstream results("Results.txt", std::ios_base::app);
        results << "Parallel Time Taken:\t" << maxTime << std::endl;
        std::cout<<testName + "_Parallel_Output.txt"<<std::endl;
    }
    MPI_Finalize();
    return 0;

}
