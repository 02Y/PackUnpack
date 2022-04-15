#include "packunpack.h"



PackUnpack::PackUnpack()
{
    for(int i =0; i < MAX_PACK_LEN; i++)
    {
        mListBuf.insert(i, 0);    //ID、数据头、数据及校验和均清零
    }

    mPackLen = 0;           //数据包的长度默认为0
    mGotPackId = false;     //获取到数据包ID标志默认为false，即尚未获取到ID
    mRestByteNum = 0;       //剩余的字节数默认为 0
}


//对数据进行打包
int PackUnpack::packData(QList<uchar> &listPack)
{
    int len = 0;                    //是否经过打包
    if(listPack[0] < 0x80)          //包ID必须在0x00-0x7F之间, listPack[0]为包ID
    {
        packWithCheckSum(listPack);
        len++;
    }

    return len;
}


//对数据进行解包
bool PackUnpack::unpackData(uchar data)
{
    bool findPack = false;

    if(mGotPackId)             //已接收到包ID
    {
        if(0x80 <= data)       //包数据（非包ID）必须大于或等于0x80（因为字节最高位已置为1）
        {
            mListBuf[mPackLen] = data;     //第一个字节是包ID 依次存储
            mPackLen++;                    //包长自增
            mRestByteNum--;                //剩余字节数自减

            if((0 >= mRestByteNum) && (10 == mPackLen))       //已经接收到完整的数据包
            {
                findPack = unpackWithCheckSum(mListBuf);      //接收到完整数据包后尝试解包
                mGotPackId = false;        //清除获取到包ID的标志，方便重新判断下一个数据包
            }
        }
        else mGotPackId = false;
    }
    else if(data < 0x80)        //包ID最大为 0x7F -> 0111 1111
    {
        mRestByteNum = 9;       //除去包ID，要读取的字节数
        mPackLen = 1;           //已经读取到包ID
        mListBuf[0] = data;     //数据包ID
        mGotPackId = true;      //获取到包ID标志置为true
    }

    return findPack;
}


//获取解包后的数据
QList<uchar> PackUnpack::getUnpackRslt()
{
    return (mListBuf);
}


//带校验和的数据打包
//pack 为待打包的数据
void PackUnpack::packWithCheckSum(QList<uchar> &pack) //目前包格式：模块ID 二级ID 1 2 3 4 5 6 【00】 【00】
{
    uchar dataHead;     //数据头，在数据包的第二个位置，即ID之后
    uchar checkSum;     //校验和，在数据包的最后一个位置

    if(MAX_PACK_LEN != pack.length())    //数据包长度不够（不足10）
    {
        return;
    }

    checkSum = pack[0];  //取出模块ID加到校验和
    dataHead = 0;        //数据头清零

    for(int i = 8; i > 1; i--)
    {
        dataHead <<= 1;    //1~~~~~~~  //低七位存放（二级ID、数据1~6）的最高位    0000 0010

        //对数据进行最高位置1操作，并将数据右移一位（因为数据头要插入在二级ID之前）
        pack.replace(i, (uchar)((pack[i - 1]) | 0x80));    //将索引位置i的值替换为value  //0x80 -> 1000 0000

        checkSum += pack.at(i);    //数据加到校验和
        dataHead |= (uchar)(((pack[i - 1]) & 0x80) >> 7);    //取出原始数据的最高位，放在数据头最低位 //(0000 0000 或 0000 0001)
    }

    pack.replace(1, (uchar)(dataHead | 0x80));    //数据头在数据包的第二个位置，仅次于包头，数据头的最高位也要置为1
    checkSum += pack[1];     //将数据头加到校验和
    pack.replace(9, (uchar)(checkSum) | 0x80);      //校验和的最高位也要置为1
}


//带校验和的数据解包
//pack 为待解包的数据
bool PackUnpack::unpackWithCheckSum(QList<uchar> &pack)
{
    uchar dataHead;     //数据头，在数据包的第二个位置，即ID之后
    uchar checkSum;     //先求出校验和，判断包数据是否正常

    if(MAX_PACK_LEN != pack.length())    //数据包长度不够（不足10）
    {
        return false;
    }

    checkSum = pack[0];    //取出模块ID加到校验和
    dataHead = pack[1];    //取出数据头
    checkSum += dataHead;  //数据头加到校验和

    for(int i = 1; i < 8; i++)
    {
        checkSum += pack[i + 1];

        //比如1001 0100 & 0111 1111(0x7f) -> 0001 0100       //1 << 7 -> 1000 0000
        pack[i] = (uchar)((pack[i + 1] & 0x7f) | ((dataHead & 0x1) << 7));    //还原有效的8位数据
        dataHead >>= 1;
    }

     //判断ID、数据头和数据求和的结果（低七位）是否与校验和的低七位相等，如果不等返回0
    if((checkSum & 0x7f) != ((pack[9] & 0x7f))) return false;
    for(int j = 0; j < 2; j++) pack.removeLast();      //抹去最后两个残留数据

    return true;
}




