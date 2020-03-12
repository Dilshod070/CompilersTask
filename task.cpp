#define export exports
extern "C" {
	#include "qbe/all.h"
}

#include <list>

struct answerElem {
	char *_headBlk;
	std::list<Blk *> _blks;
	std::list<std::pair<char *, char *> > _edges;
};


void getLastElem(Blk *headBlk, Blk *curBlk, answerElem *e) {
	if (curBlk == headBlk) {
		return;
	}
	if (curBlk->visit == 1) {
		return getLastElem(headBlk, curBlk->pred[0], e);
	}
	char *tmpCurName = new char[NString];
	memcpy(tmpCurName, curBlk->name, strlen(curBlk->name));
	e->_blks.push_front(curBlk);
	for (int n = 0; n < curBlk->npred; ++n) {
		Blk *predBlk = curBlk->pred[n];
		bool blkAlreadyBeen = false;
		for (int k = n - 1; k >= 0; --k) {
			if (curBlk->pred[n] == curBlk->pred[k]) {
				blkAlreadyBeen = true;			
			}
		}
		if (predBlk->id >= curBlk->id or blkAlreadyBeen) {
			continue;
		}
		char *tmpPredName = new char[NString];
		memcpy(tmpPredName, predBlk->name, strlen(predBlk->name));
		e->_edges.push_front(std::pair<char*, char*> (tmpPredName, tmpCurName));
		getLastElem(headBlk, predBlk, e);
	}
	curBlk->visit = 1;
	return;
}

static void readFn(Fn *fn) {

	fillrpo(fn); // пронумеровываем блоки    
	
// ищем обратные ребра	
	std::list<std::pair<Blk*, Blk*> > backEdges;
	for (Blk *curBlk = fn->start; curBlk; curBlk = curBlk->link) {
		for (int n = curBlk->npred - 1; n >= 0; --n) {
			Blk *predBlk = curBlk->pred[n];
			if (predBlk->id >= curBlk->id) {
				backEdges.push_front(std::pair<Blk*, Blk*>(predBlk, curBlk));
			}
		}	
	}

/*
// вывод обратных ребер
	printf("back edges:\n");
	for (std::list<std::pair<Blk*, Blk*> >::iterator it = backEdges.begin(); it != backEdges.end(); ++it) {
		printf("  @%s -> @%s\n", it->first->name, it->second->name);
	}
*/

// добавляем облать-лист для каждего блока
	std::list<Blk*> endBlks; // нужны указатели на поседние блоки при поиске последней области
	std::list<answerElem *> answer;
	for (Blk *blk = fn->start; blk; blk = blk->link) {
		answerElem *e = new struct answerElem;
		e->_headBlk = new char[NString];
		memcpy(e->_headBlk, blk->name, strlen(blk->name));
		answer.push_back(e);
		strcat(blk->name, "__r");
		if (blk->s1 == NULL and blk->s2 == NULL)
			endBlks.push_front(blk);
		blk->visit = 0;
	}

	for (std::list<std::pair<Blk*, Blk*> >::iterator it = backEdges.begin(); it != backEdges.end(); ++it) {
		Blk *from = it->first; // первый элемент обратного ребра
		Blk *to = it->second;
		if (from != to) {
// добавляем область-тело для каждого обратного ребра 
// p.s. когда from == to область-тело совпадает с областью-листом
			answerElem *e = new struct answerElem;
			e->_headBlk = new char[NString];
			memcpy(e->_headBlk, to->name, strlen(to->name));
			getLastElem(to, from, e);
			answer.push_back(e);	
			strcat(to->name, "__r");
			
			// в массиве ссылок на предыдущие блоки
			// все ссылки на блоки, которые входят в область-тело,
			// заменяем на ссылку на заголовок этой области-тела
			for (std::list<Blk *>::iterator it = e->_blks.begin(); it != e->_blks.end(); ++it) {
				if ((*it)->s1) {
					for (int n = 0; n < (*it)->s1->npred; ++n) {
						if ((*it)->s1->pred[n] == (*it)) {
							(*it)->s1->pred[n] = to;
						}
					}
				}
				if ((*it)->s2) {
					for (int n = 0; n < (*it)->s2->npred; ++n) {
						if ((*it)->s2->pred[n] == (*it)) {
							(*it)->s2->pred[n] = to;
						}
					}
				}
			}
			
		}
// добавляем область-цикл для каждого обратного ребра 
		answerElem *e = new struct answerElem;
		e->_headBlk = new char[NString];
		memcpy(e->_headBlk, to->name, strlen(to->name));
		char *tmpstr = new char[NString];
		memcpy(tmpstr, to->name, strlen(to->name));
		e->_edges.push_front(std::pair<char*, char*> (tmpstr, tmpstr));
		answer.push_back(e);
		strcat(to->name, "__r");
	}

// собираем последнюю область
	answerElem *e = new struct answerElem;
	for (std::list<Blk*>::iterator it = endBlks.begin(); it != endBlks.end(); ++it) {
		answerElem *tmpE = new struct answerElem;
		getLastElem(fn->start, (*it), tmpE);
		e->_blks.splice(e->_blks.end(), tmpE->_blks);
		e->_edges.splice(e->_edges.end(), tmpE->_edges);
	}
	e->_headBlk = new char[NString];
	memcpy(e->_headBlk, fn->start->name, strlen(fn->start->name));
	answer.push_back(e);

// вывод ответа
	for (std::list<answerElem*>::iterator it = answer.begin(); it != answer.end(); ++it) {
		printf("@%s", (*it)->_headBlk);
		printf(" : ");
		for (std::list<Blk*>::iterator blkIt = (*it)->_blks.begin(); blkIt != (*it)->_blks.end(); ++blkIt) {
			printf("@%s ", (*blkIt)->name);
		}
		printf(":");
		for (std::list<std::pair<char*, char*> >::iterator edgeIt = (*it)->_edges.begin(); edgeIt != (*it)->_edges.end(); ++edgeIt) {
			printf(" @%s-@%s", edgeIt->first, edgeIt->second);
		}
		printf("\n");
	}

}

static void readDat(Dat *dat) {
}


int main(int argc, char *argv[]){
/*	if (argc < 2) {
		printf("err: add file name\n");
		return 1;
	}
	FILE *fp = fopen(argv[1], "r");
	if (!fp) {
		printf("err: cannot open file\n");
		return 1;
	}
	
	parse(fp, argv[1], readDat, readFn);
	
	fclose(fp);*/
	parse(stdin, "<stdin>", readDat, readFn);
	freeall();
}
