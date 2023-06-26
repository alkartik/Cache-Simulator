#include<iostream>
#include <fstream>
#include<string>
#include<vector>
#include<tuple>
#include<iomanip>
#define int long long
using namespace std;
//gotta store the addresses in long, so for that we can just define it instead
int outputFormat = 1; //0 corresponds to printing the output for graphing, which we parse to form the graphs.
    //while 1 corressponds to the output given in the assignment.

int useTime = 0;
int WriteMiss[3], ReadMiss[3], WriteBacks[3], Writes[3], Reads[3];

int totalTimeTaken = 0;

struct Block
{
    int dirtyBit = 0, lru = 0;
    vector<int> data;
    Block(int size)
    {
        data = vector<int>(size,-1);
    }
};
int HexatoDecimal(string hex){
    int base  = 1;
    int decimal = 0;
    for(int i= hex.size()-1; i>=0;i--){
        if(hex[i]>='0' && hex[i]<='9'){
            decimal+=(hex[i]-48)*base;
             base*=16;
        }
        else if(hex[i] >= 'a' && hex[i] <= 'f'){
            decimal+=(hex[i]-87)*base;
             base*=16;
        }
        else{
             decimal+=((hex[i])-55)*base;
             base*=16;
        }
    }
    return decimal;
}

class Cache
{
    int cntSet = 8; 
    int cacheType = 1; //1 corresponds to L1, 2 corresponds to L2, 0 corresponds to DRAM
    
    public:
    int Blocksize = 64, Size = 1024, Assoc = 2; //default values
    Cache *L2;

    vector<Block> Set;
    vector<vector<Block>> Mem;
    vector<vector<int>> TagArray;

    Cache(int bs, int s, int a, int type)
    {
        cacheType = type;
        Blocksize = bs;
        Size = s;
        Assoc = a;
        cntSet = Size/(Blocksize*Assoc);
        Block blk = Block(Blocksize);
        Set = vector<Block>(Assoc,blk);
        Mem = vector<vector<Block>>(Size/(Blocksize*Assoc),Set);
        TagArray = vector<vector<int>>(Size/(Blocksize*Assoc),vector<int>(Assoc,-1));
        
    }

    void Reset(int bs, int s, int a, int type)
    {
        cacheType = type;
        Blocksize = bs;
        Size = s;
        Assoc = a;
        cntSet = Size/(Blocksize*Assoc);
        Block blk = Block(Blocksize);
        Set = vector<Block>(Assoc,blk);
        Mem = vector<vector<Block>>(Size/(Blocksize*Assoc),Set);
        TagArray = vector<vector<int>>(Size/(Blocksize*Assoc),vector<int>(Assoc,-1));
    }

    void Read(int address)
    {
        totalTimeTaken += (cacheType==1)?1:(cacheType==2)?20:200;
        Reads[cacheType]++;
        if(!cacheType) //then this is the DRAM memory
        {
            //return Mem[address/Blocksize][address%Blocksize];
            return; //returns a block of the same blocksize.
        }
        int offset = address%(Blocksize);
        int tag = address/(Size/Assoc);
        int set = (address/(Blocksize))%(Size/(Blocksize*Assoc));

        for(int i=0;i<Assoc;i++) //this is O(size(Assoc)), hence we can also set the LRU calculation to be O(size(Assoc)).
        {
            if(TagArray[set][i] == tag) //then we have a match and we can just return this value.
            {
                Mem[set][i].lru = useTime++;
                return; //We should return the byte at this offset value. this is a Read Hit.
            }
        } 
        ReadMiss[cacheType]++; //otherwise we have a ReadMiss at this cache.

            
        int leastUsed = 0;
        for (int i = 0; i < Assoc; i++)
        {
            if(Mem[set][i].lru < Mem[set][leastUsed].lru)
            {
                leastUsed = i;
            }
        }
        //now we have the block that is least used, and can evict it.
        if(Mem[set][leastUsed].dirtyBit) //if the dirty bit is set, then we must write it back to the L2 cache.
        {
            // L2WriteBacks++;
            WriteBacks[cacheType]++;
            //the address to write here is actually
            int remAddr = TagArray[set][leastUsed]*(Size/Assoc)+set*(Blocksize);
            L2->Write(remAddr);
        }
        //now we can replace the block with the new value.
        L2->Read(address); //then we read the required data from the L2 cache. then we must find the least recently used block



        Mem[set][leastUsed].lru = useTime++; //reset its last used time.
        Mem[set][leastUsed].dirtyBit = 0; //resets the dirty bit.
        //also gotta update the value in the tag array
        TagArray[set][leastUsed] = tag; //the tag of the current address.

    
    }

