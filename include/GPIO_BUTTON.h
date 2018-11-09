
// Author: Maroonpu
// Time:   2018.04.08

#ifndef GPIO_BUTTON_H
#define GPIO_BUTTON_H

#include "GPIO_CORE.h"

/*******************主函数宏定义**********************/  
#define FUNC_IDLE_EVENT         					0x00		//空闲  
#define FUNC_GUA_KEY_SHORT_EVENT                    0x10		//短按键处理事件  
#define FUNC_GUA_KEY_LONG_EVENT                     0x11		//长按键处理事件  

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

// /**********************DEMO************************/  
// int main(int argc, char **argv)
// {
// 	int ret = -1;

// 	if (gpio_mmap())
// 		return -1;
// 	unsigned char gGUA_Function = 0;
// 	unsigned char nRet = 0;
// 	while (1)
// 	{

// 		switch(gGUA_Function)  
// 		{  
// 			//空闲  
// 			case FUNC_IDLE_EVENT:   
// 			{                
// 				//检测按键当前状态  
// 				nRet = Key_Scan(14);  

// 				//短按键触发时  
// 				if(nRet == KEY_STATUS_TRIGGER_SHORT)  
// 				{  
// 					gGUA_Function = FUNC_GUA_KEY_SHORT_EVENT;    
// 				}  
// 				//长按键触发时  
// 				else if(nRet == KEY_STATUS_TRIGGER_LONG)  
// 				{  
// 					gGUA_Function = FUNC_GUA_KEY_LONG_EVENT;                 
// 				}  

// 				break;            
// 			}  

// 			//短按键处理  
// 			case FUNC_GUA_KEY_SHORT_EVENT:   
// 			{
// 				printf("short press\n");  
// 				//返回空闲状态  
// 				gGUA_Function = FUNC_IDLE_EVENT;    
// 				break;                 
// 			}         

// 			//长按键处理  
// 			case FUNC_GUA_KEY_LONG_EVENT:   
// 			{  
// 				printf("long press\n");
// 				//返回空闲状态  
// 				gGUA_Function = FUNC_IDLE_EVENT;    
// 				break;                 
// 			}    

// 			//其他  
// 			default:   
// 			{  
// 				//返回空闲状态  
// 				gGUA_Function = FUNC_IDLE_EVENT;                                       
// 				break;             
// 			}                       
// 		}                		

// 	}
// 	close(gpio_mmap_fd);

// 	return ret;
// }

#endif