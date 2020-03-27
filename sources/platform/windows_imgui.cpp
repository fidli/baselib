#ifndef WINDOWS_IMGUI_CPP
#define WINDOWS_IMGUI_CPP

#include "util_imgui.cpp"


bool guiHandleInputWin(UINT message, WPARAM wParam, LPARAM lParam){
    bool inputHandled = false;    
    switch(message)
    {
        case WM_MOUSEMOVE:{
            guiInput.mouse.y = (int16) (lParam >> 16);
            guiInput.mouse.x = (int16) (lParam);
        }break;
        case WM_LBUTTONUP:{
            guiInput.mouse.buttons.leftUp = true;
            inputHandled = guiAnyPopup();
        }break;
        case WM_LBUTTONDOWN:{
            guiInput.mouse.buttons.leftDown = true;
            inputHandled = guiAnyPopup();
            // NOTE(fidli): any input gets re-selected potentially
            guiDeselectInput();
        }break;
        case WM_KEYDOWN:{
            //first hit
            if(~lParam & (1<<30)){
                switch (wParam){
                    case 0x09:{// tab
                        if(GetKeyState(0x10) & (1 << 15)){//shift
                            guiSelectPreviousInput();
                        }else{
                            guiSelectNextInput();
                        }
                        inputHandled = true;
                        return inputHandled;
                    }break;
                }
                if(guiAnyInputSelected()){
                    switch (wParam){
                        case 0x1B:{// escape
                            guiDeselectInput();
                            inputHandled = true;
                        }break;
                        case 0x0D:{// enter
                            guiActivateSelection();
                            inputHandled = true;
                        }break;
                        
                    }
                } else if(guiAnyPopup()){
                    switch (wParam){
                        case 0x1B:{// escape
                            guiClosePopup(NULL);
                            inputHandled = true;
                        }break;
                    }
                }
                if(guiValid(guiContext->activeInput)){
                    switch (wParam){
                        case 0x1B: {// escape
                            guiDeselectInput();
                            inputHandled = true;
                        }break;
                        case 0x28: // down arrow
                        case 0x26:{ // up arrow
                        }break;
                        case 0x25:{// left arrow
                            if(guiContext->caretPos > 0) guiContext->caretPos--;
                        }break;
                        case 0x27:{ //right arrow
                            guiContext->caretPos++;
                        }break;
                        case 0x10:{ //shift
                            //is handled by get key state
                        }break;
                        case 0x14:{ //caps lock
                            //is handled by get key state
                        }break;
                        case 0x0D:{
                            //enter
                            if(guiAnyInputSelected()){ // NOTE(fidli): this must be the input
                                guiSelectNextInput();
                            }else{
                                guiDeselectInput();
                            }
                        }break;
                        case 0x08:{ //backspace
                            //assuming this fits
                            int32 textlen = strlen(guiContext->inputData);
                            for(int32 i = guiContext->caretPos-1; i < textlen; i++){
                                guiContext->inputData[i] = guiContext->inputData[i+1];
                            } 
                            guiContext->inputData[textlen-1] = 0;
                            if(guiContext->caretPos > 0){
                                guiContext->caretPos--;
                            }
                        }break;
                        case 0x2E:{ //delete
                            //assuming this fits
                            int32 textlen = strlen(guiContext->inputData);
                            for(int32 i = guiContext->caretPos; i < textlen; i++){
                                guiContext->inputData[i] = guiContext->inputData[i+1];
                            } 
                            guiContext->inputData[textlen-1] = 0;
                        }break;
                        default:{
                            char c = wParam;//assuming ascii character
                            if(c == (char)0x6D || c == (char)0xBD){ //minus
                                c = '-';
                            }
                            if(c >= (char)0x60 && c <= (char)0x69){//numpad
                                c -= (char)0x30;
                            }
                            bool capitals = false;
                            if(GetKeyState(0x14) & 1){//caps lock
                                capitals = true;
                            }
                            if(GetKeyState(0x10) & (1 << 15)){//shift
                                capitals = !capitals;
                            }
                            if(!capitals){
                                //TODO(AK): Other chars
                                if(c >= (char)0x41 && c <= (char)0x5A){//is A-Z
                                    c += (char)0x20;
                                }
                            }
                            
                            bool isValid = true;
                            if(guiContext->inputCharlist != NULL){
                                //start parse format
                                const char * format = guiContext->inputCharlist;
                                bool inverted = false;
                                int32 formatIndex = 0;
                                if(format[formatIndex] == '^'){
                                    inverted = true;
                                    formatIndex++;
                                }
                                char digitRangeLow = 0;
                                char digitRangeHigh = 0;
                                char smallLetterRangeLow = 0;
                                char smallLetterRangeHigh = 0;
                                char capitalLetterRangeLow = 0;
                                char capitalLetterRangeHigh = 0;
                                const char* charlist[4] = {};
                                uint8 charlistLengths[4] = {};
                                uint8 charlistCount = 0;
                                
                                uint8 charlistLen = 0;
                                for(;format[formatIndex] != 0; formatIndex++){
                                    if(format[formatIndex+1] == '-'){
                                        if(charlistLen > 0){
                                            charlistLengths[charlistCount] = charlistLen;
                                            charlistLen = 0;
                                        }
                                        
                                        if(format[formatIndex] >= 'A' && format[formatIndex] <= 'Z'){
                                            capitalLetterRangeLow = format[formatIndex];                         
                                        }else if(format[formatIndex] >= 'a' && format[formatIndex] <= 'z'){
                                            smallLetterRangeLow = format[formatIndex];                            
                                        }else if(format[formatIndex] >= '0' && format[formatIndex] <= '9'){
                                            digitRangeLow = format[formatIndex];
                                        }else{
                                            INV; //implement me maybe? makes sense? i havent been in these depths for long
                                        }
                                        
                                        formatIndex += 2;
                                        
                                        if(format[formatIndex] >= 'A' && format[formatIndex] <= 'Z'){
                                            capitalLetterRangeHigh = format[formatIndex];                    
                                            ASSERT(capitalLetterRangeHigh > capitalLetterRangeLow);
                                        }else if(format[formatIndex] >= 'a' && format[formatIndex] <= 'z'){
                                            smallLetterRangeHigh = format[formatIndex];
                                            ASSERT(capitalLetterRangeHigh > capitalLetterRangeLow);
                                        }else if(format[formatIndex] >= '0' && format[formatIndex] <= '9'){
                                            digitRangeHigh = format[formatIndex];
                                            ASSERT(digitRangeHigh > digitRangeLow);
                                        }else{
                                            INV; //implement me maybe? makes sense?
                                        }
                                        
                                    }else{
                                        
                                        if(charlistLen == 0){
                                            charlistCount++;
                                            charlist[charlistCount-1] = &format[formatIndex];
                                        }
                                        charlistLengths[charlistCount-1]++;
                                        charlistLen++;
                                        
                                    }
                                }
                                //end parse format
                                isValid = false;
                                //start validate char
                                do{
                                    if(digitRangeLow != '\0'){
                                        if(c >= digitRangeLow && c <= digitRangeHigh && !inverted){
                                            isValid = true;
                                            break;
                                        }else if((c < digitRangeLow && c > digitRangeHigh) && inverted){
                                            isValid = true;
                                            break;
                                        }
                                    }
                                    if(capitalLetterRangeLow != '\0'){
                                        if(c >= capitalLetterRangeLow && c <= capitalLetterRangeHigh && !inverted){
                                            isValid = true;
                                            break;
                                        }else if((c < capitalLetterRangeLow && c > capitalLetterRangeHigh) && inverted){
                                            isValid = true;
                                            break;
                                        }
                                    }
                                    if(smallLetterRangeLow != '\0'){
                                        if(c >= smallLetterRangeLow && c <= smallLetterRangeHigh && !inverted){
                                            isValid = true;
                                            break;
                                        }else if((c < smallLetterRangeLow || c > smallLetterRangeHigh) && inverted){
                                            isValid = true;
                                            break;
                                        }                        
                                    }
                                    
                                    bool found = false;
                                    for(int charlistIndex = 0; charlistIndex < charlistCount; charlistIndex++){
                                        for(int charIndex = 0; charIndex < charlistLengths[charlistIndex]; charIndex++){
                                            if(c == charlist[charlistIndex][charIndex]){
                                                isValid = true;
                                                break;
                                            }
                                            
                                        }
                                        if(found){
                                            break;
                                        }
                                    }
                                    
                                    
                                    if(!inverted && found){
                                        isValid = true;
                                        break;
                                    }else if(inverted && !found){
                                        isValid = true;
                                        break;
                                    }
                                    
                                }while(false);
                                //end validate char
                            }
                            if(isValid){
                                //assuming this fits
                                int32 textlen = strlen(guiContext->inputData);
                                for(int32 i = textlen; i > guiContext->caretPos; i--){
                                    guiContext->inputData[i] = guiContext->inputData[i-1];
                                } 
                                guiContext->inputData[guiContext->caretPos] = c;
                                guiContext->caretPos++;
                            }
                        }break;
                    }
                    inputHandled = true;
                }
            };
        }break;
    }
    return inputHandled;
}

#endif
