/****************************
Created  BY Saki     @20171031
Modified BY Maroonpu @20181109
****************************/

#include "include/alexa.h"
#include <queue>
#include <vector>
#include <fstream>
#include <string>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <typeinfo>
#include <thread>
#include <sys/ioctl.h>

#define IS_WIDORA      0    //1 for WIDORA compile, 1 for PC 


/*********WIDORA DEBUG************/
// GPIO
#include <stdint.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <wait.h>

// GPIO
#define MMAP_PATH   "/dev/mem"

#define RALINK_GPIO_DIR_IN      0
#define RALINK_GPIO_DIR_OUT     1

#define RALINK_REG_PIOINT       0x690
#define RALINK_REG_PIOEDGE      0x6A0
#define RALINK_REG_PIORENA      0x650
#define RALINK_REG_PIOFENA      0x660
#define RALINK_REG_PIODATA      0x620
#define RALINK_REG_PIODIR       0x600
#define RALINK_REG_PIOSET       0x630
#define RALINK_REG_PIORESET     0x640

#define RALINK_REG_PIO6332INT       0x694
#define RALINK_REG_PIO6332EDGE      0x6A4
#define RALINK_REG_PIO6332RENA      0x654
#define RALINK_REG_PIO6332FENA      0x664
#define RALINK_REG_PIO6332DATA      0x624
#define RALINK_REG_PIO6332DIR       0x604
#define RALINK_REG_PIO6332SET       0x634
#define RALINK_REG_PIO6332RESET     0x644

#define RALINK_REG_PIO9564INT       0x698
#define RALINK_REG_PIO9564EDGE      0x6A8
#define RALINK_REG_PIO9564RENA      0x658
#define RALINK_REG_PIO9564FENA      0x668
#define RALINK_REG_PIO9564DATA      0x628
#define RALINK_REG_PIO9564DIR       0x608
#define RALINK_REG_PIO9564SET       0x638
#define RALINK_REG_PIO9564RESET     0x648

/*******************主函数宏定义**********************/  
#define FUNC_IDLE_EVENT                             0x00        //空闲  
#define START_FUNC_GUA_KEY_SHORT_EVENT              0x10        //短按键处理事件  
#define FUNC_GUA_KEY_LONG_EVENT                     0x11        //长按键处理事件
#define VOLUP_FUNC_GUA_KEY_SHORT_EVENT              0x12        //短按键处理事件  
#define VOLDOWN_FUNC_GUA_KEY_SHORT_EVENT            0x13        //短按键处理事件  

/*********************宏定义************************/  
//按键的触发状态  
#define KEY_STATUS_IDLE                             0           //按键没触发  
#define KEY_STATUS_TRIGGER_SHORT                    1           //短按键触发  
#define KEY_STATUS_TRIGGER_LONG                     2           //长按键触发  
#define KEY_STATUS_NO_LOOSEN                        3           //长按键触发后未松开  

//按键触发宏  
#define KEY_TRIGGER                                 0           //低电平触发  
  
//按键消抖宏  
#define KEY_DISAPPEARS_SHAKES_SHORT_COUNT           500000      //短按键消抖数，约xms(估值)  
#define KEY_DISAPPEARS_SHAKES_LONG_COUNT            10000000    //长按键消抖数，约xS (估值)
  
/*********************内部变量************************/  
static unsigned long sKey_DisappearsShakes_TriggerCount = 0;    //消抖时的触发状态计数值  

/*********************************/

using namespace std;

typedef short SAMPLE;

ALEXA_DEVICE devices;
string Workdir;
//音频缓存队列
vector<char> block;
queue<vector<char>> blocks;

const int MaxBlock = 512;



struct WriteThis {
  const char *readptr;
  long sizeleft;
};

FILE* f;
PaError err = paNoError;
PaStream *stream;
int Alexa::writeAudio = 1;
int uploadCount = 0;
int rec_status = 0;

//GPIO
uint8_t* gpio_mmap_reg = NULL;
int gpio_mmap_fd = 0;



Alexa::Alexa()
{

}

Alexa::~Alexa()
{
    Pa_CloseStream(stream);
    Pa_Terminate();
}


