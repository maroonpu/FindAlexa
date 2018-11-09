import os, json

homedir =  os.path.expandvars('$HOME')

a_token = 'Atza|IwEBICQh9XwLH2tzpBfIKhd3epRC174X4Y4Xyrx20ytUQagpByrZe333kBmpE4dk7dbf_jk6kxFl1DYfGdSBzShnt0mbeFJRN2Ql_IAB0-S4SgpzjIotSgrCXL-o59y2lu0rzKucMOz04bGzsPf0aB-PCMj0ab9eyxWmHWXbQcvv24gRbaQUEw3cEao-I833lE6BMACdT2NvyHHfs8au0smdJTbySdBV2yLxeZC5QOXKj_ns6yIDkv6zbgO2J9mgkWBq5q12GEWpS-dUlHFBhLcQ34bdVuWq0mUzyNQfGaiI0n-YolovWHv0YqMw5jpF34eNU7uCI9oj29nrBBGUcseeKiMnDEdAWGfKt61NcnyvjJ-x8-kcs2GCZkj_fBn-Ib30O_Sb1ebmVmAaeGogrVkGXvAz0iwiZLH4Z8mhYbVOocURd-ggq-0rEX-uEqFN-j0urmW_0V6mj9D98j8K1AGj5Rrprr6u3wdXk-VZgBztUU_uEZp-DJw_XwYcQisA4ZBxI0Pi2jmDDHl5iRw0ICkxob2Djh5C0lNiVhxvOzRZsGzg0CcbqFeNtBXwm3RkgthwSak'
r_token = 'Atzr|IwEBIKS2OgOUs8W6UMyAy0bDB2VWYd_pxoyLfsgHwyuk7NI3Zm0kSk81lz8JL4hCPzvTaPQgxngdSXjvMs5Q2ahjx5frYPz6MT5MsJclBB1DQCAar4ImLxxHxs-76-896Qbwod2XK82-fF_g245FJhN2pDAhdYQwSBjf81fA3eO8LUDY3gmYoOedhkHfbcWTacxK-Ed6LZ4tY5JeOKiNfaMAGYs5f14v3gq7TyihqpL30sJQYbWTUXRIEie5toh7eipTm6-soxtaLN86jqDK_JvxGY_UjJYlQ4mKpT0hmeu-Q4Iqr97VpC1fKZ41yjIOZduhVhk0VO2PXPggJtU2nm8CnxU8LWy6NG4FnpTf0d9BH_clJqk6XCclupd9kW_fssvGvp3F3UV7iRZCDnQkC5baekxCVwnpuDpBm0HBtXcQRpcrdoC6DFh0z2gA6uyDeTIGe-iNePB4kwkmBWLwcAh6uKYlhCCLfciDLEi5Uecw6i58RWlunwY6MU2-cOddHIhFaEDGUoOoPfONHi2eMnQBjIPx0ZFBw0vPhPrkc9Ag4SHV4w'
t_type = 'bearer'
expires = 3600
client_id = 'amzn1.application-oa2-client.0e303b23fb394e9991cb2822d26e51fa'
client_sec = '5fce0d3f2ee573fc74faa8a93c898a3badd2bf27d7151b518e2e73b14c2c6668'
s_path = os.path.join(homedir,'.Alexa/audio/send/')
r_path = os.path.join(homedir,'.Alexa/audio/receive/')
sys_path = os.path.join(homedir, '.Alexa/src/audio/')

if(not os.path.exists(os.path.join(homedir,'.Alexa/audio/send'))):
    os.makedirs(os.path.join(homedir,'.Alexa/audio/send'))

if(not os.path.exists(os.path.join(homedir,'.Alexa/audio/receive'))):
    os.makedirs(os.path.join(homedir,'.Alexa/audio/receive'))

config_dict = {}
config_dict['access_token'] = a_token
config_dict['refresh_token'] = r_token
config_dict['token_type'] = t_type
config_dict['expires_in'] = expires
config_dict['client_id'] = client_id
config_dict['client_secret'] = client_sec
config_dict['s_audio_path'] = s_path
config_dict['r_audio_path'] = r_path
config_dict['sys_audio_path'] = sys_path

with open(os.path.join(homedir,'.Alexa/config.json'),'w+') as f:
    json.dump(config_dict,f,indent=4)



device_dict = {}
device_dict['InputDeviceNum'] = 5
device_dict['RecTruckSize'] = 512

with open(os.path.join(homedir,'.Alexa/deviceinfo.json'),'w+') as p:
    json.dump(device_dict,p,indent=4)


print('Init Completed!\n')

print('Something else...\n')
print('Check the input device number by running \'deviceinfo\' \n')
print('Go to ~/.Alexa/deviceinfo.json and modify value of \'InputDeviceNum\' \n')
