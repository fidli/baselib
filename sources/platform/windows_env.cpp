#pragma once

i32 platformGetDpi(){
    i32 r = GetDpiForWindow(window);
    if(r == 0){
        return 96;
    }
    return r;
}
