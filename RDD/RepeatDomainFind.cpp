#include "RepeatDomainFind.h"
#include "sequence.hh"
#include <sstream>
#include <fstream>
#include <queue>
#include <bits/unordered_set.h>
#include "Upgma.h"

bool compareFam(const Family& f1, const Family& f2) {
	return f1.getCount() > f2.getCount();
}

RepeatDomainFind::RepeatDomainFind(){
    // initialization
}

void RepeatDomainFind::splitEleLine(const std::string& src, std::vector<std::string>& dest){
	std::vector<std::string>().swap(dest);  // dest.clear();
	std::string buf;
	std::stringstream ss(src); 
	while (ss >> buf)
		dest.push_back(buf);
}

void RepeatDomainFind::readInFamilies(std::string eleFilePath) {
	std::ifstream fin(eleFilePath.c_str());    
	std::string eleLine("");
	getline(fin,eleLine);

	std::vector<std::string> eleStrs;
	Family curFam;
	int preFamilyId = -1;
	int curFamilyId;
	int startPos;
	int endPos;

	while(getline(fin,eleLine)) {
		splitEleLine(eleLine, eleStrs);
		curFamilyId = atoi(eleStrs[0].c_str());
		startPos = atoi(eleStrs[eleStrs.size() - 2].c_str());
		if (curFamilyId != preFamilyId ) {
			if (preFamilyId != -1)
				families.push_back(curFam);
			curFam = Family();
			curFam.setID(curFamilyId);
			endPos = atoi(eleStrs[eleStrs.size() - 1].c_str());
			curFam.setMinL(endPos - startPos + 1);
			preFamilyId = curFamilyId;
		}
		curFam.increaseCount();
		curFam.addStartPosition(startPos);
	}
	families.push_back(curFam);
	fin.close();

	sort(families.begin(), families.end(), compareFam);
	std::cout<< "families sorted finished, families size: "<< families.size() <<std::endl;

	for (int j=0; j<(int)families.size(); j++) {
		int cnt = families[j].getCount();
		for (int i =0; i< cnt; i++)
			familyMap[families[j].startPositions[i]] = &families[j];
	}

}