size_t Upload(void *ptr, ALEXA_DEVICE devices)
{

    if(!blocks.empty()){
        cout<<"Queue Size is : "<<blocks.size()<<endl;
        vector<char> tmp;

        tmp = blocks.front();//取队列最前端数据
        memcpy(ptr,&tmp[0],devices.recTruckSize*2);

        blocks.pop();//剔除最前端数据
        fwrite(ptr,1,devices.recTruckSize*2,f);//音频数据写入文件

        uploadCount++;
        if(uploadCount==20){
            cout<<"############### UPLOADING ###############\n\r"<<endl;
            uploadCount=0;
        }
        return devices.recTruckSize*2;//上传数据
    }
    else{
        memset(ptr,0,2);
        fwrite(ptr,1,2,f);
        return 2;
    }
}

//音频分块上传回调函数
size_t Alexa::trunkUpload(void *ptr) 
{
//    ALEXA_DEVICE device = ReadDeviceinfo(getWorkdir());
//    等待一小段时间，同步采样和上传速度
//    usleep(35000);
    switch(rec_status)
    {
    case 0:
        cout<<"\n\n############### UPLOAD FINISHED ################\n\r"<<endl;
        for(int i=0;i<blocks.size();i++){
            blocks.pop();//上传结束，清空队列
            uploadCount=0;
        }
        return 0;
    case 1:
         return Upload(ptr,devices);
    }

    cout<<"\n\n############## UPLOAD RETURNED IRREGULAR ###############\n\r"<<endl;
    return 0;
}


//回调函数，接收服务器发送回来的所有数据并写入stream指向的文件中
size_t Alexa::write_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
    cout<<"writing data..."<<endl;
    int written = fwrite(ptr,size,nmemb,(FILE *)stream);
    fflush((FILE *)stream);
    return written;
}


//回调函数
size_t Alexa::ReWriteConfig(void *ptr)
{
    cout<<"writing config data..."<<endl;
    Document doc,_doc;
    doc.SetObject();
    Document::AllocatorType &allocator = doc.GetAllocator();
    size_t t= strlen((char*)ptr);
    doc.Parse<0>((char*)ptr);

    ifstream fin((Workdir+"/config.json").c_str());
    stringstream buffer;
    buffer<<fin.rdbuf()<<endl;
    string contents(buffer.str());
    _doc.Parse<0>(contents.c_str());

    Value &client_id = _doc["client_id"];
    Value &client_secret = _doc["client_secret"];
    Value &s_audio_path = _doc["s_audio_path"];
    Value &r_audio_path = _doc["r_audio_path"];
    Value &sys_audio_path = _doc["sys_audio_path"];
    fin.close();


    doc.AddMember("client_id",client_id,allocator);
    doc.AddMember("client_secret",client_secret,allocator);
    doc.AddMember("s_audio_path",s_audio_path,allocator);
    doc.AddMember("r_audio_path",r_audio_path,allocator);
    doc.AddMember("sys_audio_path",sys_audio_path,allocator);
    StringBuffer buf;
    PrettyWriter<StringBuffer> pretty_writer(buf);
    doc.Accept(pretty_writer);

    ofstream fout((Workdir+"/config.json").c_str());
    fout<<buf.GetString()<<endl;
    fout.close();
    return t;
}

/*********WIDORA DEBUG************/

int gpio_mmap(void)
{
    if ((gpio_mmap_fd = open(MMAP_PATH, O_RDWR)) < 0) {
        fprintf(stderr, "unable to open mmap file");
        return -1;
    }

    gpio_mmap_reg = (uint8_t*) mmap(NULL, 1024, PROT_READ | PROT_WRITE,
        MAP_FILE | MAP_SHARED, gpio_mmap_fd, 0x10000000);
    if (gpio_mmap_reg == MAP_FAILED) {
        perror("foo");
        fprintf(stderr, "failed to mmap");
        gpio_mmap_reg = NULL;
        close(gpio_mmap_fd);
        return -1;
    }

    return 0;
}

