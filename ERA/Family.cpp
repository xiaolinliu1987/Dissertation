#include "Family.h"

Family::Family() {
	id = -1;
	cnt = 0;
	minL = 0;
	maxWgt = 0;
	reWgt = 0;
	status = 0;
	hasParent = false;
	nextNode = NULL;
	nxtNoneInterLen = 0;
}

Family::Family(Family* otherFam) {
	id = otherFam -> id;
	cnt = otherFam -> cnt;
	minL = otherFam -> minL;
	maxWgt =otherFam -> maxWgt;
	status = 0;
	std::copy(otherFam->startPositions.begin(), otherFam->startPositions.end(), std::back_inserter(startPositions));
	nextNode = NULL;
	nxtNoneInterLen = 0;
	hasParent = otherFam->hasParent;
}

void Family::increaseCount(){
	cnt++;
}

int Family::getCount() const{
	return cnt;
}

int Family::getTotalLen() const{
	return cnt * minL;
}

void Family::setMinL(int L) {
	minL = L;
}

void Family::addStartPosition(int startPos){
	startPositions.push_back(startPos);
}

void Family::addPosition(int startPos, int endPos){
	startPositions.push_back(startPos);
	endPositions.push_back(endPos);
}

void Family::setID(int ID){
	this->id = ID;
}

int Family::getID() const{
	return id;
}


void Family::dfsHunt(Family* preNode) {
	// if preNode->descendants[this] == - 1 
	if (status == 1) {
		preNode->descendants[this] = -1;
		return;
	} else if (status == 2) return;

	if (descendants.size() == 0) {
		status = 2;
		return;
	}
	status = 1;
	for (std::unordered_map<Family*, float>::iterator iter = descendants.begin(); iter != descendants.end(); iter++) {
		Family *child = iter->first;
		if(isAdjacent(preNode, this, child)){
			child->dfsHunt(this); // this points to preNode
			if (this->descendants[child] == -1) continue; // to determine if loop exists !
			float wgt = child -> maxWgt;
			if (wgt + descendants[child] > maxWgt) {
				maxWgt = wgt + descendants[child];
				nextNode = child;
				nxtNoneInterLen = deNoneInterLen[child];
			}
		}
	}
	
	if (nextNode != NULL){
		nextNode->hasParent = true;
		nextNode->parents.push_back(this);
	}
	status = 2;
}

void Family::dfsHuntRePath(Family* nxtNode){
	if (status == 1) {
		nxtNode->descendants[this] = -1;
		return;
	} else if (status == 2) return;
	
	if (parents.empty()) {
		status = 2;
		return;
	}
	status = 1;
	Family *maxPar = NULL;
	for (size_t famIndex = 0; famIndex < parents.size(); famIndex++) {
		Family *par = parents[famIndex];
		if (par->descendants[this] == -1) continue; // loop detected
		par->dfsHuntRePath(this);
		int wgt = par->reWgt + 1;
		if (wgt > reWgt) {
			reWgt = wgt;
			maxPar = par;
		}
	}
	
	for (size_t famIndex = 0; famIndex < parents.size(); famIndex++) {
		Family *par = parents[famIndex];
		if(par == maxPar) continue;
        // all the other fams (not maxPar)
		if (par->descendants[this] > 0) {
			int tmpWgt = par->reWgt + 1;
			if (1.0 * reWgt / tmpWgt > 1.33333) par->descendants[this] = 0;
			else par->descendants[this] = -2;
		}
	}
	status = 2;
}

void Family::clear() {
	maxWgt = 0.0;
}

bool Family::operator<(const Family& otherFam) const {
	return this->id < otherFam.id;
}