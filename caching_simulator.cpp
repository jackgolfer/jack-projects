#include <iostream>
#include <fstream>
#include <cstdio>
#include <cmath>
#include <string>
#include <iomanip>
#include <list>

using namespace std;

int main()
{
	int DTLB_numSets, DTLB_setSize, PT_numVirtual, PT_numPhysical, PT_pageSize, DC_numSets, DC_setSize, DC_lineSize, L2_numSets, L2_setSize, L2_lineSize;
	int DTLBIndexBits, PTIndexBits, PTOffsetBits, DCIndexBits, DCOffsetBits, L2IndexBits, L2OffsetBits,tempAdress;
	int DTLBIndexBitsAnd, PTIndexBitsAnd, PTOffsetBitsAnd, DCIndexBitsAnd, DCOffsetBitsAnd, L2IndexBitsAnd, L2OffsetBitsAnd;
	char DC_write, virtualAdd, TLB, L2Cache, readWrite = 0, nullChar;
	bool foundInvalid, foundInvalid2;
	int dtlbHits, dtlbMisses, ptHits, ptFaults, dcHits, dcMisses, L2Hits, L2Misses, TotalReads, TotalWrites, memRefs, ptRefs, diskRefs;
	double dtlbHitRatio, ptHitRatio, dcHitRatio, L2HitRatio, readRatio;
	
	struct DCEntry {
		bool validBit, dirty;
		int tag;
	};
	struct PTEntry {
		bool valid, dirty;
		int virtualPage, physicalPage;		
	};
	struct TLBEntry {
		int physicalPage, tag;
		bool valid;
	};

	dtlbHits = dtlbMisses = ptHits = ptFaults = dcHits = dcMisses = L2Hits = L2Misses = TotalReads = TotalWrites = memRefs = ptRefs = diskRefs = 0;
	
	
	FILE * config;
	config = fopen("trace.config","r");
	
	fscanf(config, "Data TLB configuration\nNumber of sets: %i\nSet size: %i\n\n", &DTLB_numSets, &DTLB_setSize);
	if(DTLB_numSets > 256 || DTLB_numSets < 1)
		cout << "hierarchy: error - number of dtlb sets requested exceeds MAXTLBSETS\n";
	if(ceil(log2(DTLB_numSets)) != floor(log2(DTLB_numSets)))
		cout << "hierarchy: number of dtlb sets is not a power of two\n";
	if(DTLB_setSize > 8 || DTLB_setSize < 1)
		cout << "hierarchy: error - dtlb set size requested exceeds MAXSETSIZE\n";
	if(ceil(log2(DTLB_setSize)) != floor(log2(DTLB_setSize)))
		cout << "hierarchy: error - number of virtual pages requested exceeds MAXVIRTPAGES\n";
	
	fscanf(config, "Page Table configuration\nNumber of virtual pages: %i\nNumber of physical pages: %i\nPage size: %i\n\n", &PT_numVirtual, &PT_numPhysical, &PT_pageSize);
	if(PT_numVirtual > 8192 || PT_numVirtual < 1)
		cout << "hierarchy: error - number of virtual pages requested exceeds MAXVIRTPAGES\n";
	if(ceil(log2(PT_numVirtual)) != floor(log2(PT_numVirtual)))
		cout << "hierarchy: number of virtual pages is not a power of two\n";
	if(PT_numPhysical > 1024 || PT_numPhysical < 1)
		cout << "hierarchy: error - number of physical pages requested exceeds MAXPHYPAGES\n";
	if(ceil(log2(PT_numPhysical)) != floor(log2(PT_numPhysical)))
		cout << "hierarchy: number of physical pages is not a power of two\n";
	if(ceil(log2(PT_pageSize)) != floor(log2(PT_pageSize)))
		cout << "hierarchy: number of physical pages is not a power of two\n";
	
	fscanf(config, "Data Cache configuration\nNumber of sets: %i\nSet size: %i\nLine size: %i\nWrite through/no write allocate: %c\n\n", &DC_numSets, &DC_setSize, &DC_lineSize, &DC_write);
	
	fscanf(config, "L2 Cache configuration\nNumber of sets: %i\nSet size: %i\nLine size: %i\n\n", &L2_numSets, &L2_setSize, &L2_lineSize);
	fscanf(config, "Virtual addresses: %c\nTLB: %c\nL2 cache: %c", &virtualAdd, &TLB, &L2Cache);	
	fclose(config);
	
	
	if(TLB == 'y' && virtualAdd == 'n')
		cout << "hierarchy: TLB cannot be enabled when virtual addresses are disabled\n";
	
	
	
	
	
	
	DTLBIndexBits = ceil(log2(DTLB_numSets));
	PTIndexBits = ceil(log2(PT_numVirtual));
	PTOffsetBits = ceil(log2(PT_pageSize));
	DCIndexBits = ceil(log2(DC_numSets));
	DCOffsetBits = ceil(log2(DC_lineSize));
	L2IndexBits = ceil(log2(L2_numSets));
	L2OffsetBits = ceil(log2(L2_lineSize));
	
	printf("Data TLB contains %i sets.\nEach set contains %i entries.\nNumber of bits used for the index is %i.\n\n", DTLB_numSets, DTLB_setSize, DTLBIndexBits);
	printf("Number of virtual pages is %i.\nNumber of physical pages is %i.\nEach page contains %i bytes.\nNumber of bits used for the page table index is %i.\nNumber of bits used for the page offset is %i.\n\n", PT_numVirtual, PT_numPhysical, PT_pageSize, PTIndexBits, PTOffsetBits);
	printf("D-cache contains %i sets.\nEach set contains %i entries.\nEach line is %i bytes.\n", DC_numSets, DC_setSize, DC_lineSize);
	if(DC_write == 'y')
		cout << "The cache uses a no write-allocate and write-through policy.\n";
	else
		cout << "The cache uses a write-allocate and write-back policy.\n";
	printf("Number of bits used for the index is %i.\nNumber of bits used for the offset is %i.\n\n", DCIndexBits, DCOffsetBits);
	printf("L2-cache contains %i sets.\nEach set contains %i entries.\nEach line is %i bytes.\nNumber of bits used for the index is %i.\nNumber of bits used for the offset is %i.\n\n", L2_numSets, L2_setSize, L2_lineSize, L2IndexBits, L2OffsetBits);
	
	
	if(virtualAdd == 'y')
		cout << "The addresses read in are virtual addresses.\n";
	else
		cout << "The addresses read in are physical addresses.\n";
	
	if(TLB == 'n')
		cout << "TLB is disabled in this configuration.\n";
	if(L2Cache == 'n')
		cout << "L2 cache is disabled in this configuration.\n";
	cout << endl;
	
	if(virtualAdd == 'y')
		cout << "Virtual  ";
	else 
		cout << "Physical ";
	
	cout << "Virt.  Page TLB    TLB TLB  PT   Phys        DC  DC          L2  L2\n";
	cout << "Address  Page # Off  Tag    Ind Res. Res. Pg # DC Tag Ind Res. L2 Tag Ind Res.\n";
	cout << "-------- ------ ---- ------ --- ---- ---- ---- ------ --- ---- ------ --- ----\n";
	
	DCOffsetBitsAnd = 0;
	for(int i = 0; i < DCOffsetBits; i++) {
		DCOffsetBitsAnd = DCOffsetBitsAnd << 1;
		DCOffsetBitsAnd++;
	}
	DCIndexBitsAnd = 0;
	for(int i = 0; i < DCIndexBits; i++) {
		DCIndexBitsAnd = DCIndexBitsAnd << 1;
		DCIndexBitsAnd++;
	}
	DCIndexBitsAnd = DCIndexBitsAnd << DCOffsetBits;
	PTOffsetBitsAnd = 0;
	for(int i=0; i < PTOffsetBits; i++) {
		PTOffsetBitsAnd = PTOffsetBitsAnd << 1;
		PTOffsetBitsAnd++;
	}
	PTIndexBitsAnd = 0;
	for(int i = 0; i < PTIndexBits; i++) {
		PTIndexBitsAnd = PTIndexBitsAnd << 1;
		PTIndexBitsAnd++;
	}
	PTIndexBitsAnd = PTIndexBitsAnd << PTOffsetBits;
	L2OffsetBitsAnd = 0;
	for(int i = 0; i < L2OffsetBits; i++) {
		L2OffsetBitsAnd = L2OffsetBitsAnd << 1;
		L2OffsetBitsAnd++;
	}
	L2IndexBitsAnd = 0;
	for(int i = 0; i < L2IndexBits; i++) {
		L2IndexBitsAnd = L2IndexBitsAnd << 1;
		L2IndexBitsAnd++;
	}
	L2IndexBitsAnd = L2IndexBitsAnd << L2OffsetBits;
	DTLBIndexBitsAnd = 0;
	for(int i = 0; i < DTLBIndexBits; i++) {
		DTLBIndexBitsAnd = DTLBIndexBitsAnd << 1;
		DTLBIndexBitsAnd++;
	}
	
	list<DCEntry> *directCache = new list<DCEntry>[DC_numSets];
	list<DCEntry> *L2_cache = new list<DCEntry>[L2_numSets];
	list<PTEntry> pageTable;
	list<TLBEntry> *TLB_table = new list<TLBEntry>[DTLB_numSets];

	
	
	for(int i = 0; i < PT_numPhysical; i++) {
		PTEntry tempEntry;
		tempEntry.virtualPage = -1;
		tempEntry.physicalPage = i;
		tempEntry.valid = 0;
		tempEntry.dirty = 0;
		pageTable.push_back(tempEntry);
	}

	
	for(int i = 0; i < DC_numSets; i++) {
		for(int x = 0; x < DC_setSize; x++) {
			DCEntry tempEntry;
			tempEntry.validBit = 0;
			tempEntry.tag = -1;
			tempEntry.dirty = 0;
			directCache[i].push_front(tempEntry);
		}
	}
	
	for(int i = 0; i < L2_numSets; i++) {
		for(int x = 0; x < L2_setSize; x++) {
			DCEntry tempEntry;
			tempEntry.validBit = 0;
			tempEntry.tag = -1;
			tempEntry.dirty = 0;
			L2_cache[i].push_front(tempEntry);
		}
	}
	
	for(int i = 0; i < DTLB_numSets; i++) {
		for(int x = 0; x < DTLB_setSize; x++) {
			TLBEntry tempEntry;
			tempEntry.valid = 0;
			tempEntry.tag = -1;
			tempEntry.physicalPage = -1;
			TLB_table[i].push_front(tempEntry);
		}
	}
	
	
	
	while(1) {
		int inputAddress, virtualPageNum, pageOffset, TLBTag, TLBInd, physicalPageNum, DCTag, DCInd, L2Tag, L2Ind, L2Tag2, L2Ind2, DCTag2, DCInd2, printAdress;
		string TLBRes, PTRes, DCRes, L2Res;
		
		cin >> readWrite;
		if(cin.eof())
			break;
		cin >> nullChar;
		cin >> hex >> inputAddress;
		
		if(readWrite == 'W')
			TotalWrites++;
		else if(readWrite == 'R')
			TotalReads++;
		else {
			cout << "hierarchy: unexpected access type\n";
			return 0;
		}
		
		pageOffset = inputAddress & PTOffsetBitsAnd;
		
		printAdress = inputAddress;
		if(virtualAdd == 'n') {
			physicalPageNum = inputAddress & PTIndexBitsAnd;
			physicalPageNum = physicalPageNum >> PTOffsetBits;
		}
		else {
			virtualPageNum = inputAddress & PTIndexBitsAnd;
			virtualPageNum = virtualPageNum >> PTOffsetBits;
			physicalPageNum = 0;
		}
		if(physicalPageNum >= PT_numPhysical && virtualAdd == 'n') {
			printf("hierarchy: physical address %x is too large\n", inputAddress);
			return 0;
		}
		if(virtualPageNum >= PT_numVirtual && virtualAdd == 'y') {
			printf("hierarchy: virtual address %x is too large\n", inputAddress);
			return 0;
		}
		
		
		DCRes = "miss";
		L2Res = "miss";
		PTRes = "miss";
		TLBRes = "miss";
		//if it is a hit
		
		if(TLB == 'y') {
			TLBTag = virtualPageNum >> DTLBIndexBits;
			TLBInd = virtualPageNum & DTLBIndexBitsAnd;
			
			for (list<TLBEntry>::iterator it=TLB_table[TLBInd].begin(); it != TLB_table[TLBInd].end(); ++it) {
				if(it->tag == TLBTag && it->valid == 1) {
					TLBRes = "hit ";
					TLBEntry tempEntry;
					tempEntry.valid = 1;
					tempEntry.tag = it->tag;
					tempEntry.physicalPage = it->physicalPage;
					physicalPageNum = tempEntry.physicalPage;
					TLB_table[TLBInd].erase(it);
					TLB_table[TLBInd].push_front(tempEntry);
					inputAddress = tempEntry.physicalPage;
					inputAddress = inputAddress << PTOffsetBits;
					inputAddress = inputAddress + pageOffset;
					break;
				}
			}
		}
		
		
		
		
		
		if(virtualAdd == 'y' && TLBRes == "miss") {
			ptRefs++;
			for (list<PTEntry>::iterator it=pageTable.begin(); it != pageTable.end(); ++it) {
				if(it->virtualPage == virtualPageNum && it->valid == 1) {
					PTRes = "hit ";
					ptHits++;
					PTEntry tempEntry;
					tempEntry.valid = 1;
					tempEntry.virtualPage = it->virtualPage;
					tempEntry.physicalPage = it->physicalPage;
					physicalPageNum = it->physicalPage;
					if(readWrite == 'W' || it->dirty == 1)
						tempEntry.dirty = 1;
					else
						tempEntry.dirty = 0;
					pageTable.erase(it);
					pageTable.push_front(tempEntry);
					inputAddress = tempEntry.physicalPage;
					inputAddress = inputAddress << PTOffsetBits;
					inputAddress = inputAddress + pageOffset;
					break;
				}
			}
			
			foundInvalid = 0;
			if(PTRes == "miss") {
				ptFaults++;
				diskRefs++;
				for (list<PTEntry>::iterator it=pageTable.begin(); it != pageTable.end(); ++it) {
					if(it->valid == 0) {
						PTEntry tempEntry;
						tempEntry.valid = 1;
						tempEntry.physicalPage = it->physicalPage;
						tempEntry.virtualPage = virtualPageNum;
						physicalPageNum = it->physicalPage;
						foundInvalid = 1;
						if(readWrite == 'W' || it->dirty == 1)
							tempEntry.dirty = 1;
						else
							tempEntry.dirty = 0;
						inputAddress = tempEntry.physicalPage;
						inputAddress = inputAddress << PTOffsetBits;
						inputAddress = inputAddress + pageOffset;
						pageTable.erase(it);
						pageTable.push_front(tempEntry);
						break;
					}
				}
			}
			
			if(foundInvalid == 0 && PTRes == "miss") {
				PTEntry tempEntry;
				tempEntry.valid = 1;
				tempEntry.virtualPage = virtualPageNum;
				if(readWrite == 'W')
					tempEntry.dirty = 1;
				else
					tempEntry.dirty = 0;
				list<PTEntry>::iterator itr = pageTable.end();
				itr--;
				if(itr->dirty == 1)
					diskRefs++;
				tempEntry.physicalPage = itr->physicalPage;
				physicalPageNum = itr->physicalPage;
				inputAddress = tempEntry.physicalPage;
				inputAddress = inputAddress << PTOffsetBits;
				tempAdress = inputAddress;
				inputAddress = inputAddress + pageOffset;
				
				for(int i=tempAdress; i < tempAdress + PT_pageSize; i = i + L2_lineSize) {
					L2Ind2 = i & L2IndexBitsAnd;
					L2Ind2 = L2Ind2 >> L2OffsetBits;
					L2Tag2 = i >> (L2IndexBits + L2OffsetBits);
					
					for (list<DCEntry>::iterator it=L2_cache[L2Ind2].begin(); it != L2_cache[L2Ind2].end(); ++it) {
						if(it->tag == L2Tag2 && it->validBit == 1) {
							it->validBit = 0;
							cout << "L2 Invalid Bit set | tag: " << it->tag << " index: " << L2Ind2 << endl;
							if(it->dirty == 1){
								cout << "writing back to memory | tag: " << it->tag << " index: " << L2Ind2 << endl;
								memRefs++;
							}
						}
					}
				}
				for(int i=tempAdress; i < tempAdress + PT_pageSize; i = i + DC_lineSize) {
					DCInd2 = i & DCIndexBitsAnd;
					DCInd2 = DCInd2 >> DCOffsetBits;
					DCTag2 = i >> (DCIndexBits + DCOffsetBits);
				
					
					for (list<DCEntry>::iterator it=directCache[DCInd2].begin(); it != directCache[DCInd2].end(); ++it) {
							if(it->tag == DCTag2 && it->validBit == 1) {
							it->validBit = 0;
							cout << "DC Invalid Bit set | tag: " << it->tag << " index: " << DCInd2 << endl;
							if(it->dirty == 1 && L2Cache == 'y') {
								L2Hits++;
								cout << "writing back to L2 | tag: " << it->tag << " index: " << DCInd2 << endl;
							}
							else if(it->dirty == 1) {
								memRefs++;
							}
						}
					}
				}
				if(TLB == 'y'){
					for (list<TLBEntry>::iterator it=TLB_table[TLBInd].begin(); it != TLB_table[TLBInd].end(); ++it) {
						if(it->tag == TLBTag) {
							it->valid = 0;
						}
					}
				}
				pageTable.pop_back();
				pageTable.push_front(tempEntry);
			}
			foundInvalid = 0;
			if(TLB == 'y') {
				for (list<TLBEntry>::iterator it=TLB_table[TLBInd].begin(); it != TLB_table[TLBInd].end(); ++it) {
					if(it->valid == 0) {
						TLBEntry tempEntry;
						tempEntry.valid = 1;
						tempEntry.tag = TLBTag;
						tempEntry.physicalPage = physicalPageNum;
						foundInvalid = 1;
						TLB_table[TLBInd].erase(it);
						TLB_table[TLBInd].push_front(tempEntry);
						break;
					}
				}
			}
			if(TLB == 'y' && foundInvalid == 0) {
				TLBEntry tempEntry;
				tempEntry.valid = 1;
				tempEntry.tag = TLBTag;
				tempEntry.physicalPage = physicalPageNum;
				foundInvalid = 1;
				TLB_table[TLBInd].pop_back();
				TLB_table[TLBInd].push_front(tempEntry);
			}
		}
				
		DCInd = inputAddress & DCIndexBitsAnd;
		DCInd = DCInd >> DCOffsetBits;
		DCTag = inputAddress >> (DCIndexBits + DCOffsetBits);
		L2Ind = inputAddress & L2IndexBitsAnd;
		L2Ind = L2Ind >> L2OffsetBits;
		L2Tag = inputAddress >> (L2OffsetBits + L2IndexBits);
		
		for (list<DCEntry>::iterator it=directCache[DCInd].begin(); it != directCache[DCInd].end(); ++it) {
			if(it->tag == DCTag && it->validBit == 1) {
				DCRes = "hit ";
				DCEntry tempEntry;
				tempEntry.validBit = 1;
				tempEntry.tag = it->tag;
				//marks dirty if write on write back
				if((readWrite == 'W' && DC_write == 'n') || it->dirty == 1)
					tempEntry.dirty = 1;
				else
					tempEntry.dirty = 0;
				directCache[DCInd].erase(it);
				directCache[DCInd].push_front(tempEntry);
				break;
			}
		}
		
		//searches for invalid bits, skips if its a write miss on write thru 
		foundInvalid = 0;
		if(DCRes == "miss" && L2Cache == 'n')
			memRefs++;
		if(DCRes == "miss" && !(readWrite == 'W' && DC_write == 'y')) {
			for (list<DCEntry>::iterator it=directCache[DCInd].begin(); it != directCache[DCInd].end(); ++it) {
				if(it->validBit == 0) {
					DCEntry tempEntry;
					tempEntry.validBit = 1;
					tempEntry.tag = DCTag;
					foundInvalid = 1;
					if(readWrite == 'W' && DC_write == 'n')
						tempEntry.dirty = 1;
					else
						tempEntry.dirty = 0;
					directCache[DCInd].erase(it);
					directCache[DCInd].push_front(tempEntry);
					break;
				}
			}
		}
		//replaces the LRU if no invalids, skips if its a write miss on write thru 
		if(foundInvalid == 0 && DCRes == "miss" && !(readWrite == 'W' && DC_write == 'y')) {
			DCEntry tempEntry;
			tempEntry.validBit = 1;
			tempEntry.tag = DCTag;
			if(readWrite == 'W' && DC_write == 'n')
				tempEntry.dirty = 1;
			else
				tempEntry.dirty = 0;
			list<DCEntry>::iterator itr = directCache[DCInd].end();
			itr--;
			if(itr->dirty == 1 && L2Cache == 'y') {
				cout << "writing back to L2 | tag: " << itr->tag << " index: " << DCInd << endl;
				L2Hits++;
				tempAdress = itr->tag;
				tempAdress = tempAdress << DCIndexBits;
				tempAdress = tempAdress + DCInd;
				tempAdress = tempAdress << DCOffsetBits;
				L2Ind2 = tempAdress & L2IndexBitsAnd;
				L2Ind2 = L2Ind2 >> L2OffsetBits;
				L2Tag2 = tempAdress >> (L2OffsetBits + L2IndexBits);
				for (list<DCEntry>::iterator it=L2_cache[L2Ind2].begin(); it != L2_cache[L2Ind2].end(); ++it) {
					if(it->tag == L2Tag2 && it->validBit == 1) {
						DCEntry tempEntry;
						tempEntry.validBit = 1;
						tempEntry.tag = it->tag;
						if(readWrite == 'W' || it->dirty == 1)
							tempEntry.dirty = 1;
						else
							tempEntry.dirty = 0;
						L2_cache[L2Ind2].erase(it);
						L2_cache[L2Ind2].push_front(tempEntry);
						break;
					}
				}
			}
			else if(itr->dirty == 1) {
				cout << "writing back to memory | tag: " << itr->tag << " index: " << DCInd << endl;
				memRefs++;
			}
			directCache[DCInd].pop_back();
			directCache[DCInd].push_front(tempEntry);
		}
		
		//L2 cache 
		if(DCRes == "miss" && L2Cache == 'y') {
			for (list<DCEntry>::iterator it=L2_cache[L2Ind].begin(); it != L2_cache[L2Ind].end(); ++it) {
				if(it->tag == L2Tag && it->validBit == 1) {
					L2Res = "hit ";
					L2Hits++;
					DCEntry tempEntry;
					tempEntry.validBit = 1;
					tempEntry.tag = it->tag;
					if(readWrite == 'W' || it->dirty == 1)
						tempEntry.dirty = 1;
					else
						tempEntry.dirty = 0;
					L2_cache[L2Ind].erase(it);
					L2_cache[L2Ind].push_front(tempEntry);
					break;
				}
			}
			
			foundInvalid2 = 0;
			if(L2Res == "miss") {
				L2Misses++;
				memRefs++;
				for (list<DCEntry>::iterator it=L2_cache[L2Ind].begin(); it != L2_cache[L2Ind].end(); ++it) {
					if(it->validBit == 0) {
						DCEntry tempEntry;
						tempEntry.validBit = 1;
						tempEntry.tag = L2Tag;
						foundInvalid2 = 1;
						if(readWrite == 'W')
							tempEntry.dirty = 1;
						else
							tempEntry.dirty = 0;
						L2_cache[L2Ind].erase(it);
						L2_cache[L2Ind].push_front(tempEntry);
						break;
					}
				}
			}
			//replaces the LRU if no invalids, skips if its a write miss on write thru 
			if(foundInvalid2 == 0 && L2Res == "miss") {
				DCEntry tempEntry;
				tempEntry.validBit = 1;
				tempEntry.tag = L2Tag;
				if(readWrite == 'W')
					tempEntry.dirty = 1;
				else
					tempEntry.dirty = 0;
				list<DCEntry>::iterator itr = L2_cache[L2Ind].end();
				itr--;
				if(itr->validBit == 1) {
					tempAdress = itr->tag;
					tempAdress = tempAdress << L2IndexBits;
					tempAdress = tempAdress + L2Ind;
					tempAdress = tempAdress << L2OffsetBits;
					if(itr->dirty == 1) {
						cout << "writing back to memory | tag: " << itr->tag << " index: " << L2Ind << endl;
						memRefs++;
					}
					for(int i=tempAdress; i < tempAdress + L2_lineSize; i = i + DC_lineSize) {
						DCInd2 = i & DCIndexBitsAnd;
						DCInd2 = DCInd2 >> DCOffsetBits;
						DCTag2 = i >> (DCIndexBits + DCOffsetBits);
						
						for (list<DCEntry>::iterator it=directCache[DCInd2].begin(); it != directCache[DCInd2].end(); ++it) {
							if(it->tag == DCTag2 && it->validBit == 1) {
								it->validBit = 0;
								cout << "DC Invalid Bit set | tag: " << it->tag << " index: " << DCInd2 << endl;
								if(it->dirty == 1)
									L2Hits++;
							}
						}
					}
				}
				L2_cache[L2Ind].pop_back();
				L2_cache[L2Ind].push_front(tempEntry);
			}
		}
		
		
		printf("%08x ", printAdress);
		if(virtualAdd == 'y')
			printf("%6x ", virtualPageNum);
		else
			printf("       ");
		printf("%4x ",pageOffset);
		if(TLB == 'y')
			printf("%6x %3x %4s ",TLBTag, TLBInd, TLBRes.c_str());
		else
			printf("                ");
		if(virtualAdd == 'y' && TLBRes == "miss")
			printf("%4s ", PTRes.c_str());
		else
			printf("     ");
		printf("%4x %6x %3x %4s ", physicalPageNum, DCTag, DCInd, DCRes.c_str());
		if(L2Cache == 'y' && DCRes == "miss")
			printf("%6x %3x %4s", L2Tag, L2Ind, L2Res.c_str());
		cout << endl;

		if(TLBRes == "miss")
			dtlbMisses++;
		else if(TLBRes == "hit ")
			dtlbHits++;
		
		if(DCRes == "miss")
			dcMisses++;
		else if(DCRes == "hit ")
			dcHits++;
	}
	cout << "\nSimulation statistics\n\n";
	
	if(TLB == 'n')
		dtlbHits = dtlbMisses = dtlbHitRatio = 0;
	else if(dtlbMisses == 0)
		dtlbHitRatio = 0;
	else 
		dtlbHitRatio = float(dtlbHits) / float(dtlbMisses + dtlbHits);
	
	
	if(TotalReads == 0)
		readRatio = 0;
	else if(TotalWrites == 0)
		readRatio = 1;
	else
		readRatio = float(TotalReads) / float(TotalWrites + TotalReads);
	
	if(virtualAdd == 'n')
		ptHits = ptFaults = ptHitRatio = 0;
	else if(ptFaults == 0)
		ptHitRatio = 0;
	else
		ptHitRatio = float(ptHits) / float(ptHits + ptFaults);
	
	if(dcMisses == 0)
		dcHitRatio = 0;
	else
		dcHitRatio = float(dcHits) / float(dcHits + dcMisses);
	
	if(L2Cache == 'n')
		L2Hits = L2Misses = L2HitRatio = 0;
	else if(L2Misses == 0)
		L2HitRatio = 0;
	else 
		L2HitRatio = float(L2Hits) / float(L2Hits + L2Misses);
	
	printf("dtlb hits        : %i\ndtlb misses      : %i\ndtlb hit ratio   : ",dtlbHits,dtlbMisses);
	if(TLB == 'y')
		printf("%f\n\n",dtlbHitRatio);
	else
		printf("N/A\n\n");
	printf("pt hits          : %i\npt faults        : %i\npt hit ratio     : ",ptHits,ptFaults);
	if(virtualAdd == 'y')
		printf("%f\n\n",ptHitRatio);
	else
		printf("N/A\n\n");
	printf("dc hits          : %i\ndc misses        : %i\ndc hit ratio     : %f\n\n",dcHits,dcMisses,dcHitRatio);	
	printf("L2 hits          : %i\nL2 misses        : %i\nL2 hit ratio     : ",L2Hits,L2Misses);
	if(L2Cache == 'y')
		printf("%f\n\n",L2HitRatio);
	else
		printf("N/A\n\n");
	printf("Total reads      : %i\nTotal writes     : %i\nRatio of reads   : %f\n\n",TotalReads,TotalWrites,readRatio);
	printf("main memory refs : %i\npage table refs  : %i\ndisk refs        : %i\n",memRefs,ptRefs,diskRefs);
}