int mt76x8_gpio_get_pin(int pin)
{
    uint32_t tmp = 0;

    /* MT7621, MT7628 */
    if (pin <= 31) {
        tmp = *(volatile uint32_t *)(gpio_mmap_reg + RALINK_REG_PIODATA);
        tmp = (tmp >> pin) & 1u;
    } else if (pin <= 63) {
        tmp = *(volatile uint32_t *)(gpio_mmap_reg + RALINK_REG_PIO6332DATA);
        tmp = (tmp >> (pin-32)) & 1u;
    } else if (pin <= 95) {
        tmp = *(volatile uint32_t *)(gpio_mmap_reg + RALINK_REG_PIO9564DATA);
        tmp = (tmp >> (pin-64)) & 1u;
        tmp = (tmp >> (pin-24)) & 1u;
    }
    return tmp;

}

void mt76x8_gpio_set_pin_direction(int pin, int is_output)
{
    uint32_t tmp;

    /* MT7621, MT7628 */
    if (pin <= 31) {
        tmp = *(volatile uint32_t *)(gpio_mmap_reg + RALINK_REG_PIODIR);
        if (is_output)
            tmp |=  (1u << pin);
        else
            tmp &= ~(1u << pin);
        *(volatile uint32_t *)(gpio_mmap_reg + RALINK_REG_PIODIR) = tmp;
    } else if (pin <= 63) {
        tmp = *(volatile uint32_t *)(gpio_mmap_reg + RALINK_REG_PIO6332DIR);
        if (is_output)
            tmp |=  (1u << (pin-32));
        else
            tmp &= ~(1u << (pin-32));
        *(volatile uint32_t *)(gpio_mmap_reg + RALINK_REG_PIO6332DIR) = tmp;
    } else if (pin <= 95) {
        tmp = *(volatile uint32_t *)(gpio_mmap_reg + RALINK_REG_PIO9564DIR);
        if (is_output)
            tmp |=  (1u << (pin-64));
        else
            tmp &= ~(1u << (pin-64));
        *(volatile uint32_t *)(gpio_mmap_reg + RALINK_REG_PIO9564DIR) = tmp;
    }
}

void mt76x8_gpio_set_pin_value(int pin, int value)
{
    uint32_t tmp;

    /* MT7621, MT7628 */
    if (pin <= 31) {
        tmp = (1u << pin);
        if (value)
            *(volatile uint32_t *)(gpio_mmap_reg + RALINK_REG_PIOSET) = tmp;
        else
            *(volatile uint32_t *)(gpio_mmap_reg + RALINK_REG_PIORESET) = tmp;
    } else if (pin <= 63) {
        tmp = (1u << (pin-32));
        if (value)
            *(volatile uint32_t *)(gpio_mmap_reg + RALINK_REG_PIO6332SET) = tmp;
        else
            *(volatile uint32_t *)(gpio_mmap_reg + RALINK_REG_PIO6332RESET) = tmp;
    } else if (pin <= 95) {
        tmp = (1u << (pin-64));
        if (value)
            *(volatile uint32_t *)(gpio_mmap_reg + RALINK_REG_PIO9564SET) = tmp;
        else
            *(volatile uint32_t *)(gpio_mmap_reg + RALINK_REG_PIO9564RESET) = tmp;
    }
}


//******************************************************************************                
//name:             Key_Scan               
//introduce:        按键检测触发状态            
//parameter:        Pin      
//return:           KEY_STATUS_IDLE or KEY_STATUS_TRIGGER_SHORT or KEY_STATUS_TRIGGER_LONG or KEY_STATUS_NO_LOOSEN               
//changetime:       2018.04.08                        
//******************************************************************************    
unsigned char Key_Scan(int Pin)      
{      
    //触发  
    if(mt76x8_gpio_get_pin(Pin) == KEY_TRIGGER)   
    {  
        //如果上一次按键是长按键结束的，需要检测到松开方可开始下一次计数  
        if(sKey_DisappearsShakes_TriggerCount >= KEY_DISAPPEARS_SHAKES_LONG_COUNT)  
        {      
            return KEY_STATUS_NO_LOOSEN;  
        }  

        //计数  
        sKey_DisappearsShakes_TriggerCount++;  

        //判断计数是否足够  
        while(sKey_DisappearsShakes_TriggerCount >= KEY_DISAPPEARS_SHAKES_SHORT_COUNT)  
        {  
            //检测到松开、超时的时候，则处理  
            if((mt76x8_gpio_get_pin(Pin) != KEY_TRIGGER) || (sKey_DisappearsShakes_TriggerCount >= KEY_DISAPPEARS_SHAKES_LONG_COUNT))  
            {  
                //长按键时  
                if(sKey_DisappearsShakes_TriggerCount >= KEY_DISAPPEARS_SHAKES_LONG_COUNT)  
                {            
                    return KEY_STATUS_TRIGGER_LONG;                           
                }  
                //短按键时  
                else  
                {           
                    return KEY_STATUS_TRIGGER_SHORT;                            
                }  
            }  

            //继续计数用来判断长短按键  
            sKey_DisappearsShakes_TriggerCount++;        
        }      
    }  
    //未触发  
    else  
    {  
        sKey_DisappearsShakes_TriggerCount = 0;   
    }  

    return KEY_STATUS_IDLE;    
}   

