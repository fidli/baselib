#pragma once
#include <GL/glx.h>

#include "util_imgui.cpp"
// https://www.x.org/releases/X11R7.7/doc/libX11/libX11/libX11.html#Event_Types
// https://tronche.com/gui/x/xlib/events/keyboard-pointer/keyboard-pointer.html

// TODO(fidli): return system value
float GetDoubleClickTime(){
    return 0.1f;
}

bool guiHandleInputLinux(XEvent * event){
    if(!isInit){
        return false;
    }
    bool inputHandled = false;
    switch(event->type)
    {
        case MotionNotify:{
            guiInput.mouse.y = event->xmotion.y;
            guiInput.mouse.x = event->xmotion.x;
            return false;
        }break;
        case ButtonRelease:{
            if(event->xbutton.button == Button1){
                guiInput.mouse.buttons.leftUp = true;
                guiInput.mouse.leftHold = false;
            }
            return false;
        }break;
        case ButtonPress:{
            if(event->xbutton.button == Button1){
                guiInput.mouse.lastClickTime = getProcessCurrentTime();
                guiInput.mouse.buttons.leftDown = true;
                guiInput.mouse.leftHold = true;
                if(CAST(u32, 1000*(getProcessCurrentTime() - guiInput.mouse.lastClickTime)) < GetDoubleClickTime()){
                    guiInput.mouse.buttons.leftDoubleClick = true;
                    guiInput.mouse.lastDoubleClickTime = getProcessCurrentTime();
                }
                if(CAST(u32, 1000*(getProcessCurrentTime() - guiInput.mouse.lastDoubleClickTime)) < GetDoubleClickTime()){
                    guiInput.mouse.buttons.leftTripleClick = true;
                }
                // NOTE(fidli): any input gets re-selected potentially
                guiDeselectInput();
            }
            return false;
        }break;
        case KeyPress:{
            bool ctrlDown = event->xkey.state & ControlMask;
            bool shiftDown = event->xkey.state & ShiftMask;
            switch (event->xkey.keycode){
                case XK_Tab:{// tab
                    if(shiftDown){
                        guiSelectPreviousInput();
                    }else{
                        guiSelectNextInput();
                    }
                    inputHandled = true;
                    return inputHandled;
                }break;
                case XK_Escape:{// escape
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
                switch (event->xkey.keycode){
                    case XK_Return:{// enter
                        guiActivateSelection();
                        inputHandled = true;
                    }break;
                }
            } 
            if(!inputHandled && guiValid(guiContext->activeInput) && guiContext->inputText != NULL){
                switch (event->xkey.keycode){
                    case XK_Down: // down arrow
                    case XK_Up:{ // up arrow
                    }break;
                    case XK_Left:{// left arrow
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
                    case XK_Right:{ //right arrow
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
                    case XK_Home:{ // HOME
                        if(!shiftDown){
                            guiCancelCaretPositioning();
                            guiContext->caretPos = 0;
                            guiResetCaretVisually();
                        }else{
                            guiContext->caretWidth = -guiContext->caretPos;
                        }
                        inputHandled = true;
                    }break;
                    case XK_End:{ // END
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
                    case XK_Control_L: // ctrl
                    case XK_Shift_L: // shift
                    case XK_Caps_Lock:{ //caps lock
                        //is handled by get key state
                    }break;
                    case XK_Return:{
                        //enter
                        if(guiAnyInputSelected()){ // NOTE(fidli): this must be the input
                            guiSelectNextInput();
                        }else{
                            guiDeselectInput();
                        }
                        inputHandled = true;
                    }break;
                    case XK_BackSpace:{ //backspace
                        if(guiContext->caretWidth == 0){
                            guiDeleteInputCharacters(guiContext->caretPos-1, guiContext->caretPos);
                        }else if(guiContext->caretWidth){
                            guiDeleteInputCharacters(guiContext->caretPos, guiContext->caretPos + guiContext->caretWidth);
                        }
                        guiResetCaretVisually();
                        inputHandled = true;
                    }break;
                    case XK_Delete:{ //delete
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
                        if(event->xkey.keycode == XK_a && ctrlDown){
                            guiSelectWholeInput();
                            guiCancelCaretPositioning();
                            nint textlen = strnlen(guiContext->inputText, guiContext->inputMaxlen);
                            guiContext->caretPos = 0;
                            guiContext->caretWidth = textlen; 
                            inputHandled = true;
                            break;
                        }
                        // ctrl-c OR ctrl-x
                        // TODO(fidli): clipboard 
                        // ctrl-v
                        
                        if(guiContext->caretWidth != 0){
                            guiDeleteInputCharacters(guiContext->caretPos, guiContext->caretPos + guiContext->caretWidth);
                        }
                        char c = event->xkey.keycode;//assuming ascii character
                        // '.'
                        // ','
                        // minus
                        // numpad 
                        bool capitals = false;
                        if(event->xkey.state & LockMask){//caps lock
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
