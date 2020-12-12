#ifndef WINDOWS_IMGUI_CPP
#define WINDOWS_IMGUI_CPP

extern HWND window;

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
            guiInput.mouse.leftHold = false;
            return false;
        }break;
        case WM_LBUTTONDBLCLK:
            guiInput.mouse.buttons.leftDoubleClick = true;
            guiInput.mouse.lastDoubleClickTime = getProcessCurrentTime();
            return false;
        case WM_LBUTTONDOWN:{
            guiInput.mouse.buttons.leftDown = true;
            guiInput.mouse.leftHold = true;
            if(message == WM_LBUTTONDOWN && CAST(uint32, 1000*(getProcessCurrentTime() - guiInput.mouse.lastDoubleClickTime)) < GetDoubleClickTime()){
                guiInput.mouse.buttons.leftTripleClick = true;
            }
            // NOTE(fidli): any input gets re-selected potentially
            guiDeselectInput();
            return false;
        }break;
        case WM_KEYDOWN:{
            bool ctrlDown = GetKeyState(0x11) & (1 << 15);
            bool shiftDown = GetKeyState(0x10) & (1 << 15);
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
            if(!inputHandled && guiValid(guiContext->activeInput) && guiContext->inputText != NULL){
                switch (wParam){
                    case 0x28: // down arrow
                    case 0x26:{ // up arrow
                    }break;
                    case 0x25:{// left arrow
                        if(!shiftDown){
                            guiCancelCaretPositioning();
                            if(ctrlDown){
                                guiJumpToPrevWord();
                            }else{
                                guiContext->caretPos = MIN(guiContext->caretPos, guiContext->caretPos + guiContext->caretWidth);
                                if(guiContext->caretPos > 0){
                                    guiContext->caretPos--;
                                }
                            }
                            guiResetCaretVisually();
                        }else{
                            if(ctrlDown){
                                guiAppendPrevWordToSelection();
                            }else if(guiContext->caretPos + guiContext->caretWidth > 0){
                                guiContext->caretWidth--;
                            }
                        }
                        inputHandled = true;
                    }break;
                    case 0x27:{ //right arrow
                        nint textlen = strnlen(guiContext->inputText, guiContext->inputMaxlen);
                        if(!shiftDown){
                            guiCancelCaretPositioning();
                            if(ctrlDown){
                                guiJumpToNextWord();
                            }else{
                                guiContext->caretPos = MAX(guiContext->caretPos, guiContext->caretPos + guiContext->caretWidth);
                                if(guiContext->caretPos < textlen){
                                    guiContext->caretPos++;
                                }
                                guiResetCaretVisually();
                            }
                        }else{
                            if(ctrlDown){
                                guiAppendNextWordToSelection();
                            }else if(guiContext->caretPos + guiContext->caretWidth < textlen){
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
                        nint textlen = strnlen(guiContext->inputText, guiContext->inputMaxlen);
                        if(!shiftDown){
                            guiCancelCaretPositioning();
                            guiContext->caretPos = textlen;
                            guiResetCaretVisually();
                        }else{
                            guiContext->caretWidth = textlen - guiContext->caretPos;
                        }
                        inputHandled = true;
                    }break;
                    case 0x11: // ctrl
                    case 0x10: // shift
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
                        // ctrl-a
                        if(wParam == 0x41 && ctrlDown){
                            guiSelectWholeInput();
                            guiCancelCaretPositioning();
                            nint textlen = strnlen(guiContext->inputText, guiContext->inputMaxlen);
                            guiContext->caretPos = 0;
                            guiContext->caretWidth = textlen; 
                            inputHandled = true;
                            break;
                        }
                        // ctrl-c OR ctrl-x
                        if((wParam == 0x43 || wParam == 0x58) && ctrlDown){
                            if(OpenClipboard(window)){
                                HGLOBAL contentsHandle = GlobalAlloc(GMEM_MOVEABLE, ABS(guiContext->width) * sizeof(char));
                                void * contents = GlobalLock(contentsHandle);
                                int32 start = MIN(guiContext->caretPos, guiContext->caretPos + guiContext->caretWidth);
                                memcpy(contents, guiContext->inputText + start, ABS(guiContext->caretWidth));
                                CAST(char*, contents)[ABS(guiContext->caretWidth)] = '\0';
                                GlobalUnlock(contentsHandle);
                                EmptyClipboard();
                                SetClipboardData(CF_TEXT, contentsHandle);
                                CloseClipboard();
                                if(wParam == 0x58 && guiContext->caretWidth != 0){
                                    guiDeleteInputCharacters(guiContext->caretPos, guiContext->caretPos+guiContext->caretWidth);
                                }
                            }
                            inputHandled = true;
                            break;
                        }
                        // ctrl-v
                        if((wParam == 0x56) && ctrlDown){
                            if(IsClipboardFormatAvailable(CF_TEXT) && OpenClipboard(window)){
                                HGLOBAL contentsHandle = GetClipboardData(CF_TEXT); 
                                char * data = CAST(char*, GlobalLock(contentsHandle));
                                GlobalUnlock(contentsHandle); 
                                CloseClipboard();
                                if(guiContext->caretWidth != 0){
                                    guiDeleteInputCharacters(guiContext->caretPos, guiContext->caretPos + guiContext->caretWidth);
                                }
                                guiInputCharacters(data, strlen(data));
                            }
                            inputHandled = true;
                            break;
                        }
                        if(guiContext->caretWidth != 0){
                            guiDeleteInputCharacters(guiContext->caretPos, guiContext->caretPos + guiContext->caretWidth);
                        }
                        char c = wParam;//assuming ascii character
                        // '.'
                        if(wParam == 0xBE){
                            c = '.';
                        }
                        // ','
                        if(wParam == 0xBC){
                            c = ',';
                        }
                        
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
            };
        }break;
    }
    return inputHandled || guiClick();
}

#endif
