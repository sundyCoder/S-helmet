#ifndef CIC_H
#define CIC_H
#include "Serial.h"

typedef unsigned char uint8_t;
typedef unsigned short int uint16_t;
typedef unsigned int uint32_t;
typedef short int int16_t;
typedef int int32_t;

namespace cic{
    /*
     * package information for S-helmet
    */
    #pragma pack(push)
    #pragma pack(1)
        typedef struct rbNode{
            uint8_t  id;           //Node ID
            uint8_t  count;        //count
            float  ac_x;         //accelerator x
            float  ac_y;         //accelerator y
            float  ac_z;         //accelerator z
            uint16_t crc16Res;     //checksum value
        }Node;
    #pragma pack(pop)

    class CIC{
    public:
        CIC(int com_port);
        //Close the connection
        ~CIC();

    public:
        bool connectSerial();
        bool getData();

    public:
        int com_port_ = 10;
        int node_num_;
        Serial* SP_;

    public:
        Node nodeInfo[4];
        int curr_index = -1;
        bool new_data_ = false;
    };
}

#endif // CIC_H
