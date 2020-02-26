#define export exports
extern "C" {
	#include "qbe/all.h"
}

static void readFn(Fn *fn) {
	// можно все стереть
	// просто пример работы со структурами

	fillrpo(fn); // пронумеровываем блоки    
	for (int n = 0; n < fn->nblk; ++n) {
		Blk *b = fn->rpo[n];
		printf("block name = @%s, id = %i\n", b->name, b->id); // проверка нумерации 
	}

	// fillloop(fn) не будет работать без fillrpo(fn)
	// простовляем уровень (1, 10, 100 и тд) вложенности циклов (вроде бы)
//	fillloop(fn);

	// filldom(fn) не будет работать без fillrpo(fn)
	// заполняет информацию о доминаторах
//	filldom(fn);
	
	for (Blk *blk = fn->start; blk; blk = blk->link) {
		printf("\n---- block name: @%s\n", blk->name);
		printf("\nnext blocks:\n");
		if (blk->s1) {
			printf("  @%s\n", blk->s1->name); // s1
		}
		if (blk->s2) {
			printf("  @%s\n", blk->s2->name); // s2
		}
		printf("previous blocks:\n");
		for (int n = 0; n < blk->npred; ++n) {
			Blk *pblk = blk->pred[n];
			printf("  @%s\n", pblk->name);
		}

		// не будет заполнено без filldom(fn) 
		printf("\n");
		if (blk->idom) // непосредственный доминатор
			printf("idom->name: @%s\n", blk->idom->name);
		if (blk->dom) // вроде бы как имя блока, над которым доминирует
			printf("dom->name: @%s\n", blk->dom->name);
		if (blk->dlink) // вообще не поняла, что это
			printf("dlink->name: @%s\n", blk->dlink->name);
		
		// не будет заполнено без fillloop(fn)
		if (blk->loop)
			printf("\nloop = %d\n", blk->loop);
	}
}

static void readDat(Dat *dat) {
}

int main(int argc, char *argv[]){
	if (argc < 2) {
		printf("err: add file name\n");
		return 1;
	}
	FILE *fp = fopen(argv[1], "r");
	if (!fp) {
		printf("err: cannot open file\n");
		return 1;
	}

	parse(fp, argv[1], readDat, readFn);
	
	fclose(fp);
	freeall();
}
