# Maroonpu at 2017/12/24
# This script is for MTK SmartConnection(ELIAN)
import os,commands,time

os.system('iwpriv ra0 elian stop')
os.system('iwpriv ra0 elian start')

while True:
	output = commands.getoutput('iwpriv ra0 elian result')
	if -1 != output.find('rcvFlag'):
		break
	time.sleep(1)

os.system('iwpriv ra0 elian stop')

#Get SSID
startFlag = output.find('ssid=')+5
endFlag   = output.find(', pwd=')	
SSID      = output[startFlag:endFlag]

#Get KEY
startFlag = output.find('pwd=')+4
endFlag   = output.find(', user=')
KEY       = output[startFlag:endFlag]	

os.system('wifimode sta '+SSID+' '+KEY)
