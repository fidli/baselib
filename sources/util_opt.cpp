#ifndef UTIL_OPT_C
#define UTIL_OPT_C

#include "util_rng.cpp"
#include "util_sort.cpp"

enum EvoParametersSelectionType{
    EvoParametersSelectionType_Invalid,
    EvoParametersSelectionType_RouletteSelection,
    EvoParametersSelectionType_RankSelection
};

union EvoParametersSelectionSettings{
    
};

enum EvoParametersEncodingType{
    EvoParametersEncodingType_Invalid,
    EvoParametersEncodingType_Bitfield
};

union EvoParametersEncodingSettings{
    struct {
        uint16 size;
    } bitfield;
};

enum EvoParametersMutationType{
    EvoParametersMutationType_Invalid,
    EvoParametersMutationType_Bitflip
};

union EvoParametersMutationSettings{
    struct {
        uint32 chance;
        uint32 modulus;
    } bitflip;
};

enum EvoParametersCrossoverType{
    EvoParametersCrossoverType_Invalid,
    EvoParametersCrossoverType_Uniform,
    EvoParametersCrossoverType_OnePoint
};

union EvoParametersCrossoverSettings{
    struct {
        uint8 chance;
    } uniform;
    struct {
        uint32 position;
    } onePoint;
};

enum EvoParametersInitPopType{
    EvoParametersInitPopType_Invalid,
    EvoParametersInitPopType_Random,
    EvoParametersInitPopType_RandomAndValid
};

struct Instance;


struct EvoIndividual{
    union{
        struct{
            uint16 arraysize;
            uint16 size;
            uint64 * bits;
        } bitfield;
    } genotype;
    float32 fitness;
};

static struct EvoPopulation{
    EvoIndividual * individuals;
    uint32 count;
};


struct BasicEvoParameters{
    uint32 popsize;
    uint32 generations;
    uint16 elitCount;
    EvoParametersEncodingType encoding;
    EvoParametersEncodingSettings encodingSettings;
    EvoParametersInitPopType initPop;
    EvoParametersSelectionType selection;
    EvoParametersSelectionSettings selectionSettings;
    EvoParametersCrossoverType crossover;
    EvoParametersCrossoverSettings crossoverSettings;
    EvoParametersMutationType mutation;
    EvoParametersMutationSettings mutationSettings;
    float32 (*fitness)(const EvoIndividual *, Instance *);
    void (*saveResult)(const EvoIndividual *, Instance *);
};


static int8 fitnessCmp(const EvoIndividual * a, const EvoIndividual * b){
    if(aseq(a->fitness, b->fitness)) return 0;
    if (a->fitness > b->fitness) return 1;
    return 0;
}


