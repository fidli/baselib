#ifndef UTIL_ANN
#define UTIL_ANN



enum NeuralNetActivationFunctionType{
    NeuralNetActivationFunctionType_Invalid,
    NeuralNetActivationFunctionType_Sigmoid,
    NeuralNetActivationFunctionType_Identity
};

enum NeuralNetTrainLossFunctionType{
    NeuralNetTrainLossFunctionType_Invalid,
    NeuralNetTrainLossFunctionType_QuadError
};




static inline float32 sigmoidActivationFunction(float32 value){
    return 1.0 / (1.0 + epow(-1*value));
}


struct NeuralHiddenLayer{
    NeuralNetActivationFunctionType activationFunction;
    matNM W;
};

struct MLPNeuralNet{
    NeuralHiddenLayer * hiddenLayers;
    uint16 hiddenLayersAmount;
};

struct RNNNeuralNet{
    NeuralNetActivationFunctionType activationFunction;
    matNM W;
    vN outputs;
};
/////////////// prediction

void resetMemoryRNNNeuralNet(RNNNeuralNet * toReset){
    for(uint32 i = 0; i < toReset->outputs.size; i++){
        toReset->outputs.v[i] = 0;
    }
}

bool predictRNNStep(RNNNeuralNet * model, vN * input, vN * output){
    ASSERT(output->size == model->outputs.size);
    for(uint32 i = 0; i < model->outputs.size; i++){
        output->v[i] = model->outputs.v[i];
    }
    PUSHI;
    vN matrixInput;
    matrixInput.size = model->W.height;
    matrixInput.v = &PUSHA(float32, matrixInput.size);
    for(int i = 0; i < model->outputs.size; i++){
        matrixInput.v[i] = model->outputs.v[i];
    }
    for(int i = 0; i < input->size; i++){
        matrixInput.v[model->outputs.size + i] = input->v[i];
    }
    matrixInput.v[matrixInput.size-1] = 1;
    
    ASSERT(input->size == model->W.height - model->W.width - 1);
    
    mul(&matrixInput, &model->W, &model->outputs);
    for(int i = 0; i < model->outputs.size; i++){
        switch(model->activationFunction){
            case NeuralNetActivationFunctionType_Sigmoid:
            model->outputs.v[i] = sigmoidActivationFunction(model->outputs.v[i]);
            break;
            case NeuralNetActivationFunctionType_Identity:
            model->outputs.v[i] = model->outputs.v[i];
            break;
            default:
            ASSERT(!"FUCK");
            break;
        }
    }
    
    POPI;
    return true;
}


bool predictMLP(const MLPNeuralNet * model, vN * input, vN * result){
    PUSHI;
    
    float32 yP;
    vN * layerOuts = &PUSHA(vN, model->hiddenLayersAmount+1);
    vN * layerOutput = &layerOuts[0];
    layerOutput->size = input->size+1;
    layerOutput->v = &PUSHA(float32, layerOutput->size);
    for(int i = 0; i < input->size; i++){
        layerOutput->v[i] = input->v[i];
    }
    layerOutput->v[input->size] = 1;
    
    for(int hiddenLayerIndex = 0; hiddenLayerIndex < model->hiddenLayersAmount; hiddenLayerIndex++){
        NeuralHiddenLayer * layer = &model->hiddenLayers[hiddenLayerIndex];
        
        vN layerArg; 
        layerArg.size = layer->W.width;
        layerArg.v = &PUSHA(float32, layerArg.size);
        
        mul(layerOutput, &layer->W, &layerArg);
        
        layerOutput = &layerOuts[hiddenLayerIndex+1];
        layerOutput->size = layer->W.width+1;
        layerOutput->v = &PUSHA(float32, layerOutput->size);
        
        switch(layer->activationFunction){
            case NeuralNetActivationFunctionType_Sigmoid:
            for(int i = 0; i < layerOutput->size-1; i++){
                layerOutput->v[i] = sigmoidActivationFunction(layerArg.v[i]);
            }
            break;
            case NeuralNetActivationFunctionType_Identity:
            for(int i = 0; i < layerOutput->size-1; i++){
                layerOutput->v[i] = layerArg.v[i];
            }
            break;
            default:
            ASSERT(!"FUCK");
            break;
        }
        layerOutput->v[layerOutput->size-1] = 1;
        
    }
    
    
    ASSERT(result->size == layerOutput->size-1); //no bias
    for(uint32 i = 0; i < layerOutput->size-1; i++){
        result->v[i] = layerOutput->v[i];
    }
    
    POPI;
    
    return true;
}



/////////////// training

