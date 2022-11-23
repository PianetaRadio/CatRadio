#include "dialogradioinfo.h"
#include "ui_dialogradioinfo.h"
#include "rig.h"

#include <QString>


DialogRadioInfo::DialogRadioInfo(RIG *rig, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogRadioInfo)
{
    ui->setupUi(this);
    my_rig = rig;
    QString text;
    text = "Model: ";
    text.append(QString::number(my_rig->caps->rig_model));
    ui->plainTextEdit_RadioInfo->appendPlainText(text);

    text = "Model name: ";
    text.append(my_rig->caps->model_name);
    ui->plainTextEdit_RadioInfo->appendPlainText(text);

    text = "Mfg name: ";
    text.append(my_rig->caps->mfg_name);
    ui->plainTextEdit_RadioInfo->appendPlainText(text);

    text = "Backend version: ";
    text.append(my_rig->caps->version);
    ui->plainTextEdit_RadioInfo->appendPlainText(text);

    text = "Backend status: ";
    text.append(rig_strstatus(my_rig->caps->status));
    ui->plainTextEdit_RadioInfo->appendPlainText(text);

    text = "Rig type: ";
    switch (my_rig->caps->rig_type)
    {
    case RIG_TYPE_OTHER:
        text.append("Other");
        break;
    case RIG_FLAG_RECEIVER:
        text.append("Receiver");
        break;
    case RIG_FLAG_TRANSMITTER:
        text.append("Transmitter");
        break;
    case RIG_FLAG_SCANNER:
        text.append("Scanner");
        break;
    case RIG_FLAG_MOBILE:
        text.append("Mobile");
        break;
    case RIG_FLAG_HANDHELD:
        text.append("Handheld");
        break;
    case RIG_FLAG_COMPUTER:
        text.append("Computer");
        break;
    case RIG_FLAG_TRANSCEIVER:
        text.append("Transceiver");
        break;
    case RIG_FLAG_TRUNKING:
         text.append("Trunking scanner");
         break;
    case RIG_FLAG_APRS:
         text.append("APRS");
         break;
    case RIG_FLAG_TNC:
         text.append("TNC");
         break;
    case RIG_FLAG_DXCLUSTER:
         text.append("DxCluster");
         break;
    case RIG_FLAG_TUNER:
         text.append("Tuner");
         break;
    default:
        text.append("Unknown");
    }
    ui->plainTextEdit_RadioInfo->appendPlainText(text);

    text = "Port type: ";
    switch (my_rig->caps->port_type)
    {
    case RIG_PORT_SERIAL:
        text.append("RS-232");
        break;
    case RIG_PORT_PARALLEL:
        text.append("Parallel");
        break;
    case RIG_PORT_DEVICE:
        text.append("Device driver");
        break;
    case RIG_PORT_USB:
        text.append("USB");
        break;
    case RIG_PORT_NETWORK:
        text.append("Network link");
        break;
    case RIG_PORT_UDP_NETWORK:
        text.append("UDP Network link");
        break;
    case RIG_PORT_NONE:
        text.append("None");
        break;
    default:
        text.append("Unknown");
    }
    ui->plainTextEdit_RadioInfo->appendPlainText(text);

    if (my_rig->caps->port_type == RIG_PORT_SERIAL)
    {
        text = QString("Serial speed: %1...%2 bauds, %3%4%5 %6")
                .arg(my_rig->caps->serial_rate_min)
                .arg(my_rig->caps->serial_rate_max)
                .arg(my_rig->caps->serial_data_bits)
                .arg(my_rig->caps->serial_parity == RIG_PARITY_NONE ? 'N' :
                                                                      (my_rig->caps->serial_parity == RIG_PARITY_ODD ? 'O' :
                                                                                                                       (my_rig->caps->serial_parity == RIG_PARITY_EVEN ? 'E' :
                                                                                                                                                                         (my_rig->caps->serial_parity == RIG_PARITY_MARK ? 'M' : 'S'))))
                .arg(my_rig->caps->serial_stop_bits)
                .arg(my_rig->caps->serial_handshake == RIG_HANDSHAKE_NONE ? "" :
                                                                            (my_rig->caps->serial_handshake == RIG_HANDSHAKE_XONXOFF ? "XONXOFF" : "CTS/RTS"));
        ui->plainTextEdit_RadioInfo->appendPlainText(text);
    }
}

DialogRadioInfo::~DialogRadioInfo()
{
    delete ui;
}
