
#include "../inc/cic.h"
#include "../inc/Serial.h"
#include <QMessageBox>
#include <queue>
#include <memory>

namespace cic{
    CIC::CIC(int com_port){
        this->com_port_ = com_port;
        memset(nodeInfo,0,sizeof(nodeInfo));
    }

    CIC::~CIC(){
        delete SP_;
    }

    bool CIC::connectSerial(){
        char comm[64];
        sprintf_s(comm, "\\\\.\\COM%d", com_port_);
        SP_ = new Serial(comm);    // adjust as needed

        if (!SP_->IsConnected()){
            return false;
        }
        return true;
    }

    bool CIC::getData(){
        std::queue<char> dataBuff;
        static unsigned char incomingData[1000] = "";		// don't forget to pre-allocate memory
        static unsigned char incomingDataOut[1000] = "";
        int dataLength = 18;
        int readResult = 0;
        static Node bInfo;

        if(SP_->IsConnected())
        {
            readResult = SP_->ReadData((char *)incomingData,dataLength);
            //printf("Bytes read: (0 means no data available) %i\n",readResult);
            if (readResult){
                incomingData[readResult] = 0;
                for (int i = 0; i < readResult; ++i) {
                    dataBuff.push(incomingData[i]);
                }

                if (dataBuff.size() >= dataLength){
                    int k = 0;
                    for (int i = 0; i < dataLength; i++){
                        incomingDataOut[k] = dataBuff.front();
                        k++;
                        dataBuff.pop();
                    }
                    memcpy((unsigned char *)(&bInfo), incomingDataOut + 2, sizeof(incomingDataOut)); // 7E 45 ID count X Y Z crc
                    if (bInfo.id < 5 && bInfo.id > 0){
                        new_data_ = true;
                        //printf("%d,%d,%f,%f,%f\n", bInfo.id, bInfo.count, bInfo.ac_x, bInfo.ac_y, bInfo.ac_z);
                        if(bInfo.id == 1){
                            curr_index = 1;
                            nodeInfo[0] = bInfo;
                        }else if(bInfo.id == 2){
                            curr_index = 2;
                            nodeInfo[1] = bInfo;
                        }else if(bInfo.id == 3){
                            curr_index = 3;
                            nodeInfo[2] = bInfo;
                        }else{
                            curr_index = -1;
                        }
                    }
                }
            }
        }else{
            return false;
        }
        return true;
    }    
}


