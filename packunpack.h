#ifndef PACKUNPACK_H
#define PACKUNPACK_H
#include <QList>

class PackUnpack
{
public:
    PackUnpack();

    int packData(QList<uchar>& listPack);   //对数据进行打包
    bool unpackData(uchar data);            //对数据进行解包
    QList<uchar> getUnpackRslt();           //获取解包后数据

    const int MAX_PACK_LEN = 10;

private:
    void packWithCheckSum(QList<uchar> &pack);      //带校验和的数据打包
    bool unpackWithCheckSum(QList<uchar> &pack);    //带校验和的数据解包

    int mPackLen;     //数据包长度
    int mGotPackId;   //获取到ID的标志，获得正确的ID即为TRUE，否则为FALSE
    int mRestByteNum; //剩余字节数

    //定义m_listBuf
    //m_listBuf[0]  : 对应packId
    //m_listBuf[1-8]: 对应arrData(数据头+二级ID+数据)
    //m_listBuf[9]  : 对应checkSum
    QList<uchar> mListBuf;        //存储完整包
};

#endif // PACKUNPACK_H