struct NeuralNetTrainParams{
    float32 learningRate;
    NeuralNetTrainLossFunctionType lossFunction;
    //todo more parameters, like type of learning, batches, evo, etc...
};

struct NeuralNetTrainPair{
    vN input;
    vN result;
    bool * matters;
};




void trainRNN(RNNNeuralNet * model, const NeuralNetTrainParams * params, const NeuralNetTrainPair ** pairs, const uint32 * pairInputCount, uint64 datasize){
    ASSERT(params->lossFunction == NeuralNetTrainLossFunctionType_QuadError);
    PUSHI;
    float32 alfa = params->learningRate;
    vN nodeDErrors;
    nodeDErrors.size = model->W.width;
    nodeDErrors.v = &PUSHA(float32, nodeDErrors.size);
    matNM * pkDB[2];
    for(int db = 0; db < 2; db++){
        pkDB[db] = &PUSHA(matNM, model->W.width);
        for(int i = 0; i < model->W.width; i++){
            pkDB[db][i].width = model->W.width;
            pkDB[db][i].height = model->W.height;
            pkDB[db][i].c = &PUSHA(float32, model->W.height * model->W.width);
            
        }
    }
    int derivationIndex = 1;
    vN nodeArgs;
    nodeArgs.size = model->W.width;
    nodeArgs.v = &PUSHA(float32, nodeArgs.size);
    
    
    for(uint32 samplesIndex = 0; samplesIndex < datasize; samplesIndex++){
        PUSHI;
        //Pkij = 0 <=> t = t0;
        for(int db = 0; db < 2; db++){
            for(int i = 0; i < model->W.width; i++){
                for(int j = 0; j < model->W.height * model->W.width; j++){
                    pkDB[db][i].c[j] = 0;
                }
            }
        }
        
        for(uint32 inputIndex = 0; inputIndex < pairInputCount[samplesIndex]; inputIndex++){
            matNM * currentP = pkDB[derivationIndex];
            matNM * previousP = pkDB[(derivationIndex + 1)%2];
            
            //prediction part
            vN matrixInput;
            matrixInput.size = model->W.height;
            matrixInput.v = &PUSHA(float32, matrixInput.size);
            
            const vN * input = &pairs[samplesIndex][inputIndex].input;
            
            for(int i = 0; i < model->outputs.size; i++){
                matrixInput.v[i] = model->outputs.v[i];
            }
            for(int i = 0; i < input->size; i++){
                matrixInput.v[model->outputs.size + i] = input->v[i];
            }
            matrixInput.v[matrixInput.size-1] = 1;
            
            ASSERT(input->size == model->W.height - model->W.width - 1);
            
            
            mul(&matrixInput, &model->W, &nodeArgs);
            for(int i = 0; i < model->outputs.size; i++){
                switch(model->activationFunction){
                    case NeuralNetActivationFunctionType_Sigmoid:
                    model->outputs.v[i] = sigmoidActivationFunction(nodeArgs.v[i]);
                    break;
                    case NeuralNetActivationFunctionType_Identity:
                    model->outputs.v[i] = nodeArgs.v[i];
                    break;
                    default:
                    ASSERT(!"FUCK");
                    break;
                }
            }
            
            //realtime learning part. indexation according to the paper (A learning algorithm for continually running fully recurrent Neural Networks [R. J. Williams, D. Zipser]
            
            
            //determine nodes error derivations ~Ek (using custom loss func)
            for(uint32 k = 0; k < model->W.width; k++){
                if(pairs[samplesIndex][inputIndex].matters[k]){
                    nodeDErrors.v[k] = -2 * (pairs[samplesIndex][inputIndex].result.v[k] - model->outputs.v[k]);
                }else{
                    nodeDErrors.v[k] = 0;
                }
            }
            
            //determine nodes derivations Pkij
            for(uint32 k = 0; k < model->W.width; k++){ //the node which all weights are calculated for
                float32 newDerivation = 1;
                switch(model->activationFunction){
                    case NeuralNetActivationFunctionType_Sigmoid:{
                        float32 val = sigmoidActivationFunction(nodeArgs.v[k]);
                        newDerivation = val * (1-val);
                    }break;
                    case NeuralNetActivationFunctionType_Identity:
                    
                    break;
                    default:
                    ASSERT(!"FUCK");
                    break;
                }
                
                for(int i = 0; i < model->W.width; i++){ //the target weight node 
                    for(int j = 0; j < model->W.height; j++){ //the source weight node (can be input weight or bias weight)
                        float32 sum = 0;
                        for(int l = 0; l < model->W.width; l++){
                            sum += model->W.c[k* model->W.height + l] * previousP[l].c[i*model->W.height + j];
                        }
                        
                        currentP[k].c[i * model->W.height + j] = newDerivation*(sum + KRONECKER(i, k) * matrixInput.v[j]);
                    }
                }
            }
            
            //sub the dW
            for(uint32 nodeIndex = 0; nodeIndex < model->W.width; nodeIndex++){
                for(uint32 weightIndex = 0; weightIndex < model->W.height; weightIndex++){
                    float32 sum = 0;
                    for(uint32 k = 0; k < model->W.width; k++){
                        sum += currentP[k].c[nodeIndex*model->W.height + weightIndex] * nodeDErrors.v[k];
                    }
                    model->W.c[nodeIndex*model->W.height + weightIndex] -= alfa * sum;
                }
            }
            
            derivationIndex = (derivationIndex + 1) % 2;
            POP;
        }
        resetMemoryRNNNeuralNet(model);
        POPI;
    }
    
    POPI;
}