/*********************************/

//检测键盘输入
void Alexa::KeyboardInput(char* cmd)
{
   
   thread p1(&Alexa::_KeyboardInput,this,cmd);//创建新的线程检测输入，避免主线程阻塞
   p1.detach();
}

/***********PC DEBUG**************/
// void Alexa::_KeyboardInput(char* cmd)
// {
//     while(1){
//         cin>>cmd;
//     }

// }
/*********************************/


/*********WIDORA DEBUG************/
void Alexa::_KeyboardInput(char* cmd)
{

/************PC DEBUG************/
    if(!IS_WIDORA){
        while(1){
            cin>>cmd;
        }
    }

/*********WIDORA DEBUG************/
    gpio_mmap();

    unsigned char gGUA_Function = 0;
    unsigned char nRet = 0;
    // unsigned char volUpRet = 0;
    // unsigned char volDownRet = 0;
    while (1)
    {

        switch(gGUA_Function)  
        {  
            //空闲  
            case FUNC_IDLE_EVENT:   
            {                
                //检测按键当前状态  
                nRet = Key_Scan(14);
                // volUpRet = Key_Scan(15);
                // volDownRet = Key_Scan(16);

                //短按键触发时  
                if(nRet == KEY_STATUS_TRIGGER_SHORT)  
                {  
                    gGUA_Function = START_FUNC_GUA_KEY_SHORT_EVENT;    
                }  
                //长按键触发时  
                else if(nRet == KEY_STATUS_TRIGGER_LONG)  
                {  
                    gGUA_Function = FUNC_GUA_KEY_LONG_EVENT;                 
                }  
                // if(volUpRet == KEY_STATUS_TRIGGER_SHORT)  
                // {  
                //     gGUA_Function = VOLUP_FUNC_GUA_KEY_SHORT_EVENT;    
                // } 

                // if(volDownRet == KEY_STATUS_TRIGGER_SHORT)  
                // {  
                //     gGUA_Function = VOLDOWN_FUNC_GUA_KEY_SHORT_EVENT;    
                // }  

                break;            
            }  

            //短按键处理  
            case START_FUNC_GUA_KEY_SHORT_EVENT:   
            {
                // printf("short press\n");
                cout<<"short press"<<endl;
                // start recording
                system("mpc clear");
                system("mpc add start.mp3");
                system("mpc play 1");//start rec sound

                cmd[0] = 'b';
                //返回空闲状态  
                gGUA_Function = FUNC_IDLE_EVENT;    
                break;                 
            }    

            // case VOLUP_FUNC_GUA_KEY_SHORT_EVENT:   
            // {
            //     // printf("short press\n");
            //     cout<<"short press"<<endl;
            //     // start recording
            //     // system("mpc clear");
            //     // system("mpc add start.mp3");
            //     system("mpc play 1");//start rec sound

            //     // cmd[0] = 'b';
            //     //返回空闲状态  
            //     gGUA_Function = FUNC_IDLE_EVENT;    
            //     break;                 
            // }         

            // case VOLDOWN_FUNC_GUA_KEY_SHORT_EVENT:   
            // {
            //     // printf("short press\n");
            //     cout<<"short press"<<endl;
            //     // start recording
            //     // system("mpc clear");
            //     // system("mpc add start.mp3");
            //     system("mpc volume +10");//start rec sound

            //     // cmd[0] = 'b';
            //     //返回空闲状态  
            //     gGUA_Function = FUNC_IDLE_EVENT;    
            //     break;                 
            // }         

            //长按键处理  
            case FUNC_GUA_KEY_LONG_EVENT:   
            {  
                cout<<"long press"<<endl;
                //enter network connet mode
                // printf("long press\n");

                //返回空闲状态  
                gGUA_Function = FUNC_IDLE_EVENT;    
                break;                 
            }    

            //其他  
            default:   
            {  
                //返回空闲状态  
                gGUA_Function = FUNC_IDLE_EVENT;                                       
                break;             
            }                       
        }                       

    }
    close(gpio_mmap_fd);

}


