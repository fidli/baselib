#pragma once

#define DIRECTINPUT_VERSION 0x0800
#include <Dinput.h>

static IDirectInput8 * context = NULL;

enum ControllerObject{
    ControllerObject_Invalid,

    ControllerObject_Button,
    ControllerObject_XAxis,
    ControllerObject_YAxis,
    ControllerObject_ZAxis,

    ControllerObject_RxAxis,
    ControllerObject_RyAxis,
    ControllerObject_RzAxis,

    ControllerObjectCount
};

struct ControllerHandle{
    u32 sequence;
    u8 slotIndex;
};

struct ControllerState{
    struct {
        f32 x;
        f32 y;
    } position; 
};

struct Controller{
    GUID winId;
    bool present;
    u32 sequence;

    LPDIRECTINPUTDEVICE8A dev;
    DIDEVICEOBJECTDATA data[2];
    char name[50];

    struct {
        char name[50];
        ControllerObject type;
        DWORD dwType;
        GUID guidType;
        i32 offset;
        i32 min;
        i32 max;
        i32 center;
        i32 deviation;
        bool want;
        
    } buttons[50];

    i32 buttonsCount;

    ControllerState state;
    

} controllers[20];

u8 controllersCount;

bool gamepadInit(){
    HRESULT hr = DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, CAST(LPVOID*, &context), NULL);
    return hr == DI_OK && context != NULL; 
}

bool gamepadFinalize(){
    context->Release();
    return true;
}

static const char * controllerObjectToStr(ControllerObject type)
{
    switch(type){
        case ControllerObject_Button:
            return "Button";
        case ControllerObject_XAxis:
            return "X Axis";
        case ControllerObject_YAxis:
            return "Y Axis";
        case ControllerObject_ZAxis:
            return "Z Axis";
        case ControllerObject_RxAxis:
            return "Rotation X";
        case ControllerObject_RyAxis:
            return "Rotation Y";
        case ControllerObject_RzAxis:
            return "Rotation Z";
        default:
            return "Unknown";
    }
}


static BOOL DIEnumDeviceObjectsCallback(LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef)
{
    Controller * controller = CAST(Controller*, pvRef);
    strncpy(controller->buttons[controller->buttonsCount].name, lpddoi->tszName, ARRAYSIZE(controller->buttons[controller->buttonsCount].name));
    ControllerObject type = ControllerObject_Invalid;
    if (lpddoi->dwType & DIDFT_PSHBUTTON && lpddoi->guidType == GUID_Button){
        type = ControllerObject_Button;
    } else if (lpddoi->dwType & DIDFT_ABSAXIS){
        if (lpddoi->guidType == GUID_XAxis){
            type = ControllerObject_XAxis;
        }
        if (lpddoi->guidType == GUID_YAxis){
            type = ControllerObject_YAxis;
        }
        if (lpddoi->guidType == GUID_ZAxis){
            type = ControllerObject_ZAxis;
        }
        if (lpddoi->guidType == GUID_RxAxis){
            type = ControllerObject_RxAxis;
        }
        if (lpddoi->guidType == GUID_RyAxis){
            type = ControllerObject_RyAxis;
        }
        if (lpddoi->guidType == GUID_RzAxis){
            type = ControllerObject_RzAxis;
        }
    }
    controller->buttons[controller->buttonsCount].type = type;
    controller->buttons[controller->buttonsCount].dwType = lpddoi->dwType;
    controller->buttons[controller->buttonsCount].guidType = lpddoi->guidType;
    controller->buttonsCount++;
    return DIENUM_CONTINUE;
}

