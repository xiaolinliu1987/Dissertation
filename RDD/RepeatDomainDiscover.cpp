#include "RepeatDomainFind.h"

int main(int argc, char** argv)
{
	RepeatDomainFind repeatDomainFind = RepeatDomainFind();
	repeatDomainFind.readInFamilies(argv[1]);
	repeatDomainFind.buildGraph(argv[2], argv[3], argv[4], argv[5]);
	return 0;
}

