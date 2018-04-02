//
// Created by xiaolin on 1/27/2017.
//
#include "Upgma.h"

float** Upgma::createMatriz(size_t x, size_t y) {
    float ** m;
    m = new float*[x];
    for (size_t i=0; i<x; ++i) {
        m[i] = new float[y];
        for (size_t j=0; j<y; ++j)
            m[i][j] = 0;
    }
    return m;
}

std::vector<int> Upgma::getMin(std::vector<std::string> header, float** matriz,
                        size_t row, size_t col) {

    int positions[2] = {1, 0};
    this->value = matriz[1][0];
    bool flag = false;
    for (int x = 1; x < row; ++x) {
        for (int y = 0; y < x; ++y) {
            if ( flag && matriz[x][y] < this->value ) {
                positions[0] = x;
                positions[1] = y;
                this->value = matriz[x][y];
            }
            else
                flag = true;
        }
    }
    std::vector<int> pos;
    for (int u=0; u<2; ++u)
        pos.push_back(positions[u]);
    std::cout << "\n" << std::setw(6) << "Min: " << this->value << ", position("<<pos.at(0) <<
              ", " <<pos.at(1)<< "), new: " << header[pos.at(0)] << header[pos.at(1)];
    return pos;
}

void Upgma::getCluster() {
    std::ofstream out("tree.txt");
    std::cout << "\n Original Matriz: \n";
    printMatrizWithHeader(this->matriz, this->header, row, col);
    int clusters = row;
    float ** currentMatriz = this->matriz;
    float ** dynamic_matriz;
    std::string result;
    std::vector<std::string> dynamic_header = this->header;
    while(clusters > 2) {
        std::cout << "\n **************************************** \n";
        std::vector<int> min_position = getMin(
                dynamic_header, currentMatriz, clusters, clusters);
        std::vector<std::string> temporal_header;
        for (int u=0; u<dynamic_header.size(); ++u) {
            if (u != min_position.at(0) && u != min_position.at(1))
                temporal_header.push_back(dynamic_header.at(u));
        }
        temporal_header.push_back(
                dynamic_header.at(
                        min_position.at(0)) + dynamic_header.at(min_position.at(1)));
        result = "("+dynamic_header.at(min_position.at(0))
                 + "," + dynamic_header.at(min_position.at(1))+"):"; //+  std::to_string (this->value);
        out << result << std::endl;
        clusters --;
        dynamic_matriz = createMatriz(clusters, clusters);
        size_t temp_x = 0;
        // set new matriz
        std::cout <<  std::endl << " Clusters: " << clusters;
        for (size_t x=0; x<clusters+1; ++x) {
            size_t temp_y = 0;
            if (x!=min_position.at(0) && x!=min_position.at(1)) {
                for (size_t y=0; y<clusters+1; ++y) {
                    if (y!=min_position.at(0) && y!=min_position.at(1)) {
                        dynamic_matriz[temp_x][temp_y] = currentMatriz[x][y];
                        temp_y++;
                    }
                }
                temp_x++;
            }
        }
        if (clusters == 2) {
            float sum = 0;
            for (size_t u=1; u<3; ++u) {
                sum += currentMatriz[0][u];
            }
            dynamic_matriz[1][0] = sum/2;
            dynamic_matriz[0][1] = dynamic_matriz[1][0];
            std::string temp = "(("+temporal_header.at(0)
                               + "," + temporal_header.at(1)+"):";//+std::to_string(dynamic_matriz[1][0])
            //+ ");";
            out <<  temp;
        }
        else {
            size_t u = 0;
            for (size_t x=0; x<clusters+1; ++x) {
                if (x!=min_position.at(0) && x!=min_position.at(1) ) {
                    float sum = (
                            currentMatriz[min_position.at(0)][x] + currentMatriz[min_position.at(1)][x]);
                    dynamic_matriz[temp_x][u] = sum/2;
                    dynamic_matriz[u][temp_x] = dynamic_matriz[temp_x][u];
                    u++;
                }
            }
        }
        std::cout << "\n New Matriz: \n";
        printMatrizWithHeader(dynamic_matriz, temporal_header, clusters, clusters);
        currentMatriz = dynamic_matriz;
        dynamic_header = temporal_header;
    }
    out.close();
}



void Upgma::buildMatrizFromMap(std::unordered_map<Family*, std::unordered_map<Family*, float>> interMatrix) {

    this->row = interMatrix.size();
    this->col = interMatrix.size();
    this->matriz = createMatriz(row, col);
    std::vector<Family*> fams;
    for (std::unordered_map<Family*, std::unordered_map<Family*, float>>::iterator iter = interMatrix.begin(); iter != interMatrix.end(); iter++) {
        fams.push_back(iter->first);
        std::ostringstream stm;
        stm << iter->first->getID();
        header.push_back(stm.str());
    }

    for (size_t i = 0; i < fams.size(); ++i) {
        Family* rFam = fams[i];
        std::unordered_map<Family*, float> curMap = interMatrix[rFam];
        for (size_t j = 0; j < fams.size(); ++j) {
            Family* cFam = fams[j];
            if(curMap.find(cFam) != curMap.end()){
                this->matriz[i][j] = curMap[cFam];
            } else if(cFam != rFam){
                this->matriz[i][j] = 10.0;
            }
        }
    }
}

void Upgma::printMatriz(float ** matriz, size_t row, size_t col) {
    for (size_t x = 0; x < row; ++x) {
        for (size_t y = 0; y < col; ++y) {
            std::cout << std::setw(7) << *(*(matriz + x) + y);
        }
        std::cout << std::endl;
    }
}

void Upgma::printMatrizWithHeader(float ** matriz, std::vector<std::string> header, size_t row, size_t col) {
    std::cout << "\n" << std::setw(7) << "-";
    for (size_t x = 0; x < header.size(); ++x)
        std::cout << std::setw(7) << header.at(x);
    std::cout << std::endl;
    for (size_t x = 0; x < row; ++x) {
        std::cout << std::setw(7) << header.at(x);
        for (size_t y = 0; y < col; ++y) {
            std::cout << std::setw(7) << *(*(matriz + x) + y);
        }
        std::cout << std::endl;
    }
}

Upgma::~Upgma() {
    for (size_t i = 0; i < this->row; ++i) {
        delete[] this->matriz[i];
    }
    delete[] this->matriz;
}