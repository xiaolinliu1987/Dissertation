//
// Created by xiaolin on 1/27/2017.
//

#ifndef COMHUNTER_UPGMA_H
#define COMHUNTER_UPGMA_H

#include <cstdlib>
#include <iostream>
#include <ctime>
#include <iomanip>
#include <vector>
#include <fstream>
#include <sstream>
#include <cfloat>
#include "Family.h"
#include <string>


class Upgma  {
public:
    std::size_t row, col;
    float ** matriz;
    float value;
    std::vector<std::string> header;
    std::string file;
    float** createMatriz(size_t x, size_t y);
    std::vector<int> getMin(std::vector<std::string> header, float** matriz, size_t row, size_t col);
    void getCluster();
    void buildMatrizFromMap(std::unordered_map<Family*, std::unordered_map<Family*, float>> interMatrix);
    void printMatriz(float ** matriz, size_t row, size_t col);
    void printMatrizWithHeader(float ** matriz, std::vector<std::string> header, size_t row, size_t col);
    ~Upgma();
};


#endif //COMHUNTER_UPGMA_H