static BOOL syncControllers(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef)
{
    (void) pvRef;
    i16 emptySlotI = controllersCount;
    for (u8 i = 0; i < controllersCount; i++){
        if (controllers[i].winId == lpddi->guidInstance && controllers[i].dev != NULL)
        {
            controllers[i].present = true;
            return DIENUM_CONTINUE;
        }
        if (controllers[i].dev == NULL && !controllers[i].present)
        {
            emptySlotI = i;
        }

    }

    ASSERT(emptySlotI < ARRAYSIZE(controllers));
    Controller * controller = &controllers[emptySlotI];
    controller->winId = lpddi->guidInstance;
    controller->present = true;
    controller->buttonsCount = 0;
    controllersCount++;

    strncpy(controller->name, lpddi->tszInstanceName, ARRAYSIZE(controller->name));
    HRESULT hr = context->CreateDevice(controller->winId, &controller->dev, NULL);
    if (hr == DI_OK)
    {
        hr = controller->dev->EnumObjects(DIEnumDeviceObjectsCallback, controller, DIDFT_ALL);
        if (hr == DI_OK)
        {
            controller->sequence++;
            LOG(default, controller, "New controller: %s", controller->name);
            LOG(default, controller, "Buttons:");
            for(i32 i = 0; i < controller->buttonsCount; i++){
                LOG(default, controller, " - %s [%s]", controller->buttons[i].name, controllerObjectToStr(controller->buttons[i].type));
            }
        }
        else {
            controller->dev->Release();
            controller->dev = NULL;
            controller->present = false;
        }
    }
    else {
        controller->dev->Release();
        controller->dev = NULL;
        controller->present = false;
    }

    return DIENUM_CONTINUE;
}


void retireAvailableControllers()
{
    for (u8 i = 0; i < controllersCount; i++){
        if (controllers[i].dev != NULL)
        {
            LOG(default, controller, "Removing controller: %s", controllers[i].name);
            controllers[i].dev->Release();
            controllers[i].dev = NULL;
            controllers[i].present = false;
        }
    }
}

bool refreshAvailableControllers()
{
    bool result = true;
    for (u8 i = 0; i < controllersCount; i++){
        controllers[i].present = false;
    }
    HRESULT hr = context->EnumDevices(DI8DEVCLASS_GAMECTRL, syncControllers, NULL, DIEDFL_ATTACHEDONLY);
    result = result && (hr == DI_OK);
    for (u8 i = 0; i < controllersCount; i++){
        if (!controllers[i].present && controllers[i].dev != NULL)
        {
            LOG(default, controller, "Removing controller: %s", controllers[i].name);
            controllers[i].dev->Release();
            controllers[i].dev = NULL;
        }
    }

    return result;
}

