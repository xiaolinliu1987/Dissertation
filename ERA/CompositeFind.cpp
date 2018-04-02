#include "CompositeFind.h"
#include "sequence.hh"
#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>


bool compareFam(const Family& f1, const Family& f2) {
	return f1.getCount() > f2.getCount();
}

CompositeFind::CompositeFind(){
    //initialization
}

void CompositeFind::splitEleLine(const std::string& src, std::vector<std::string>& dest){
	std::vector<std::string>().swap(dest);  // dest.clear();
	std::string buf;
	std::stringstream ss(src); 
	while (ss >> buf)
		dest.push_back(buf);
}

void CompositeFind::readInFamilies(std::string eleFilePath) {
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

void CompositeFind::buildGraph(std::string sequenceFilePath, std::string outFilePath) {
	std::ofstream outFile(outFilePath);

	Family* curFm;
	std::vector<int> startPs;
	std::map<Family*, int> endFamCache;
	for (size_t famIndex = 0; famIndex < families.size(); famIndex++) {
		curFm = &families[famIndex];
		startPs = curFm->startPositions;
		int bp;
		int rightPos;
		for (size_t i=0; i < startPs.size(); i++) {
			bp = startPs[i];
			rightPos = bp + curFm->minL - 1;
			for (int tmpFmStartPos=bp+1; tmpFmStartPos<rightPos; tmpFmStartPos++) {
				if (familyMap.find(tmpFmStartPos) != familyMap.end() && familyMap[tmpFmStartPos] != curFm) {
					Family* tmpFamily = familyMap[tmpFmStartPos];
					if (bp + curFm->minL >= tmpFmStartPos + tmpFamily->minL) continue;
					int interLen =  bp + curFm->minL- tmpFmStartPos ;
					if ( endFamCache.find(tmpFamily) != endFamCache.end() ) 
						endFamCache[tmpFamily] += interLen;
					else {
						endFamCache[tmpFamily] = interLen; 
						curFm->deNoneInterLen[tmpFamily] = tmpFamily->minL - interLen;
					}
				} 
			}
		}

		for (std::map<Family*, int>::iterator iter = endFamCache.begin(); iter != endFamCache.end(); iter++) {
			double ratio = iter->second * 1.0 / (curFm->getTotalLen());
			if (ratio >= 0.2)
				curFm->descendants[iter->first] = ratio;
		}
		std::map<Family*, int>().swap(endFamCache);
		std::vector<int>().swap(startPs);
	}

	for (size_t famIndex = 0; famIndex < families.size(); famIndex++) { 
		try {
			Family *famOrigin = &families[famIndex];
			if (famOrigin->status == 0)
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

	Sequence sequence(sequenceFilePath);
	sequence.read();
	int globalFamIndex = 0;
	for (size_t famIndex = 0; famIndex < families.size(); famIndex++) {
		Family* curNode = &families[famIndex];
		if ( (!curNode->hasParent) && (curNode->status == 2  || curNode->status == 0) ) {
			std::vector<Family*> compositeFams;
			bool hasPath = true;
			while (curNode != NULL) {
				compositeFams.push_back(curNode);
				if (curNode -> nextNode != NULL) {
					if (curNode -> descendants[curNode -> nextNode] == 0 || curNode -> descendants[curNode -> nextNode] == -1) {
						hasPath = false;
						break;
					}  else if (curNode -> descendants[curNode -> nextNode] == -2) {
						break;
					}
				} 
				curNode = curNode -> nextNode;
			}

			if (hasPath && (compositeFams.size() > 10)){
				int preNoneInterLen = 0, famStartPos = 0, famLen = 0;
				std::vector<std::string> elements;
				std::unordered_map<char, int> cntMap;
				globalFamIndex++;
				outFile << ">" << globalFamIndex <<  std::endl;
				for (size_t i = 0; i < compositeFams.size(); i++) {
					Family* fam = compositeFams[i];
					if (i == 0) {
						famStartPos = 0;
						famLen = fam->minL;
					} else {
						famStartPos = fam->minL - preNoneInterLen;
						famLen = preNoneInterLen;
					}
					for (int j = 0; j < fam->cnt; j++) {
						elements.push_back(sequence.subsequence(fam->startPositions[j] + famStartPos, famLen));
					}
					for (int n = 0; n < famLen; n++){
						cntMap['A'] = 0; cntMap['T'] = 0; cntMap['C'] = 0; cntMap['G'] = 0; 
						for (int m = 0; m < fam->cnt; m++)  cntMap[toupper(elements[m][n])]++;
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
					preNoneInterLen = fam->nxtNoneInterLen;
					std::vector<std::string>().swap(elements);
				}
				outFile << std::endl;
			} 
			std::vector<Family*>().swap(compositeFams);
		}
	}
	std::cout << "CompositesHunter finished" << std::endl;
}