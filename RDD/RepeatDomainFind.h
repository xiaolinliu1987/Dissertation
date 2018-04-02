#ifndef LIU_COMPOSITE_FIND
#define LIU_COMPOSITE_FIND

#include <string>
#include <vector>
#include <map>
#include <stdexcept>
#include "Family.h"
#include "Composite.h"
#include "FamilyPath.h"


class RepeatDomainFind
{
public:
	RepeatDomainFind();
	void splitEleLine(const std::string& src, std::vector<std::string>& dest);
	void readInFamilies(std::string eleFilePath);
	void buildGraph(std::string sequenceFilePath, std::string outFilePath, std::string distanceMatrixFile, std::string graphVizDotFile);

private:
	std::vector<Family> families;
	std::map<int, Family*> familyMap;
};

#endif