bool setUpAndUseController(ControllerHandle * handle)
{
    Controller * controller = &controllers[handle->slotIndex];
    if (handle->sequence != controller->sequence){
        return false;
    }
    // TODO whole layout
    DIPROPDWORD val = {};
    val.diph.dwSize = sizeof(DIPROPDWORD);
    val.diph.dwHeaderSize = sizeof(DIPROPHEADER);
    val.diph.dwObj = 0;
    val.diph.dwHow = DIPH_DEVICE;
    val.dwData = MEGABYTE(1);
    HRESULT hr = controller->dev->SetProperty(DIPROP_BUFFERSIZE, CAST(DIPROPHEADER*, &val));
    if (hr != DI_OK)
    {
        return false;
    }
    val.dwData = 0;
    hr = controller->dev->SetProperty(DIPROP_DEADZONE, CAST(DIPROPHEADER*, &val));
    if (hr != DI_OK)
    {
        return false;
    }
    val.dwData = 10000;
    hr = controller->dev->SetProperty(DIPROP_SATURATION, CAST(DIPROPHEADER*, &val));
    if (hr != DI_OK)
    {
        return false;
    }
    val.dwData = 10000;
    hr = controller->dev->SetProperty(DIPROP_FFGAIN, CAST(DIPROPHEADER*, &val));
    if (hr != DI_OK)
    {
        return false;
    }

    hr = controller->dev->SetCooperativeLevel(window, DISCL_EXCLUSIVE | DISCL_BACKGROUND);
    if (hr != DI_OK)
    {
        return false;
    }

    i32 iWant = 2;
    DIOBJECTDATAFORMAT objs[2] = {};

    DIPROPRANGE range = {};
    range.diph.dwSize = sizeof(DIPROPRANGE);
    range.diph.dwHeaderSize = sizeof(DIPROPHEADER);

    i32 runningOffset = 0;
    for (i32 i = 0; i < iWant; i++)
    {
        objs[i].pguid = &controller->buttons[i].guidType; 
        objs[i].dwOfs = runningOffset; 
        objs[i].dwType = controller->buttons[i].dwType;
        switch(controller->buttons[i].type)
        {
            case ControllerObject_XAxis:
            case ControllerObject_YAxis:
            case ControllerObject_ZAxis:
            case ControllerObject_RxAxis:
            case ControllerObject_RyAxis:
            case ControllerObject_RzAxis:
            {
                range.diph.dwHow = DIPH_BYID;
                range.diph.dwObj = controller->buttons[i].dwType;
                hr = controller->dev->GetProperty(DIPROP_RANGE, CAST(DIPROPHEADER*, &range));
                if (hr != DI_OK)
                {
                    return false;
                }
                controller->buttons[i].min = range.lMin;
                controller->buttons[i].max = range.lMax;
                controller->buttons[i].center = (controller->buttons[i].max + controller->buttons[i].min) / 2;
                controller->buttons[i].deviation = MIN(controller->buttons[i].center - controller->buttons[i].min, controller->buttons[i].max - controller->buttons[i].center); 
                controller->buttons[i].offset = runningOffset;
                controller->buttons[i].want = true;
                runningOffset += 4;
            }break;
            default:{
                INV;
            }
        }
    }

    DIDATAFORMAT objects = {};
    objects.dwSize = sizeof(DIDATAFORMAT);
    objects.dwObjSize = sizeof(DIOBJECTDATAFORMAT);
    objects.dwFlags = DIDF_ABSAXIS;
    objects.dwDataSize = runningOffset;
    objects.dwNumObjs = ARRAYSIZE(objs);
    objects.rgodf = objs;
    hr = controller->dev->SetDataFormat(&objects);
    if (hr != DI_OK)
    {
        return false;
    }

    hr = controller->dev->Acquire();
    if (hr != DI_OK)
    {
        return false;
    }

    return true;
}

bool unuseController(ControllerHandle * handle)
{
    Controller * controller = &controllers[handle->slotIndex];
    if (handle->sequence == controller->sequence){
        return controller->dev->Unacquire() == DI_OK;
    }
    return false;
}

ControllerState * getControllerState(ControllerHandle * handle)
{
    Controller * controller = &controllers[handle->slotIndex];
    if (handle->sequence == controller->sequence){
        return &controller->state;
    }
    return NULL;
}

bool pollInput(ControllerHandle * handle)
{
    Controller * controller = &controllers[handle->slotIndex];
    if (handle->sequence != controller->sequence){
        return false;
    }
    DWORD datasize = ARRAYSIZE(controller->data);
    HRESULT hr = controller->dev->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), controller->data, &datasize, 0);
    ASSERT(hr == DI_OK);
    for (DWORD i = 0; i < datasize; i++){
        i32 b = -1;

        for(i32 cb = 0; cb < controller->buttonsCount; cb++){
            if (controller->buttons[cb].want && controller->buttons[cb].offset == controller->data[i].dwOfs)
            {
                b = cb;
                break;
            }
        }
        ASSERT(b != -1);

        i32 value = controller->data[i].dwData - controller->buttons[b].center;
        i32 deadzone = CAST(i32, 0.15f * controller->buttons[b].deviation);
        i32 saturation = CAST(i32, 0.85f * controller->buttons[b].deviation);
        if (value >= -deadzone && value <= deadzone)
        {
            value = 0;
        }
        if (value <= -saturation)
        {
            value = -controller->buttons[b].deviation;
        }
        if (value >= saturation)
        {
            value = controller->buttons[b].deviation;
        }

        f32 val = CAST(f32, value) / CAST(f32, controller->buttons[b].deviation);
        //f32 val = CAST(f32,data[i].dwData);
        if(controller->data[i].dwOfs == 0)
        {
            // We want Y up
            controller->state.position.y = -val;
        }else
        {
            ASSERT(controller->data[i].dwOfs == 4);
            controller->state.position.x = val;
        }
    }
    return true;
}
