#pragma once

#define DIRECTINPUT_VERSION 0x0800
#include <Dinput.h>

static IDirectInput8 * context = NULL;

enum ControllerObject{
    ControllerObject_Invalid,

    ControllerObject_Button,
    ControllerObject_Dpad,
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
    u32 sequence;

    GUID guid;
    bool present;

    LPDIRECTINPUTDEVICE8A dev;
    DIDEVICEOBJECTDATA data[50];
    char name[50];

    struct {
        DWORD dwOffset;
        GUID guid;
        DWORD dwType;

        char name[50];
        ControllerObject type;
        union
        {
            struct
            {
                u8 rank;
            } button;
            struct{
                i32 min;
                i32 max;
                i32 center;
                i32 deviation;
            } axis;
        };

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
        case ControllerObject_Dpad:
            return "D Pad";
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
    ControllerObject type = ControllerObject_Invalid;
    if (lpddoi->dwType & DIDFT_PSHBUTTON && lpddoi->guidType == GUID_Button){
        type = ControllerObject_Button;
    }else if (lpddoi->guidType == GUID_POV){
        type = ControllerObject_Dpad;
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
        DIPROPRANGE range = {};
        range.diph.dwSize = sizeof(DIPROPRANGE);
        range.diph.dwHeaderSize = sizeof(DIPROPHEADER);
        range.diph.dwHow = DIPH_BYID;
        range.diph.dwObj = lpddoi->dwType;
        HRESULT hr = controller->dev->GetProperty(DIPROP_RANGE, CAST(DIPROPHEADER*, &range));
        if (hr != DI_OK)
        {
            return DIENUM_STOP;
        }
        controller->buttons[controller->buttonsCount].axis.min = range.lMin;
        controller->buttons[controller->buttonsCount].axis.max = range.lMax;
        controller->buttons[controller->buttonsCount].axis.center = (controller->buttons[controller->buttonsCount].axis.max + controller->buttons[controller->buttonsCount].axis.min) / 2;
        controller->buttons[controller->buttonsCount].axis.deviation = MIN(controller->buttons[controller->buttonsCount].axis.center - controller->buttons[controller->buttonsCount].axis.min, controller->buttons[controller->buttonsCount].axis.max - controller->buttons[controller->buttonsCount].axis.center); 

    } else if (lpddoi->dwType & DIDFT_NODATA){
        return DIENUM_CONTINUE;
    }
    strncpy(controller->buttons[controller->buttonsCount].name, lpddoi->tszName, ARRAYSIZE(controller->buttons[controller->buttonsCount].name));
    controller->buttons[controller->buttonsCount].type = type;
    if (type == ControllerObject_Button)
    {
        controller->buttons[controller->buttonsCount].button.rank = CAST(u8, lpddoi->wUsage);
    }
    controller->buttons[controller->buttonsCount].dwOffset = lpddoi->dwOfs;
    controller->buttons[controller->buttonsCount].guid = lpddoi->guidType;
    controller->buttons[controller->buttonsCount].dwType = lpddoi->dwType;
    controller->buttonsCount++;
    return DIENUM_CONTINUE;
}

static BOOL syncControllers(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef)
{
    (void) pvRef;
    i16 emptySlotI = controllersCount;
    for (u8 i = 0; i < controllersCount; i++){
        if (controllers[i].guid == lpddi->guidInstance && controllers[i].dev != NULL)
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
    controller->guid = lpddi->guidInstance;
    controller->present = true;
    controller->buttonsCount = 0;
    controllersCount++;

    strncpy(controller->name, lpddi->tszInstanceName, ARRAYSIZE(controller->name));
    HRESULT hr = context->CreateDevice(controller->guid, &controller->dev, NULL);
    if (hr == DI_OK)
    {
        hr = controller->dev->EnumObjects(DIEnumDeviceObjectsCallback, controller, DIDFT_ALL);
        if (hr == DI_OK)
        {
            controller->sequence++;
            LOG(default, controller, "New controller: %s", controller->name);
            LOG(default, controller, "Inputs:");
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

    DIOBJECTDATAFORMAT objs[ARRAYSIZE(controller->data)] = {};

    i32 runningOffset = 0;
    for (i32 i = 0; i < controller->buttonsCount; i++)
    {
        objs[i].pguid = &controller->buttons[i].guid; 
        objs[i].dwOfs = runningOffset; 
        objs[i].dwType = controller->buttons[i].dwType;
        runningOffset += 4;
    }

    DIDATAFORMAT objects = {};
    objects.dwSize = sizeof(DIDATAFORMAT);
    objects.dwObjSize = sizeof(DIOBJECTDATAFORMAT);
    objects.dwFlags = DIDF_ABSAXIS;
    objects.dwDataSize = runningOffset;
    objects.dwNumObjs = controller->buttonsCount;
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
    DWORD datasize = controller->buttonsCount;
    HRESULT hr = controller->dev->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), controller->data, &datasize, 0);
    ASSERT(hr == DI_OK);
    for (DWORD i = 0; i < datasize; i++){
        i32 b = -1;

        for(i32 cb = 0; cb < controller->buttonsCount; cb++){
            if (controller->buttons[cb].dwOffset == controller->data[i].dwOfs)
            {
                b = cb;
                break;
            }
        }
        ASSERT(b != -1);

        if (controller->buttons[b].type == ControllerObject_XAxis ||
            controller->buttons[b].type == ControllerObject_YAxis)
        {
            i32 value = controller->data[i].dwData - controller->buttons[b].axis.center;
            i32 deadzone = CAST(i32, 0.15f * controller->buttons[b].axis.deviation);
            i32 saturation = CAST(i32, 0.85f * controller->buttons[b].axis.deviation);
            if (value >= -deadzone && value <= deadzone)
            {
                value = 0;
            }
            if (value <= -saturation)
            {
                value = -controller->buttons[b].axis.deviation;
            }
            if (value >= saturation)
            {
                value = controller->buttons[b].axis.deviation;
            }

            f32 val = CAST(f32, value) / CAST(f32, controller->buttons[b].axis.deviation);
            //f32 val = CAST(f32,data[i].dwData);
            if (controller->buttons[b].type == ControllerObject_YAxis)
            {
                // We want Y up
                controller->state.position.y = -val;
            }else if (controller->buttons[b].type == ControllerObject_XAxis)
            {
                controller->state.position.x = val;
            }
        }
    }
    return true;
}