void RepeatDomainFind::buildGraph(std::string sequenceFilePath, std::string outFilePath, std::string distanceMatrixFile, std::string graphVizDotFile) {
    std::ofstream outFile(outFilePath);

    Family *curFm;
    std::vector<int> startPs;
    std::map<Family *, int> endFamCache;
    for (size_t famIndex = 0; famIndex < families.size(); famIndex++) {
        curFm = &families[famIndex];
        startPs = curFm->startPositions;
        int bp;
        int rightPos;
        for (size_t i = 0; i < startPs.size(); i++) {
            bp = startPs[i];
            rightPos = bp + curFm->minL - 1;
            for (int tmpFmStartPos = bp + 1; tmpFmStartPos < rightPos; tmpFmStartPos++) {
                if (familyMap.find(tmpFmStartPos) != familyMap.end() && familyMap[tmpFmStartPos] != curFm) {
                    Family *tmpFamily = familyMap[tmpFmStartPos];
                    if (bp + curFm->minL >= tmpFmStartPos + tmpFamily->minL) continue;
                    int interLen = bp + curFm->minL - tmpFmStartPos;
                    if (endFamCache.find(tmpFamily) != endFamCache.end())
                        endFamCache[tmpFamily] += interLen;
                    else {
                        endFamCache[tmpFamily] = interLen;
                        curFm->deNoneInterLen[tmpFamily] = tmpFamily->minL - interLen;
                    }
                }
            }
        }
        // keep families with overlapping ratio > ...
        for (std::map<Family *, int>::iterator iter = endFamCache.begin(); iter != endFamCache.end(); iter++) {
            double ratio = iter->second * 1.0 / (curFm->getTotalLen());
            if (ratio >= 0.15){
                curFm->descendants[iter->first] = ratio;
            } else {
                curFm->deNoneInterLen.erase(iter->first);
            }
        }
        std::map<Family *, int>().swap(endFamCache);
        std::vector<int>().swap(startPs);
    }


    for (size_t famIndex = 0; famIndex < families.size(); famIndex++) {
        try {
            Family *famOrigin = &families[famIndex];
            if (famOrigin->status == 0)  // && !famOrigin->hasParent
                famOrigin->dfsHunt(NULL);
        } catch (std::exception e) {
            std::cout << "famIndex : " << famIndex << std::endl;
            std::cout << e.what() << std::endl;
        }
    }

    for (size_t famIndex = 0; famIndex < families.size(); famIndex++) {
        Family *fam = &families[famIndex];
        fam->status = 0;
    }

    for (size_t famIndex = 0; famIndex < families.size(); famIndex++) {
        Family *fam = &families[famIndex];
        // dealing with multiple parental paths : proceed only when there exists at least 2 paths
        if ((fam->status == 0) && (fam->parents.size() > 1)) {
            fam->dfsHuntRePath(NULL);
        }
    }

    // RDD Process
    for (size_t famIndex = 0; famIndex < families.size(); famIndex++) {
        Family *curNode = &families[famIndex];
        if ((!curNode->hasParent) && (curNode->status == 2 || curNode->status == 0)) {
            int famSize = 0;
            bool hasPath = true;
            while (curNode != NULL) {
                famSize++;
                if (curNode->nextNode != NULL &&
                    (curNode->descendants[curNode->nextNode] == 0 || curNode->descendants[curNode->nextNode] == -1)) {
                    hasPath = false;
                    break;
                }
                curNode = curNode->nextNode;
            }

            if (hasPath && (famSize > 1)) {
                Family *curFam = NULL;
                Family *nxtFam = NULL;
                bool hasDomain = false;
                for (size_t i = 0; i < famSize - 1; i++) {
                    curFam = (i == 0) ? (&families[famIndex]) : nxtFam;
                    nxtFam = curFam->nextNode;
                    if (curFam->descendants[nxtFam] == -2) {
                        if (!hasDomain) {
                            hasDomain = true;
                            if (nxtFam->domain >= 0) {
                                nxtFam->domain++;
                            } else {
                                nxtFam->domain--;
                            }
                        } else {
                            if (nxtFam->domain >= 0) {
                                nxtFam->domain = -(nxtFam->domain + 1);
                            } else {
                                nxtFam->domain--;
                            }
                        }
                    }
                }
            }
        }
    }


    std::queue<Family *> domains;
    for (size_t famIndex = 0; famIndex < families.size(); famIndex++) {
        Family *curNode = &families[famIndex];
        if (curNode->domain > 3) {
            domains.push(curNode);
        }
    }

    std::unordered_map<Family*, std::unordered_map<Family*, float>> interMatrix;
    int globalDomainIdx = 0;
    Sequence sequence(sequenceFilePath);
    sequence.read();

    int globalFamIndex = 0;
    while (!domains.empty()) {
        Family *curNode = domains.front();
        bool  doSys = true;
        float  interLen = 0;
        domains.pop();
        std::vector<Family *> domainFams;
        while (curNode != NULL) {
            if (curNode->domain == 1) {
                break;
            }
            if (doSys){
                domainFams.push_back(curNode);
                if (curNode->domain != 0) {
                    curNode->domain = 1;
                }
                if (curNode->validDescendants > 1) {
                    doSys = false;
                    interLen = curNode->nextNode->minL - curNode->nxtNoneInterLen;
                }
            } else{
                if(curNode->nextNode != NULL) {
                    float tmpLen = curNode->minL + curNode->nxtNoneInterLen - curNode->nextNode->minL;
                    interLen -= tmpLen;
                }
                if (curNode->domain < 0 && curNode->parents.size() > 2) {
                    if((interLen > 0) || (abs(curNode->domain) + abs(curNode->validDescendants) > abs(domainFams.front()->domain) + abs(domainFams.front()->validDescendants))) {
                        domains.push(curNode);
                        domainFams.clear();
                    }
                    break;
                }
            }
            curNode = curNode->nextNode;
        }
        if ((domainFams.size() > 0 && domainFams[0]->parents.size() > 1)) {
            std::cout<<"Domain Idx : "<<++globalDomainIdx<<std::endl;
            Family *startDomainFam = domainFams[0];
            int domainFamsLen = startDomainFam->getPathLen(domainFams[domainFams.size() - 1]->getID());
            std::vector<Family*> sdFams;
            for (size_t i = 0; i < std::min((int)startDomainFam->parents.size(), 5); i++) {
                Family *curFp = startDomainFam->parents[i];
                while(curFp->maxPar != NULL){
                    curFp = curFp->maxPar;
                }
                sdFams.push_back(curFp);
            }
            for (size_t j = 0; j < sdFams.size(); j++) {
                Family *famRow = sdFams[j];
                for (size_t k = 0; k < sdFams.size(); k++) {
                    Family *famCol = sdFams[k];
                    if(famRow == famCol) continue;
                    std::cout<<"Fam_1 ID : "<<famRow->getID()<<"    Fam_2 ID : "<<famCol->getID()<<std::endl;
                    interMatrix[famRow][famCol] += domainFamsLen;
                }
            }
            std::vector<Family *>().swap(sdFams);
        }
        std::vector<Family *>().swap(domainFams);
    }

    std::cout<<"Inter Matrix Len :  "<<interMatrix.size()<<std::endl;

    std::unordered_map<Family*, std::unordered_set<Family*>> gravizMap;

    for (std::unordered_map<Family*, std::unordered_map<Family*, float>>::iterator rIter = interMatrix.begin(); rIter != interMatrix.end(); rIter++) {
        Family *curFp = rIter->first;
        bool isStaFam = true;
        int preNoneInterLen = 0, famStartPos = 0, famLen = 0;
        std::vector<std::string> elements;
        std::unordered_map<char, int> cntMap;
        globalFamIndex++;
        outFile << ">" << globalFamIndex << std::endl;
        while (curFp != NULL) {
            if (isStaFam) {
                famStartPos = 0;
                famLen = curFp->minL;
                isStaFam = false;
            } else {
                famStartPos = curFp->minL - preNoneInterLen;
                famLen = preNoneInterLen;
            }
            for (int q = 0; q < curFp->cnt; q++) {
                elements.push_back(sequence.subsequence(curFp->startPositions[q] + famStartPos, famLen));
            }
            for (int n = 0; n < famLen; n++) {
                cntMap['A'] = 0;
                cntMap['T'] = 0;
                cntMap['C'] = 0;
                cntMap['G'] = 0;
                for (int m = 0; m < curFp->cnt; m++) cntMap[toupper(elements[m][n])]++;
                char maxBase = 'N';
                int maxCnt = 0;
                for (std::unordered_map<char, int>::iterator iter = cntMap.begin(); iter != cntMap.end(); iter++) {
                    if (iter->second > maxCnt) {
                        maxBase = iter->first;
                        maxCnt = iter->second;
                    }
                }
                outFile << maxBase;
            }
            preNoneInterLen = curFp->nxtNoneInterLen;
            std::vector<std::string>().swap(elements);

            if (curFp->nextNode != NULL) {
                gravizMap[curFp].insert(curFp->nextNode);
            }

            curFp = curFp->nextNode;
        }
        outFile << std::endl;
    }

    for (std::unordered_map<Family*, std::unordered_map<Family*, float>>::iterator rIter = interMatrix.begin(); rIter != interMatrix.end(); rIter++) {
        Family* rFam = rIter->first;
        std::unordered_map<Family*, float> curMap = interMatrix[rFam];
        for(std::unordered_map<Family*, float>::iterator cIter = curMap.begin(); cIter != curMap.end(); cIter++) {
            if(cIter->second < 1.0) continue;
            Family* cFam = cIter->first;
            float similarity = (1.0 * cIter->second / rFam->getPathLen() + 1.0 * cIter->second / cFam->getPathLen()) / 2;
            if(similarity >= 1.0) std::cout<<"Unexpected Similarity!"<<std::endl;
            interMatrix[rFam][cFam] = 1.0 - similarity;
            interMatrix[cFam][rFam] = interMatrix[rFam][cFam];
            std::cout<<interMatrix[rFam][cFam]<<std::endl<<interMatrix[cFam][rFam]<<std::endl;
        }
    }

    std::ofstream outFile2(distanceMatrixFile);
    std::vector<Family*> allFams;
    for (std::unordered_map<Family*, std::unordered_map<Family*, float>>::iterator iter = interMatrix.begin(); iter != interMatrix.end(); iter++) {
        allFams.push_back(iter->first);
    }

    outFile2<<allFams.size()<<std::endl;
    for (size_t i = 0; i < allFams.size(); ++i) {
        Family* rFam = allFams[i];
        outFile2<< rFam->getID();
        std::unordered_map<Family*, float> curMap = interMatrix[rFam];
        for (size_t j = 0; j < allFams.size(); ++j) {
            Family* cFam = allFams[j];
            if(curMap.find(cFam) != curMap.end()){
                outFile2<<" "<<curMap[cFam];
            } else if(cFam != rFam){
                outFile2<<" 10.0";
            } else{
                outFile2<<" 0.0";
            }
        }
        outFile2<< std::endl;
    }

    std::ofstream outFile3(graphVizDotFile);
    outFile3<<"digraph G {"<<std::endl;
    for (std::unordered_map<Family*, std::unordered_set<Family*>>::iterator iter = gravizMap.begin(); iter != gravizMap.end(); iter++) {
        outFile3<<iter->first->id<<";"<<std::endl;
    }
    for (std::unordered_map<Family*, std::unordered_set<Family*>>::iterator iter = gravizMap.begin(); iter != gravizMap.end(); iter++) {
        Family* curFam = iter->first;
        std::unordered_set<Family*> curSet = iter->second;
        for (std::unordered_set<Family*>::iterator itr = curSet.begin(); itr != curSet.end(); ++itr) {
            outFile3<<curFam->id<<"->"<<(*itr)->id<<";"<<std::endl;
        }
    }
    outFile3<<"}";

}
