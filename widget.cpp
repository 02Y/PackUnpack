#include "widget.h"
#include "ui_widget.h"
#include <QDebug>
#include "packunpack.h"
#include <QMessageBox>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    setWindowTitle(QString::fromUtf8("打包解包小工具"));
    setWindowFlags(windowFlags()&~Qt::WindowMaximizeButtonHint);   //禁用最大化按钮
    setFixedSize(this->width(), this->height());       //禁止拖动窗口大小
}


Widget::~Widget()
{
    delete ui;
}


//转换为十进制数据
static char convertHexChar(char ch)
{
    if((ch >= '0') && (ch <= '9'))
    {
        return ch - 0x30;       //0x30 十进制是48，ASCII码值中为'0'   //得到对应的十进制数值(0~9)
    }
    else if((ch >= 'A') && (ch <= 'F'))
    {
        return ch - 'A' + 10;   //得到对应的十进制数值(10~15)
    }
    else if((ch >= 'a') && (ch <= 'f'))
    {
        return ch - 'a' + 10;   //得到对应的十进制数值(10~15)
    }
    else
    {
        return (-1);           //ch 为其他字符时, 不在16进制数范围内
    }
}


//十六进制字符串转换为Byte数组
static QByteArray convertHexStrToByteArray(QString hexStr)     //3132
{
    QByteArray sendData;
    char lStr, hStr;
    int len = hexStr.length();
    int hexDataLen = 0;
    int highHexData, lowHexData;
    sendData.resize(len / 2);

    for(int i = 0; i < len;)
    {
        //判断索引处字符是否在(0x00~0xff)的范围内(Latin1字符集范围是0x00~0xFF)，如果在该范围内，返回对应的值，否则返回‘\0’
        hStr = hexStr.at(i).toLatin1();

        if(hStr == ' ')     //字符是空格
        {
            i++;
            continue;
        }

        i++;
        if(i >= len) break;   //当i值大于hexStr的长度时，跳出循环

        lStr = hexStr.at(i).toLatin1();        //高位数获取后获取低位数
        highHexData = convertHexChar(hStr);    //把高位数转化为十进制数并赋值给 highHexData
        lowHexData = convertHexChar(lStr);     //把低位数转化为十进制数并赋值给 lowHexData

        if((highHexData == -1) || (lowHexData == -1))
        {
            break;
        }
        else
        {
            highHexData = highHexData * 16 + lowHexData;    //16进制数转换为十进制数
        }

        i++;
        sendData[hexDataLen] = (char)highHexData;   //将 hexData 强制转换为字符赋值给 sendData 数组   // '1' '2'
        hexDataLen++;
    }

    sendData.resize(hexDataLen);       //将 sendData 数组的长度设定为 hexDataLen 当前的值，抹去没有使用到的字节
    return sendData;
}


void Widget::on_packButton_clicked()
{
    QList<QString> dataList;
    QList<uchar> dataTempList;

    dataList.append(ui->modIDLineEdit->text().toLatin1().toHex());    //返回字节数组的十六进制编码副本   //"12"  //"3132"
    dataList.append(ui->secIDLineEdit->text().toLatin1().toHex());
    qDebug() << "packButton click";

    QStringList packDinList = ui->packDinLineEdit->text().split(" ");
    qDebug() << packDinList.length();

    if(packDinList.length() != 6)
    {
        QMessageBox::information(Q_NULLPTR, tr("提示"), tr("打包数据长度错误"), "确定");
        return;
    }

    for(int i = 0; i < packDinList.length(); i++)
    {
        dataList.append(packDinList[i].toLatin1().toHex());
    }

    dataList.append(QString("00").toLatin1().toHex());
    dataList.append(QString("00").toLatin1().toHex());

    bool ok = false;

    for(int i = 0; i < dataList.count(); i++)
    {
        dataTempList.append(convertHexStrToByteArray(dataList.at(i)).toInt(&ok, 16));     //"12"->18
    }

    /* 打包 */
    mPackUnpack.packData(dataTempList);

    QString packDout;
    for(int i = 0 ; i < dataTempList.count(); i++)
    {
        packDout += QString::number(dataTempList.at(i), 16).right(4) + " ";
    }

    ui->packDoutLineEdit->setFont(QFont("NSimSun", 16));
    ui->packDoutLineEdit->setText(packDout);
}


void Widget::on_unpackButton_clicked()
{
    qDebug() << "UnpackButton click";

    QString unpackDin;   //解包输入
    unpackDin = ui->unpackDinLineEdit->text().trimmed();     //去掉前后空格
    QStringList unpackDinList = unpackDin.split(" ");

    if(unpackDinList.length() != 10)
    {
        QMessageBox::information(Q_NULLPTR, tr("提示"), tr("解包数据长度错误"), "确定");
        return;
    }

    bool ok;
    bool findPack = false;
    QList<uchar> unpackRsltList;   //解包数据链表
    QString unpackDout;            //解包输出
    for(int i = 0; i < unpackDinList.count(); i++)
    {
        qDebug() << convertHexStrToByteArray(unpackDinList.at(i).toLatin1().toHex()).toInt(&ok, 16);

        /* 解包 */
        findPack = mPackUnpack.unpackData(convertHexStrToByteArray(unpackDinList.at(i).toLatin1().toHex()).toInt(&ok, 16));

        if(findPack)
        {
            unpackRsltList = mPackUnpack.getUnpackRslt();

            for(int j = 0; j < unpackRsltList.count(); j++)
            {
                unpackDout += QString("%1").arg(unpackRsltList.at(j), 2, 16, QLatin1Char('0')) + " ";
            }

            ui->unpackDoutLineEdit->setFont(QFont("NSimSun", 16));
            ui->unpackDoutLineEdit->setText(unpackDout.toUpper());
            break;
        }
    }
}



