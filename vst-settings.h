#pragma once

#include <QDialog>
#include <memory>

#include "ui_vst-settings.h"

class VSTSettings : public QDialog {
	//Q_OBJECT

public:
	std::unique_ptr<Ui_VSTSettings> ui;
	VSTSettings(QWidget *parent);

	//void closeEvent(QCloseEvent *event) override;

public slots:
	void ShowHideDialog();
};