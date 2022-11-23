#include "dialogradioinfo.h"
#include "ui_dialogradioinfo.h"

#include "rig.h"


struct rig_type_s
{
    int type;
    char *description;
};

struct rig_type_s rig_type[] =
{
    {RIG_TYPE_OTHER, "Other"},
    {RIG_FLAG_RECEIVER, "Receiver"},
    {RIG_FLAG_TRANSMITTER, "Transmitter"},
    {RIG_FLAG_SCANNER, "Scanner"},
    {RIG_FLAG_MOBILE, "Mobile"},
    {RIG_FLAG_HANDHELD, "Handheld"},
    {RIG_FLAG_COMPUTER, "Computer"},
    {RIG_FLAG_TRANSCEIVER, "Transceiver"},
    {RIG_FLAG_TRUNKING, "Trunking scanner"},
    {RIG_FLAG_APRS, "APRS"},
    {RIG_FLAG_TNC, "TNC"},
    {RIG_FLAG_DXCLUSTER, "DxCluster"},
    {RIG_FLAG_TUNER, "Tuner"},
    {-1, "Unknown"}
};


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
    text.append(rig_strstatus(caps->status));
    ui->plainTextEdit_RadioInfo->appendPlainText(text);
        
    text = "Rig type: ";
    for (int i = 0; rig_type[i].type != -1; ++i)
    {
        if ((rig_type[i].type & caps->rig_type) == rig_type[i].type) text.append(rig_type[i].description);
    }
    ui->plainTextEdit_RadioInfo->appendPlainText(text);
        
    text = "Port type: ";
    switch (caps->port_type)
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
    if (caps->port_type == RIG_PORT_SERIAL)
    {
        text = QString("Serial speed: %1..%2 bauds, %3%4%5 %6").arg(
                caps->serial_rate_min,
                caps->serial_rate_max,
                caps->serial_data_bits,
                caps->serial_parity == RIG_PARITY_NONE ? 'N' :
                caps->serial_parity == RIG_PARITY_ODD ? 'O' :
                caps->serial_parity == RIG_PARITY_EVEN ? 'E' :
                caps->serial_parity == RIG_PARITY_MARK ? 'M' : 'S',
                caps->serial_stop_bits,
                caps->serial_handshake == RIG_HANDSHAKE_NONE ? "" :
                (caps->serial_handshake == RIG_HANDSHAKE_XONXOFF ? "XONXOFF" : "CTS/RTS")
         );
        ui->plainTextEdit_RadioInfo->appendPlainText(text);
    }
}

DialogRadioInfo::~DialogRadioInfo()
{
    delete ui;
}