    void Write(int address) //writes the value val at the Block val.
    {
        totalTimeTaken += (cacheType==1)?1:(cacheType==2)?20:200;
        Writes[cacheType]++;
        if(!cacheType) //then this is the DRAM memory
        {
            //return Mem[address/Blocksize][address%Blocksize];
            return; //returns a block of the same blocksize.
        }
        int offset = address%(Blocksize);
        int tag = address/(Size/Assoc);
        int set = (address/(Blocksize))%(Size/(Blocksize*Assoc));

        for(int i=0;i<Assoc;i++) //this is O(size(Assoc)), hence we can also set the LRU calculation to be O(size(Assoc)).
        {
            if(TagArray[set][i] == tag) //then we have a match and we can just return this value.
            {
                Mem[set][i].lru = useTime++;
                Mem[set][i].dirtyBit = 1; //we also change the dirty bit to 1 in this case, since it is a write.
                return; //We should return the byte at this offset value
            }
        } 
        WriteMiss[cacheType]++;


        int leastUsed = 0;
        for (int i = 0; i < Assoc; i++)
        {
            if(Mem[set][i].lru < Mem[set][leastUsed].lru)
            {
                leastUsed = i;
            }
        }
        //now we have the block that is least used, and can evict it.
        if(Mem[set][leastUsed].dirtyBit > 0) //if the dirty bit is set, then we must write it back to the L2 cache.
        {
            WriteBacks[cacheType]++;
            int remAddr = TagArray[set][leastUsed]*(Size/Assoc) + set*(Blocksize);
            L2->Write(remAddr);
        }

        L2->Read(address); //Then we read the required data from the L2 cache and place it in the least recently used block.

        //now we can replace the block with the new value.
        Mem[set][leastUsed].lru = useTime++; //reset its last used time.
        Mem[set][leastUsed].dirtyBit = 1; //resets the dirty bit.
        //also gotta update the value in the tag array
        TagArray[set][leastUsed] = tag; //the tag of the current address.
    }
};

  
    void PrintResults()
    {
        cout << "===== Simulation configuration =====\n";
    cout << "\n\n===== Simulation Results =====";
    cout << "\ni. number of L1 reads:\t\t\t\t" << dec << Reads[1];
    cout << "\nii. number of L1 read misses:\t\t\t" << dec << ReadMiss[1];
    cout << "\niii. number of L1 writes:\t\t\t" << dec << Writes[1];
    cout << "\niv. number of L1 write misses:\t\t\t" << dec << WriteMiss[1];
    cout << "\nv. L1 miss rate:\t\t\t\t" << fixed << setprecision(4) << ((double)(ReadMiss[1] + WriteMiss[1]))/(Reads[1] + Writes[1]);
    cout << "\nvi. number of writebacks from L1 memory:\t" << dec << WriteBacks[1] << "\n";
    
    {
        cout << "\nvii. number of L2 reads:\t\t\t" << dec << Reads[2];
        cout << "\nviii. number of L2 read misses:\t\t\t" << dec << ReadMiss[2];
        cout << "\nix. number of L2 writes:\t\t\t" << dec << Writes[2];
        cout << "\nx. number of L2 write misses:\t\t\t" << dec << WriteMiss[2];
        cout << "\nxi. L2 miss rate:\t\t\t\t" << fixed << setprecision(4) << ((double)(ReadMiss[2] + WriteMiss[2]))/(Reads[2] + Writes[2]); //((float)L2.READ_MISS+(float)L2.WRITE_MISS)/ (L2.READ+L2.WRITE);
        cout << "\nxii. number of writebacks from L2 memory:\t" << dec << WriteBacks[2] << "\n";
    }
    
    cout << "Writes to DRAM  " << Writes[0] << "\n";
    cout << "Reads to DRAM  " << Reads[0] << "\n";
    cout << "Total time taken: " << totalTimeTaken << " ns \n";
    }

  