void trainMLP(MLPNeuralNet * model, const NeuralNetTrainParams * parameters, const NeuralNetTrainPair * data, const uint64 datasize){
    for(uint64 dataIndex = 0; dataIndex < datasize; dataIndex++){
        
        PUSHI;
        
        vN * layerArgs = &PUSHA(vN, model->hiddenLayersAmount);
        vN * layerOuts = &PUSHA(vN, model->hiddenLayersAmount+1);
        vN * layerOutput = &layerOuts[0];
        layerOutput->size = data[dataIndex].input.size+1;
        layerOutput->v = &PUSHA(float32, layerOutput->size);
        for(int i = 0; i < data[dataIndex].input.size; i++){
            layerOutput->v[i] = data[dataIndex].input.v[i];
        }
        layerOutput->v[data[dataIndex].input.size] = 1;
        
        
        for(int hiddenLayerIndex = 0; hiddenLayerIndex < model->hiddenLayersAmount; hiddenLayerIndex++){
            NeuralHiddenLayer * layer = &model->hiddenLayers[hiddenLayerIndex];
            
            vN * layerArg = &layerArgs[hiddenLayerIndex]; 
            layerArg->size = layer->W.width;
            layerArg->v = &PUSHA(float32, layerArg->size);
            
            mul(layerOutput, &layer->W, layerArg);
            
            layerOutput = &layerOuts[hiddenLayerIndex+1];
            layerOutput->size = layer->W.width+1;
            layerOutput->v = &PUSHA(float32, layerOutput->size);
            
            
            switch(layer->activationFunction){
                case NeuralNetActivationFunctionType_Sigmoid:
                for(int i = 0; i < layerOutput->size-1; i++){
                    layerOutput->v[i] = sigmoidActivationFunction(layerArg->v[i]);
                }
                break;
                case NeuralNetActivationFunctionType_Identity:
                for(int i = 0; i < layerOutput->size-1; i++){
                    layerOutput->v[i] = layerArg->v[i];
                }
                break;
                default:
                ASSERT(!"FUCK");
                break;
            }
            layerOutput->v[layerOutput->size-1] = 1;
            
        }
        //no treshold
        layerOutput->v[layerOutput->size-1] = 0;
        
        
        ///learning part
        ASSERT(data[dataIndex].result.size == layerOutput->size-1);
        
        switch(parameters->lossFunction){
            case NeuralNetTrainLossFunctionType_QuadError:{
                float32 alfa = parameters->learningRate;
                
                vN * layerDerivations = &PUSHA(vN, model->hiddenLayersAmount);
                vN * prevDerivations = NULL;
                vN * currentDerivations = NULL;
                NeuralHiddenLayer layerCopies[2];
                uint8 layerCopyIndex = 1;
                NeuralHiddenLayer * layer = NULL;
                
                for(int hiddenLayerIndex = model->hiddenLayersAmount-1; hiddenLayerIndex >= 0 ; hiddenLayerIndex--){
                    NeuralHiddenLayer * prevLayer = &layerCopies[layerCopyIndex];
                    
                    layer = &model->hiddenLayers[hiddenLayerIndex];
                    layerCopies[(layerCopyIndex + 1)%2] = *layer;
                    layerCopies[(layerCopyIndex + 1)%2].W.c = &PUSHA(float32, layerCopies[(layerCopyIndex + 1)%2].W.width * layerCopies[(layerCopyIndex + 1)%2].W.height);
                    for(int i = 0; i < layerCopies[(layerCopyIndex + 1)%2].W.width * layerCopies[(layerCopyIndex + 1)%2].W.height; i++){
                        layerCopies[(layerCopyIndex + 1)%2].W.c[i] = layer->W.c[i];
                    }
                    
                    vN * layerArg = &layerArgs[hiddenLayerIndex]; 
                    
                    
                    prevDerivations = currentDerivations;
                    currentDerivations = &layerDerivations[hiddenLayerIndex];
                    currentDerivations->size = layer->W.width;
                    currentDerivations->v = &PUSHA(float32, currentDerivations->size);
                    
                    
                    
                    for(int nodeIndex = 0; nodeIndex < layer->W.width; nodeIndex++){
                        
                        if(prevDerivations == NULL && !data[dataIndex].matters[nodeIndex]){
                            currentDerivations->v[nodeIndex] = 0;
                            continue;
                        }
                        
                        //dNode
                        float32 derivationValue = 0;
                        if(prevDerivations != NULL){
                            for(int prevNodeIndex = 0; prevNodeIndex < prevLayer->W.width; prevNodeIndex++){
                                //w * prev
                                derivationValue += (prevLayer->W.c[prevNodeIndex * prevLayer->W.height + nodeIndex] * prevDerivations->v[prevNodeIndex]);
                            }
                        }else{
                            //out layer
                            derivationValue = -2*(data[dataIndex].result.v[nodeIndex] - layerOutput->v[nodeIndex]);
                        }
                        //derivation of squash
                        switch(layer->activationFunction){
                            case NeuralNetActivationFunctionType_Identity:
                            break;
                            case NeuralNetActivationFunctionType_Sigmoid:{
                                float32 val = sigmoidActivationFunction(layerArgs[hiddenLayerIndex].v[nodeIndex]);
                                derivationValue *= val * (1-val);
                            }
                            break;
                            default:
                            ASSERT(!"FUCK");
                            break;
                        }
                        
                        currentDerivations->v[nodeIndex] = derivationValue;
                        
                        //dNode * outs is the final derivation
                        for(int weightIndex = 0; weightIndex < layer->W.height; weightIndex++){
                            layer->W.c[nodeIndex * layer->W.height + weightIndex] -= alfa * derivationValue * layerOuts[hiddenLayerIndex].v[weightIndex];
                        }
                        
                        
                    }
                    
                    layerCopyIndex = (layerCopyIndex + 1) % 2;
                    
                    
                }
                
                
                
                
            }
            break;
            default:
            ASSERT(!"FUCK");
            break;
        }
        
        
        
        
        
        POPI;
        
        
    }
    
}


