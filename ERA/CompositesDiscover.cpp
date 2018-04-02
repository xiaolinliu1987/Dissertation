#include "CompositeFind.h"

int main(int argc, char** argv)
{
	CompositeFind compositeFind = CompositeFind();
	compositeFind.readInFamilies(argv[1]);
	compositeFind.buildGraph(argv[2], argv[3]);
	return 0;
}