//回调函数，接收Downchannel链接收到的数据并解析，目前仅能解析StopCapture信息，其它待增加
size_t Alexa::DownChannelParser(void *ptr, size_t size, size_t nmemb, void *pointer)
{

    system("mpc clear");
    system("mpc add stop.mp3");
    system("mpc play 1");//stop rec sound

    cout<<"---------------W-R-I-T-I-N-G-----------------"<<endl;
    char *p1 = strstr((char*)ptr,"Content-Type: application/json");
    p1 = p1 + 32;
    char *p2 = strstr((char*)p1,"--");
    memset(p2,'\0',1);

    DownParams * params = (DownParams*)pointer;
    IsEndpoint(p1, params);
    if(params->mark == ENDPOINT_DIRE){
        rec_status = 0;
        err = Pa_StopStream(stream);//停止音频录制流
        fclose(f);
    }
    else if(params->mark == SETALERT_DIRE){

    }
    return size*nmemb;
}


/**********Skill Interface********/
// //Alexa Skill interface
// string Alexa::Skill(string directiveJson)
// {
//     static string play_token  = "b4210b93-a504-492a-8998-363cd2edae80";
//     static string stop_token  = "50c74b98-dcb0-417c-91cd-12a8647207a7";
//     static string next_token  = "32cb226e-886e-4bb5-8fd4-e0e3c90ce6f7";
//     static string prev_token  = "3df35bfb-9155-4143-9519-f6f744deaeda";

//     size_t index = directiveJson.find("ask.skill.");

//     cout<<"@@@@@@@@@@"<<endl;
//     if(-1 != index)
//     {
//         string tmpJson = directiveJson.substr(index+10,36);
//         cout<<tmpJson<<endl;

//         cout<<"@@@@@@@@@@"<<endl;
//         if(play_token == tmpJson)
//         {
//             /******start code edit here******/
//             cout<<"music play"<<endl;
//             // system("mpc play 3");
//             return "1";

//             /********************************/
//         }
//         else if(stop_token == tmpJson)     
//         {
//             /******stop code edit here******/
//             cout<<"music stop"<<endl;
//             // system("mpc stop");
//             return "2";

//             /*******************************/
//         } 
//         else if(next_token == tmpJson)     
//         {
//             /******next code edit here******/
//             cout<<"music next"<<endl;
//             // system("mpc next");
//             return "3";

//             /*******************************/
//         } 
//         else if(prev_token == tmpJson)     
//         {
//             /****previous code edit here****/
//             cout<<"music prev"<<endl;
//             // system("mpc prev");
//             return "4";

//             /*******************************/
//         } 
//         else
//         {
//             cout<<"no match inner"<<endl;
//         }

//     }
//     else
//     {
//         cout<<"no match"<<endl;
//     }

//     cout<<"@@@@@@@@@@"<<endl;
        
//     return "0";
// }

/*********************************/