/////////////// saving and loading

bool loadMLPNeuralNet(const FileContents * contents, MLPNeuralNet * result){
    char * line = &PUSHA(char, 1024);
    result->hiddenLayersAmount = 0;
    uint32 scanOffset = 0;
    uint32 rowsLeft = 0;
    int32 layerIndex = -1;
    while(scanOffset < contents->size && sscanf(contents->contents + scanOffset, "%1024%1023[^\n]", line) == 1){
        if(result->hiddenLayersAmount == 0){
            ASSERT(sscanf(line, "%5%5hu", &result->hiddenLayersAmount) == 1);
            result->hiddenLayers = &PUSHA(NeuralHiddenLayer, result->hiddenLayersAmount);
        }else{
            if(rowsLeft == 0){
                layerIndex++;
                char actFunc = '0';
                ASSERT(sscanf(line, "%1024%5hu %5hu %c", &result->hiddenLayers[layerIndex].W.width, &result->hiddenLayers[layerIndex].W.height, &actFunc) == 3);
                rowsLeft = result->hiddenLayers[layerIndex].W.height;
                result->hiddenLayers[layerIndex].W.c = &PUSHA(float32, result->hiddenLayers[layerIndex].W.height * result->hiddenLayers[layerIndex].W.width);
                
                if(actFunc == 's'){
                    result->hiddenLayers[layerIndex].activationFunction = NeuralNetActivationFunctionType_Sigmoid;
                }else if(actFunc == 'i'){
                    result->hiddenLayers[layerIndex].activationFunction = NeuralNetActivationFunctionType_Identity;
                }else{
                    INV;
                }
            }else{
                uint32 lineOffset = 0;
                for(int i = 0; i < result->hiddenLayers[layerIndex].W.width; i++){
                    ASSERT(sscanf(line + lineOffset, "%32%f", &result->hiddenLayers[layerIndex].W.c[result->hiddenLayers[layerIndex].W.height*i + (result->hiddenLayers[layerIndex].W.height - rowsLeft)]) == 1);
                    lineOffset += strfind(" ", line+lineOffset) + 1;
                }
                rowsLeft--;
            }
        }
        scanOffset+= strlen(line) + 1;// \n
    }
    //POP; on random memory management, get rid of line mem
    return true;
}

