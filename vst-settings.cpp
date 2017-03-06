#include "vst-settings.h"

VSTSettings::VSTSettings(QWidget *parent)
	: QDialog(parent),
	ui(new Ui_VSTSettings)
{
	ui->setupUi(this);
}

void VSTSettings::ShowHideDialog()
{
	if (!isVisible()) {
		setVisible(true);
		//QTimer::singleShot(250, this, SLOT(show()));
	} else {
		setVisible(false);
		//QTimer::singleShot(250, this, SLOT(hide()));
	}
}