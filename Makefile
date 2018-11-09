
# Created  BY Saki
# Modified BY Maroonpu @20181109

DIR_BASE = $(shell cd .. && pwd)
DIR_SRC  = ./src
DIR_INC  = ./include

#########################PC DEBUG##########################
cc = g++ -std=c++11
LDFLAGS += -L/usr/local/lib

LIBS += -lcurl -lportaudio  \
        -lpthread           \
        -lasound -lm 

# # pocketsphinx wake-up engine, enable if necessary
# LIBS += -lpocketsphinx -lsphinxbase -lsphinxad 
###########################################################

#######################WIDORA DEBUG########################
# cc = mipsel-openwrt-linux-g++ -std=c++11
# LDFLAGS += -L$(DIR_BASE)/openwrt_widora/staging_dir/target-mipsel_24kec+dsp_uClibc-0.9.33.2/usr/lib

# LIBS += -lcurl -lportaudio -lasound  \
#          -lcrypto -lnghttp2 -lssl -lz -lffi 
###########################################################	

# Share HeadFile 
CPPFLAGS += -I$(DIR_BASE)/FindAlexa  \
            -I$(DIR_BASE)/openwrt_widora/staging_dir/target-mipsel_24kec+dsp_uClibc-0.9.33.2/usr/include 

all : alexa deviceinfo 
.PHONY : all
deviceinfo : $(DIR_SRC)/info.cpp
	$(cc) -o deviceinfo $(DIR_SRC)/info.cpp $(CPPFLAGS) $(LDFLAGS) $(LIBS)

alexa : base.o alexa.o main.o  alert.o 
	$(cc) -o alexa base.o alert.o alexa.o main.o $(CPPFLAGS) $(LDFLAGS) $(LIBS) 

main.o : $(DIR_SRC)/main.cpp $(DIR_INC)/alexa.h $(DIR_INC)/base.h 
	$(cc) -c $(DIR_SRC)/main.cpp $(CPPFLAGS) $(LDFLAGS) $(LIBS) 

base.o : $(DIR_SRC)/base.cpp $(DIR_INC)/base.h
	$(cc) -c $(DIR_SRC)/base.cpp $(CPPFLAGS)

alexa.o : $(DIR_SRC)/alexa.cpp $(DIR_INC)/alexa.h $(DIR_INC)/base.h $(DIR_INC)/alert.h
	$(cc) -c $(DIR_SRC)/alexa.cpp $(CPPFLAGS) $(LDFLAGS) $(LIBS) 

alert.o : $(DIR_SRC)/alert.cpp $(DIR_INC)/alert.h
	$(cc) -c $(DIR_SRC)/alert.cpp $(CPPFLAGS)

# # pocketsphinx wake-up engine, enable if necessary
# kws.o : $(DIR_SRC)/kws.cpp $(DIR_INC)/kws.h
# 	$(cc) -c $(DIR_SRC)/kws.cpp $(CPPFLAGS) $(LDFLAGS) $(LIBS)

.PHONY : clean
clean:
	-rm *.o alexa deviceinfo
