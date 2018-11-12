#include <portaudio.h>
#include <iostream>
using namespace std;

int main()
{
    Pa_Initialize();
    Pa_GetDefaultInputDevice();
    cout<<"default input device is: "<<Pa_GetDeviceInfo(Pa_GetDefaultInputDevice())->name<<endl;
    int numDevices = Pa_GetDeviceCount();
    cout<<numDevices<<endl;
    for(int i=0;i<numDevices;i++){
    	cout<<i<<":";
        cout<<Pa_GetDeviceInfo(i)->name;
        cout<<Pa_GetDeviceInfo(i)->defaultSampleRate<<endl;
    }
    Pa_Terminate();
}
