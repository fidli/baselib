#ifndef WINDOWS_IMGUI_CPP
#define WINDOWS_IMGUI_CPP

#include "util_imgui.cpp"


bool guiHandleInputWin(UINT message, WPARAM wParam, LPARAM lParam){
    if(!isInit){
        return false;
    }
    bool inputHandled = false;
    switch(message)
    {
        case WM_MOUSEMOVE:{
            guiInput.mouse.y = (int16) (lParam >> 16);
            guiInput.mouse.x = (int16) (lParam);
            return false;
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
            bool ctrlDown = GetKeyState(0x11) & (1 << 15);
            bool shiftDown = GetKeyState(0x10) & (1 << 15);
            //first hit
            if(~lParam & (1<<30)){
                switch (wParam){
                    case 0x09:{// tab
                        if(shiftDown){
                            guiSelectPreviousInput();
                        }else{
                            guiSelectNextInput();
                        }
                        inputHandled = true;
                        return inputHandled;
                    }break;
                    case 0x1B:{// escape
                        if(guiAnyInputSelected() || guiValid(guiContext->activeInput)){
                            if(guiContext->caretPositioning){
                                guiCancelCaretPositioning();
                            } else{
                                guiDeselectInput();
                            }
                            inputHandled = true;
                        } else if(guiValid(guiContext->activeDropdown)){
                            guiInvalidate(&guiContext->activeDropdown);
                            inputHandled = true;
                        }else if(guiAnyPopup()){
                            guiClosePopup(NULL);
                            inputHandled = true;
                        }
                    }break;
                }
                if(guiAnyInputSelected()){
                    switch (wParam){
                        case 0x0D:{// enter
                            guiActivateSelection();
                            inputHandled = true;
                        }break;
                    }
                } 
                if(!inputHandled && guiValid(guiContext->activeInput)){
                    switch (wParam){
                        case 0x28: // down arrow
                        case 0x26:{ // up arrow
                        }break;
                        case 0x25:{// left arrow
                            if(!shiftDown){
                                guiCancelCaretPositioning();
                                guiContext->caretPos = MIN(guiContext->caretPos, guiContext->caretPos + guiContext->caretWidth);
                                if(guiContext->caretPos > 0){
                                    guiContext->caretPos--;
                                }
                                guiResetCaretVisually();
                            }else{
                                if(guiContext->caretPos + guiContext->caretWidth > 0){
                                    guiContext->caretWidth--;
                                }
                            }
                            inputHandled = true;
                        }break;
                        case 0x27:{ //right arrow
                            nint textlen = strlen_s(guiContext->inputText, guiContext->inputMaxlen);
                            if(!shiftDown){
                                guiCancelCaretPositioning();
                                guiContext->caretPos = MAX(guiContext->caretPos, guiContext->caretPos + guiContext->caretWidth);
                                if(guiContext->caretPos < textlen){
                                    guiContext->caretPos++;
                                }
                                guiResetCaretVisually();
                            }else{
                                if(guiContext->caretPos + guiContext->caretWidth < textlen){
                                    guiContext->caretWidth++;
                                }
                            }
                            inputHandled = true;
                        }break;
                        case 0x24:{ // HOME
                            if(!shiftDown){
                                guiCancelCaretPositioning();
                                guiContext->caretPos = 0;
                                guiResetCaretVisually();
                            }else{
                                guiContext->caretWidth = -guiContext->caretPos;
                            }
                            inputHandled = true;
                        }break;
                        case 0x23:{ // END
                            nint textlen = strlen_s(guiContext->inputText, guiContext->inputMaxlen);
                            if(!shiftDown){
                                guiCancelCaretPositioning();
                                guiContext->caretPos = textlen;
                                guiResetCaretVisually();
                            }else{
                                guiContext->caretWidth = textlen - guiContext->caretPos;
                            }
                            inputHandled = true;
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
                            inputHandled = true;
                        }break;
                        case 0x08:{ //backspace
                            if(guiContext->caretWidth == 0){
                                guiDeleteInputCharacters(guiContext->caretPos-1, guiContext->caretPos);
                            }else if(guiContext->caretWidth){
                                guiDeleteInputCharacters(guiContext->caretPos, guiContext->caretPos + guiContext->caretWidth);
                            }
                            guiResetCaretVisually();
                            inputHandled = true;
                        }break;
                        case 0x2E:{ //delete
                            if(guiContext->caretWidth == 0){
                                guiDeleteInputCharacters(guiContext->caretPos, guiContext->caretPos+1);
                            }else if(guiContext->caretWidth){
                                guiDeleteInputCharacters(guiContext->caretPos, guiContext->caretPos+guiContext->caretWidth);
                            }
                            guiResetCaretVisually();
                            inputHandled = true;
                        }break;
                        default:{
                            if(guiContext->caretWidth != 0){
                                guiDeleteInputCharacters(guiContext->caretPos, guiContext->caretPos + guiContext->caretWidth);
                            }
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
                            if(shiftDown){//shift
                                capitals = !capitals;
                            }
                            if(!capitals){
                                //TODO(AK): Other chars
                                if(c >= (char)0x41 && c <= (char)0x5A){//is A-Z
                                    c += (char)0x20;
                                }
                            }
                            bool isValid = c >= 32;
                            if(isValid){
                                guiInputCharacters(&c, 1);
                            }
                        }break;
                    }
                    inputHandled = true;
                }
            };
        }break;
    }
    return inputHandled || guiClick();
}

#endif