void evoSolve(const BasicEvoParameters * parameters, Instance * instance){
    PUSHI;
    // prepare two sets of pops like a memory pool
    uint16 size = parameters->encodingSettings.bitfield.size;
    uint16 arraysize = (size / 64) + ((size % 64 == 0) ? 0 : 1);
    ASSERT(parameters->encoding == EvoParametersEncodingType_Bitfield);
    uint64 * bits = &PUSHA(uint64, arraysize * 2 * parameters->popsize);
    EvoPopulation popdb[2];
    for(int i = 0; i < ARRAYSIZE(popdb); i++){
        popdb[i].count = parameters->popsize;
        popdb[i].individuals = &PUSHA(EvoIndividual, parameters->popsize);
        for(int j = 0; j < parameters->popsize; j++){
            switch(parameters->encoding){
                case EvoParametersEncodingType_Bitfield:{
                    
                    popdb[i].individuals[j].genotype.bitfield.size = size;
                    popdb[i].individuals[j].genotype.bitfield.arraysize = arraysize;
                    popdb[i].individuals[j].genotype.bitfield.bits = &bits[parameters->popsize * i + j];
                    
                }break;
                default:{
                    INV;
                }break;
                
            }
        }
    }
    uint32 currentPopIndex = 1;
    uint32 previousPopIndex = 0;
    EvoPopulation * currentPop = &popdb[currentPopIndex];
    EvoPopulation * previousPop = &popdb[previousPopIndex];
    //preparation done
    
    //generate init population
    switch(parameters->initPop){
        case EvoParametersInitPopType_Random:
        case EvoParametersInitPopType_RandomAndValid:{
            ASSERT(parameters->encoding == EvoParametersEncodingType_Bitfield); //implement branching case here. if encoding is not bitfield
            for(uint32 i = 0; i < parameters->popsize; i++){
                EvoIndividual * individual = &previousPop->individuals[i];
                uint32 bits = individual->genotype.bitfield.size;
                uint16 index = 0;
                uint64 partcounter = 0;
                individual->genotype.bitfield.bits[index] = 0;
                while(bits > 0){
                    //randlcg is 16 bit...
                    int shift = MIN(16, bits);
                    bits -= shift;
                    individual->genotype.bitfield.bits[index] = individual->genotype.bitfield.bits[index] << shift;
                    individual->genotype.bitfield.bits[index] |= (randlcg() >> 16 - shift);
                    partcounter++;
                    if(partcounter % 4 == 0){
                        index++;
                        individual->genotype.bitfield.bits[index] = 0;
                    }
                }
                individual->fitness = parameters->fitness(individual, instance);
                
                if(parameters->initPop == EvoParametersInitPopType_RandomAndValid){
                    //not implemented yet
                    ASSERT(!"implement when needed - toss 1s or 0s ?");
                    if(aseq(individual->fitness, 0)){
                        uint16 ones = 0;
                        uint32 bitIndex = 0;
                        for(uint16 arrayIndex = 0; arrayIndex < individual->genotype.bitfield.arraysize; arrayIndex++){
                            if(bitIndex == individual->genotype.bitfield.size){
                                break;
                            }
                            if((individual->genotype.bitfield.bits[arrayIndex] >> (bitIndex % 64)) & 1){
                                ones++;
                            }
                            bitIndex++;
                        }
                        
                        while(ones > 0 && aseq(individual->fitness, 0)){
                            //toss  1 away
                            uint32 tossIndex = randlcg() % ones;
                            //here i stopped
                            /*
                            for(uint8 bitIndex = 0; bitIndex < individual->genotype.size; bitIndex++){
                                if((individual->genotype.bitfield >> bitIndex) & 1){
                                    if(tossIndex == 0){
                                        individual->genotype.bitfield &= ~((uint64)1 << bitIndex);
                                        individual->fitness = parameters->fitness(individual, instance);
                                        ones--;
                                        break;
                                    }else{
                                        tossIndex--;
                                    }
                                }
                                }*/
                        }
                        
                    }
                }
            }
            
        }break;
        default:{
            INV;
        }break;
    }
    
    //init pop done, now let the generations rise
    
    uint32 generation = 0;
    
    while(generation < parameters->generations){
        
        //sort previous by fitness
        mergeSort((byte *)previousPop->individuals, sizeof(EvoIndividual), parameters->popsize, (int8 (*)(void * a, void * b)) &fitnessCmp);
        
        //keep elitists
        for(uint32 elitIndex = 0; elitIndex < parameters->elitCount; elitIndex++){
            currentPop->individuals[elitIndex] = previousPop->individuals[elitIndex];
        }
        
        //populate new generation
        for(uint32 popmemberIndex = parameters->elitCount; popmemberIndex < parameters->popsize; popmemberIndex++){
            
            EvoIndividual * baby = &currentPop->individuals[popmemberIndex];
            
            EvoIndividual * sacrifice1 = NULL;
            EvoIndividual * sacrifice2 = NULL;
            //selection
            switch(parameters->selection){
                case EvoParametersSelectionType_RankSelection:{
                    
                    ASSERT(parameters->popsize < (1 << 15)); //deal with overflow later
                    //also randlcg is only 16 bits
                    uint32 parts = ((parameters->popsize) * (1+parameters->popsize))/2;
                    int32 choice = randlcg() % parts; // need to go to negative numbers
                    for(uint32 i = 0; i < parameters->popsize; i++){
                        choice -= (parameters->popsize - i);
                        if(choice < 0){
                            sacrifice1 = &previousPop->individuals[i];
                            break;
                        }
                    }
                    choice = randlcg() % parts;
                    for(uint32 i = 0; i < parameters->popsize; i++){
                        choice -= (parameters->popsize - i);
                        if(choice < 0){
                            sacrifice2 = &previousPop->individuals[i];
                            break;
                        }
                    }
                }break;
                case EvoParametersSelectionType_RouletteSelection:{
                    
                    float32 maxFitness = 0;
                    for(int i = 0; i < previousPop->count; i++){
                        maxFitness += previousPop->individuals[i].fitness;
                    }
                    
                    float32 choice = ((float32)(randlcg() % 100) / 100.0) * maxFitness;
                    for(uint32 i = 0; i < parameters->popsize; i++){
                        choice -= previousPop->individuals[i].fitness;
                        if(aseqr(choice, 0)){
                            sacrifice1 = &previousPop->individuals[i];
                            break;
                        }
                    }
                    
                    choice = ((float32)(randlcg() % 100) / 100.0) * maxFitness;
                    for(uint32 i = 0; i < parameters->popsize; i++){
                        choice -= previousPop->individuals[i].fitness;
                        if(aseqr(choice, 0)){
                            sacrifice2 = &previousPop->individuals[i];
                            break;
                        }
                    }
                }break;
                default:{
                    INV;
                }break;
            }
            
            
            //selection done
            ASSERT(sacrifice1);
            ASSERT(sacrifice2);
            
            ASSERT(parameters->encoding == EvoParametersEncodingType_Bitfield); //branch here when encoding is not bitfield
            
            
            //crossing
            switch(parameters->crossover){
                case EvoParametersCrossoverType_OnePoint:{
                    ASSERT(!"inspect test and validate this");
                    int64 breakpoint = parameters->crossoverSettings.onePoint.position; // need to go to neg numbers
                    for(uint16 p = 0; p < sacrifice1->genotype.bitfield.arraysize; p++){
                        if(breakpoint >= 64){
                            baby->genotype.bitfield.bits[p] = sacrifice1->genotype.bitfield.bits[p];
                            breakpoint -= 64;
                        }else if(breakpoint > 0){
                            baby->genotype.bitfield.bits[p] = 0;
                            baby->genotype.bitfield.bits[p] |= sacrifice1->genotype.bitfield.bits[p] & (~((uint64)0) << breakpoint);
                            baby->genotype.bitfield.bits[p] |= sacrifice2->genotype.bitfield.bits[p] & (~((uint64)0) >> (64 - breakpoint));
                        }else{
                            baby->genotype.bitfield.bits[p] = sacrifice2->genotype.bitfield.bits[p];
                        }
                        
                    }
                    
                }break;
                case EvoParametersCrossoverType_Uniform:{
                    ASSERT(parameters->crossoverSettings.uniform.chance < 100 && parameters->crossoverSettings.uniform.chance > 0);
                    
                    for(uint16 p = 0; p < baby->genotype.bitfield.arraysize; p++){
                        baby->genotype.bitfield.bits[p] = 0;
                    }
                    
                    uint16 index = -1;
                    for(uint64 i = 0; i < sacrifice1->genotype.bitfield.size; i++){
                        if(i % 64 == 0){
                            index++;
                        }
                        if(randlcg() % 100 < parameters->crossoverSettings.uniform.chance){
                            baby->genotype.bitfield.bits[index] |= sacrifice1->genotype.bitfield.bits[index] & ((uint64)1 << (i%64));
                        }else{
                            baby->genotype.bitfield.bits[index] |= sacrifice2->genotype.bitfield.bits[index] & ((uint64)1 << (i%64));
                        }
                        
                    }
                }break;
                default:{
                    INV;
                }break;
            }
            
            ASSERT(parameters->encoding == EvoParametersEncodingType_Bitfield); //other types might not suffice
            
            //mutation
            switch(parameters->mutation){
                case EvoParametersMutationType_Bitflip:{
                    uint16 index = -1;
                    for(uint64 i = 0; i < sacrifice1->genotype.bitfield.size; i++){
                        if(i % 64 == 0){
                            index++;
                        }
                        if(randlcgd() % parameters->mutationSettings.bitflip.modulus < parameters->mutationSettings.bitflip.chance){
                            if(((baby->genotype.bitfield.bits[index] >> (i%64)) & 1)){
                                baby->genotype.bitfield.bits[index] &= ~(((uint64)1) << (i%64));
                            }else{
                                baby->genotype.bitfield.bits[index] |= (((uint64)1) << (i%64));
                            }
                        }
                        
                    }
                }break;
                default:{
                    INV;
                }break;
            }
            
            //user defined fitness
            baby->fitness = parameters->fitness(baby, instance);
        }
        //switch memory pools
        previousPop = &popdb[++previousPopIndex % 2];
        currentPop = &popdb[++currentPopIndex % 2];
        generation++;
    }
    
    //save result
    mergeSort((byte *)previousPop->individuals, sizeof(EvoIndividual), parameters->popsize, (int8 (*)(void * a, void * b)) &fitnessCmp);
    EvoIndividual * alfa = &previousPop->individuals[0];
    
    //user defined result saving
    parameters->saveResult(alfa, instance);
    
    POPI;
    
}

#endif