//解析从Events Channel接收的multipart数据，multipart数据包含header、json、audio三个部分
//每个部分由boundary字符串分割
string Alexa::Parser(string &json)//,string &orderFlag)
{
//    string Time = getTime();
    ifstream fin((Workdir + "/.cache").c_str(),ios::binary);
    string temp;
    size_t b_index, e_index;
    bool _start1 = false;
    bool _start2 = false;

    while(getline(fin,temp,'\n'))//首先在header中寻找作为boundary的字符串
    {
        b_index = temp.find("boundary");
        if(b_index!=-1)
        {
            e_index = temp.find(";start");
            break;
        }

    }

    string boundary ="--"+temp.substr(b_index+9,e_index-b_index-9);
    cout<<boundary<<endl;

    //根据boundary截取json部分
    while(getline(fin,temp,'\n'))
    {
        b_index = temp.find(boundary);

        if(_start1)
        {
            json = temp;
            _start1 = false;
            break;
        }

        if(b_index!=-1){
            _start1 = true;
            getline(fin,temp,'\n');
            getline(fin,temp,'\n');
        }
    }

    // // call alexa skill 
    // orderFlag = Skill(json);

    string path = config.r_audio_path + nowtime + ".mp3";
    ofstream fout(path.c_str(), ios::binary);
    
    //根据boundary截取音频内容
    while(getline(fin,temp,'\n'))
    {
        b_index = temp.find(boundary);

        if(b_index!=-1){

            for(int i=0;i<3;i++){
                getline(fin,temp,'\n');
                if(temp.find("Content-Type: application/octet-stream")!=-1){
                    _start2 = true;
                    break;
                }
            }
        }

        if(_start2==true){
            fout<<temp<<'\n';
        }
    }

    fout.close();
    fin.close();

    string shortPath = nowtime + ".mp3";
    return shortPath; // relative address
    // return path;
}


//HTTP POST刷新access_token
//已完成：每次运行程序先刷新一次
//待完成：检测access_token是否过期，即将过期时再次刷新
void Alexa::FlashToken(CURL *curl, struct curl_slist *head)
{
//    ALEXA_CONFIG allconfig = Readtoken(getWorkdir());

    static string data = "grant_type=refresh_token&refresh_token=" \
                         + config.refresh_token \
                         + "&client_id="+config.client_id \
                         + "&client_secret="+config.client_sceret;//需要POST的字符串

//    FILE *_config;
    head = curl_slist_append(head,"Content-Type: application/x-www-form-urlencoded");

    curl_easy_setopt(curl,CURLOPT_VERBOSE,1L);
    curl_easy_setopt(curl,CURLOPT_HTTP_VERSION,CURL_HTTP_VERSION_2_0);
    curl_easy_setopt(curl,CURLOPT_SSL_VERIFYPEER,1L);
    curl_easy_setopt(curl,CURLOPT_FORBID_REUSE,1L);
    curl_easy_setopt(curl,CURLOPT_HTTPHEADER,head);
    curl_easy_setopt(curl,CURLOPT_URL,"https://api.amazon.com/auth/o2/token");
    curl_easy_setopt(curl,CURLOPT_POSTFIELDS,data.c_str());
    curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,ReWriteConfig);//回调函数接收新的token并写入config
//    curl_easy_setopt(curl,CURLOPT_WRITEDATA,_config);
}

//HTTP GET 建立DownChannel链接并保持，接收例如StopCapture等数据信息
void Alexa::SetDownChannel(CURL *curl, struct curl_slist *header, struct DownParams * params)
{
    string auth = "Authorization:Bearer " + config.access_token;

    curl_easy_setopt(curl,CURLOPT_VERBOSE,1L);
    curl_easy_setopt(curl,CURLOPT_HTTP_VERSION,CURL_HTTP_VERSION_2_0);
    curl_easy_setopt(curl,CURLOPT_SSL_VERIFYPEER,1L);
    curl_easy_setopt(curl,CURLOPT_FORBID_REUSE,1);
    curl_easy_setopt(curl,CURLOPT_TIMEOUT,3600L);

    curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,DownChannelParser);//回调函数，解析接收到的数据
    curl_easy_setopt(curl,CURLOPT_WRITEDATA, params);
    header = curl_slist_append(header,auth.c_str());

    curl_easy_setopt(curl,CURLOPT_HTTPGET,1L);
    curl_easy_setopt(curl,CURLOPT_HTTPHEADER,header);
    curl_easy_setopt(curl,CURLOPT_URL,"https://avs-alexa-na.amazon.com/v20160207/directives");
}