bool saveMLPNeuralNet(const MLPNeuralNet * source, FileContents * target){
    uint32 maxLen = 70 + 20*source->hiddenLayersAmount;
    for(int i = 0; i < source->hiddenLayersAmount; i++){
        maxLen += 33 * source->hiddenLayers[i].W.width * source->hiddenLayers[i].W.height;
    }
    target->size = 0;
    target->contents = &PUSHA(char, maxLen);
    ASSERT(sprintf(target->contents, "%d\n", (int32)source->hiddenLayersAmount) == 1);
    target->size += strlen(target->contents + target->size);
    for(int i = 0; i < source->hiddenLayersAmount; i++){
        NeuralHiddenLayer * layer = &source->hiddenLayers[i];
        char c = '0';
        switch(layer->activationFunction){
            case NeuralNetActivationFunctionType_Sigmoid:
            c = 's';
            break;
            case NeuralNetActivationFunctionType_Identity:
            c = 'i';
            break;
            default:
            INV;
            break;
        }
        ASSERT(sprintf(target->contents+target->size, "%d %d %c\n", (int32)layer->W.width, (int32)layer->W.height, c) == 3);
        target->size += strlen(target->contents + target->size);
        for(int row = 0; row < layer->W.height; row++){
            for(int col = 0; col < layer->W.width; col++){
                char divider[2] = " ";
                if(col == layer->W.width-1){
                    divider[0] = '\n';
                }
                ASSERT(sprintf(target->contents+target->size, "%,9f%c", layer->W.c[col*layer->W.height + row], divider[0]) == 2);
                
                target->size += strfind(divider, target->contents + target->size) + 1;
            }
        }
    }
    
    return true;
}

bool loadRNNNeuralNet(const FileContents * contents, RNNNeuralNet * result){
    char * line = &PUSHA(char, 1024);
    uint32 scanOffset = 0;
    uint32 rowsLeft = 0;
    int32 layerIndex = -1;
    while(scanOffset < contents->size && sscanf(contents->contents + scanOffset, "%1024%1023[^\n]", line) == 1){
        if(rowsLeft == 0){
            char actFunc = '0';
            ASSERT(sscanf(line, "%1024%5hu %5hu %c", &result->W.width, &result->W.height, &actFunc) == 3);
            rowsLeft = result->W.height;
            result->W.c = &PUSHA(float32, result->W.height * result->W.width);
            result->outputs.size = result->W.width;
            result->outputs.v = &PUSHA(float32, result->outputs.size);
            if(actFunc == 's'){
                result->activationFunction = NeuralNetActivationFunctionType_Sigmoid;
            }else if(actFunc == 'i'){
                result->activationFunction = NeuralNetActivationFunctionType_Identity;
            }else{
                INV;
            }
        }else{
            uint32 lineOffset = 0;
            for(int i = 0; i < result->W.width; i++){
                ASSERT(sscanf(line + lineOffset, "%32%f", &result->W.c[result->W.height*i + (result->W.height - rowsLeft)]) == 1);
                lineOffset += strfind(" ", line+lineOffset) + 1;
            }
            rowsLeft--;
        }
        
        scanOffset+= strlen(line) + 1;// \n
    }
    //POP; on random memory management, get rid of line mem
    return true;
}


bool saveRNNNeuralNet(const RNNNeuralNet * source, FileContents * target){
    uint32 maxLen = 20 + 33 * source->W.width * source->W.height;
    target->size = 0;
    target->contents = &PUSHA(char, maxLen);
    target->size += strlen(target->contents + target->size);
    
    char c = '0';
    switch(source->activationFunction){
        case NeuralNetActivationFunctionType_Sigmoid:
        c = 's';
        break;
        case NeuralNetActivationFunctionType_Identity:
        c = 'i';
        break;
        default:
        INV;
        break;
    }
    ASSERT(sprintf(target->contents+target->size, "%d %d %c\n", (int32)source->W.width, (int32)source->W.height, c) == 3);
    target->size += strlen(target->contents + target->size);
    for(int row = 0; row < source->W.height; row++){
        for(int col = 0; col < source->W.width; col++){
            char divider[2] = " ";
            if(col == source->W.width-1){
                divider[0] = '\n';
            }
            ASSERT(sprintf(target->contents+target->size, "%,9f%c", source->W.c[col*source->W.height + row], divider[0]) == 2);
            
            target->size += strfind(divider, target->contents + target->size) + 1;
        }
    }
    
    
    return true;
}








#endif