void RunProgram(ifstream &file , int bs,int l1s, int l2s, int l1a, int l2a)
{
    int Blocksize = bs, L1_Size = l1s, L2_Size = l2s,
    L1_Assoc = l1a, L2_Assoc = l2a;
    Cache L1 = Cache(Blocksize,L1_Size,L1_Assoc, 1);
    Cache L2 = Cache(Blocksize,L2_Size,L2_Assoc, 2);
    Cache dram = Cache(Blocksize,1024,16,0); //random values for now, since its the dram anyway.
    totalTimeTaken = 0; //reset the total time taken.
    Reads[0] = 0; Reads[1] = 0; Reads[2] = 0;
    Writes[0] = 0; Writes[1] = 0; Writes[2] = 0;
    ReadMiss[0] = 0; ReadMiss[1] = 0; ReadMiss[2] = 0;
    WriteMiss[0] = 0; WriteMiss[1] = 0; WriteMiss[2] = 0;
    WriteBacks[0] = 0; WriteBacks[1] = 0; WriteBacks[2] = 0;
    useTime = 0; //reset the use time.

    L1.L2 = &L2; //gives a pointer of L2 to cache L1
        L2.L2 = &dram;
        while(file.good())
        {
            string type, b;
            file >> type;
            if(file.good())
            {
                file >> b;
                int addr = HexatoDecimal(b);
                if(type == "r" || type == "R")
                {
                    //read
                    L1.Read(addr);
                }
                else if(type == "w" || type == "W")
                {
                    //write
                    L1.Write(addr);
                }
            }
            else
            {
                break;
            }
        }
    if(outputFormat)
        PrintResults();
    file.close();
}

  void printGraphingOutput() //function made to get the values out to easily parse and make graphs
    {
        string outFolder = "./memory_trace_files/"; //the folder where the output files will be stored.
        string trace[8];
        for (int i = 0; i < 8; i++)
        {
            trace[i] = "./memory_trace_files/trace" + to_string(i+1) + ".txt";
        }
        //first graph values
        {
            int k = 8;
            cout << "BlockSize,";
            for (int i = 0; i < 8; i++)
            {
                cout << "trace" << i+1 << ",";
            }
                cout << "\n";
            for (int j = 0; j < 5; j++)
            {
                cout << k << ",";
                for (int i = 0; i < 8; i++)
                {
                    std::ifstream file(trace[i]);
                    RunProgram(file, k, 1024, 65536, 2, 8);
                    cout << totalTimeTaken << ",";
                }
                cout << "\n";
                k = k*2;
            }
        }
        //then we have varying L1 size and keeping others fixed.
        cout << "\n\n\n" << endl; //some blank lines in between.
        {
            int k = 512;
            cout << "L1Size,";
            for (int i = 0; i < 8; i++)
            {
                cout << "trace" << i+1 << ",";
            }
            cout << "\n";
            for (int j = 0; j < 5; j++)
            {
                cout << k << ",";
                for (int i = 0; i < 8; i++)
                {
                    std::ifstream file(trace[i]);
                    RunProgram(file, 64, k, 65536, 2, 8);
                    cout << totalTimeTaken << ",";
                }
                cout << "\n";
                k = k*2;
            }
        }
        cout << "\n\n\n" << endl; //some blank lines in between.
        {
            int k = 1;
            cout << "L1Associativity,";
            for (int i = 0; i < 8; i++)
            {
                cout << "trace" << i+1 << ",";
            }
            cout << "\n";
            for (int j = 0; j < 5; j++)
            {
                cout << k << ",";
                for (int i = 0; i < 8; i++)
                {
                    std::ifstream file(trace[i]);
                    RunProgram(file, 64, 1024, 65536, k, 8);
                    cout << totalTimeTaken << ",";
                }
                cout << "\n";
                k = k*2;
            }
        }
        cout << "\n\n\n" << endl; //some blank lines in between.
        {
            int k = 16384;
            cout << "L2Size,";
            for (int i = 0; i < 8; i++)
            {
                cout << "trace" << i+1 << ",";
            }
            cout << "\n";
            for (int j = 0; j < 5; j++)
            {
                cout << k << ",";
                for (int i = 0; i < 8; i++)
                {
                    std::ifstream file(trace[i]);
                    RunProgram(file, 64, 1024, k, 2, 8);
                    cout << totalTimeTaken << ",";
                }
                cout << "\n";
                k = k*2;
            }
        }
        cout << "\n\n\n" << endl; //some blank lines in between.
        {
            int k = 1;
            cout << "L2Associativity,";
            for (int i = 0; i < 8; i++)
            {
                cout << "trace" << i+1 << ",";
            }
            cout << "\n";
            for (int j = 0; j < 5; j++)
            {
                cout << k << ",";
                for (int i = 0; i < 8; i++)
                {
                    std::ifstream file(trace[i]);
                    RunProgram(file, 64, 1024, 65536, 2, k);
                    cout << totalTimeTaken << ",";
                }
                cout << "\n";
                k = k*2;
            }
        }
        
      
        


    }

signed main(signed argc, char *argv[])
{
    
	if ((argc != 7) && (outputFormat > 0))
	{
		std::cerr << "7 arguments required\n";
		return 0;
	}
    if(outputFormat)
    {
        std::ifstream file(argv[6]);
        int bs = stoi(argv[1]), l1s = stoi(argv[2]), l1a = stoi(argv[3]), l2s = stoi(argv[4]), l2a = stoi(argv[5]);
        
        if (file.is_open())
        {    
                RunProgram(file,bs,l1s,l2s,l1a,l2a);
        }
        else
        {
            std::cerr << "File " << argv[6] << "could not be opened. Terminating...\n";

            return 0;
        }

        return 0;
    }
    else
    {
        printGraphingOutput();
        return 0;
    }
}