//每隔5min通过HTTP GET的方式ping服务器，避免DownChannel链接断开
//(待添加5min定时功能)
void Alexa::Ping(CURL *curl, struct curl_slist *header)
{
    string auth = "Authorization:Bearer " + config.access_token;

    curl_easy_setopt(curl,CURLOPT_VERBOSE,1L);
    curl_easy_setopt(curl,CURLOPT_HTTP_VERSION,CURL_HTTP_VERSION_2_0);
    curl_easy_setopt(curl,CURLOPT_SSL_VERIFYPEER,1L);
    curl_easy_setopt(curl,CURLOPT_FORBID_REUSE,1);

    header = curl_slist_append(header,auth.c_str());

    curl_easy_setopt(curl,CURLOPT_HTTPGET,1L);
    curl_easy_setopt(curl,CURLOPT_HTTPHEADER,header);
    curl_easy_setopt(curl,CURLOPT_URL,"https://avs-alexa-na.amazon.com/ping");

}

//HTTP POST向服务器同步本地状态信息，每次程序运行先要进行同步，状态信息由json格式构建
void Alexa::Synchronize(CURL *curl,struct curl_slist *header,struct curl_httppost *postFirst,struct curl_httppost *postLast)
{

    string auth = "Authorization:Bearer " + config.access_token;

    string json;
//    Event eventState = {"System",
//                        "SynchronizeState",
//                        "TAP"};

    struct Event eventState;
    eventState.header.namesp = "System";
    eventState.header.name = "SynchronizeState";
    eventState.header.msgid = "message-123";

    MultiMsgPt1(json,eventState);//json

    cout<<json<<endl;
    header = curl_slist_append(header,auth.c_str());
    header = curl_slist_append(header,"content-type:multipart/form-data");

    curl_formadd(&postFirst,&postLast,
                 CURLFORM_COPYNAME,"metadata",
                 CURLFORM_COPYCONTENTS,json.c_str(),
                 CURLFORM_CONTENTTYPE,"application/json; charset=UTF-8",
                 CURLFORM_END);//构建multipart数据

    curl_easy_setopt(curl,CURLOPT_FORBID_REUSE,1L);
    curl_easy_setopt(curl,CURLOPT_VERBOSE,1L);
    curl_easy_setopt(curl,CURLOPT_HTTP_VERSION,CURL_HTTP_VERSION_2_0);
    curl_easy_setopt(curl,CURLOPT_SSL_VERIFYPEER,1L);

    curl_easy_setopt(curl,CURLOPT_HTTPHEADER,header);
    curl_easy_setopt(curl,CURLOPT_URL,"https://avs-alexa-na.amazon.com/v20160207/events");
    curl_easy_setopt(curl,CURLOPT_HTTPPOST,postFirst);
}

//HTTP POST建立EventChannel，发送包含音频的multipart数据
void Alexa::SendAudioFile(CURL *curl,struct curl_slist *header,curl_httppost *postFirst, curl_httppost *postLast)
{
    string auth = "Authorization:Bearer " +config.access_token;
    rec_status = 1;
    FILE *saveHeaderFile;

    saveHeaderFile = fopen((Workdir+"/.cache").c_str(),"w+");

    char *buffer = (char*)malloc(64);
    static struct WriteThis pooh;
    pooh.readptr = (const char*)buffer;
    pooh.sizeleft = 64;

    string json;
    struct Event eventState;
    eventState.header.namesp = "SpeechRecognizer";
    eventState.header.name = "Recognize";
    eventState.header.msgid = "message-123";
    eventState.header.dialogRequestid = "dialogrequest-321";
    eventState.payload.initiator = "TAP";
    eventState.payload.profile = "NEAR_FIELD";
    eventState.payload.format = "AUDIO_L16_RATE_16000_CHANNELS_1";

    MultiMsgPt1(json,eventState);//状态信息

    header = curl_slist_append(header,auth.c_str());
    header = curl_slist_append(header,"content-type:multipart/form-data");

    curl_formadd(&postFirst,&postLast,
                 CURLFORM_COPYNAME,"metadata",
                 CURLFORM_COPYCONTENTS,json.c_str(),
                 CURLFORM_CONTENTTYPE,"application/json; charset=UTF-8",
                 CURLFORM_END);//json部分

    curl_formadd(&postFirst,&postLast,
                 CURLFORM_COPYNAME,"audio",
                 CURLFORM_STREAM,&pooh,
                 CURLFORM_CONTENTSLENGTH,0,
                 CURLFORM_CONTENTTYPE,"application/octet-stream",
                 CURLFORM_END);//音频部分

    curl_easy_setopt(curl,CURLOPT_TIMEOUT,100L);
    curl_easy_setopt(curl,CURLOPT_FORBID_REUSE,1L);
    curl_easy_setopt(curl,CURLOPT_READFUNCTION,trunkUpload);//通过回调函数上传音频数据
    curl_easy_setopt(curl,CURLOPT_HEADERFUNCTION,write_data);//接收返回的header数据
    curl_easy_setopt(curl,CURLOPT_HEADERDATA,saveHeaderFile);
    curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,write_data);//接收返回的multipart数据
    curl_easy_setopt(curl,CURLOPT_WRITEDATA,saveHeaderFile);
    curl_easy_setopt(curl,CURLOPT_VERBOSE,1L);
    curl_easy_setopt(curl,CURLOPT_HTTP_VERSION,CURL_HTTP_VERSION_2_0);
    curl_easy_setopt(curl,CURLOPT_SSL_VERIFYPEER,1L);
    curl_easy_setopt(curl,CURLOPT_HTTPHEADER,header);
    curl_easy_setopt(curl,CURLOPT_URL,"https://avs-alexa-na.amazon.com/v20160207/events");
    curl_easy_setopt(curl,CURLOPT_HTTPPOST,postFirst);
}


void Alexa::capture()
{
    string filepath = config.s_audio_path + nowtime + ".pcm";
    cout<< filepath <<endl;

    //delete other pcm to save space
    string del_command = "cd "+config.s_audio_path+" && ls | grep -v "+filepath+" | xargs rm";
    system(del_command.c_str());


    f = fopen(filepath.c_str(),"wb");

    err = Pa_StartStream(stream);//开启录音流
    if(err != paNoError){
        cout<<"PortAudio start stream error: "<<Pa_GetErrorText(err)<<endl;
        exit(1);
    }
    cout<<"Now recording..."<<endl;

}

//停止音频采集函数
void Alexa::stopCapture()
{
    err = Pa_StopStream(stream);//停止录音流
    if(err != paNoError){
        cout<<"PortAudio close stream error: "<<Pa_GetErrorText(err)<<endl;
        exit(1);
    }
    
    rec_status = 0;
}

void Alexa::init()//功能初始化
{

    devices = device;
    Workdir = workdir;
    block.resize(sizeof(SAMPLE) * 512);
    err = Pa_Initialize();//Portaudio初始化
    if (err != paNoError){
        cout<<"PortAudio init error: "<<Pa_GetErrorText(err)<<endl;
        exit(1);
    }

    PaStreamParameters inputParameters;
    inputParameters.device=devices.useDeviceNum;//声卡设备号
    inputParameters.channelCount=1;//声道数
    inputParameters.sampleFormat=paInt16;//PCM格式
    cout<<Pa_GetDeviceInfo(inputParameters.device)<<endl;
    inputParameters.suggestedLatency=Pa_GetDeviceInfo(inputParameters.device)->defaultLowInputLatency;
    inputParameters.hostApiSpecificStreamInfo=NULL;


    err = Pa_OpenStream(
                &stream,
                &inputParameters,
                NULL,
                16000,
                devices.recTruckSize,
                paClipOff,
                &CaptureCallback,//录音回调
                NULL);//开启流
    if(err != paNoError){
        cout<<"PortAudio open stream error: "<<Pa_GetErrorText(err)<<endl;
        exit(1);
    }

}

//录音回调函数，音频数据塞入音频缓存队列最后端
int CaptureCallback(const void *input,
                    void *output,unsigned long frameCount,
                    const PaStreamCallbackTimeInfo * timeInfo,
                    PaStreamCallbackFlags statusFlags,void *userData)
{

    memcpy(&block[0],input, sizeof(SAMPLE)*frameCount);

    if(blocks.size() > MaxBlock){
        blocks.pop();
    }
    blocks.push(block);//塞入队列
    return paContinue;
}
