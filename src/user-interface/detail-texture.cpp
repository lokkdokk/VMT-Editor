#include "user-interface/detail-texture.h"

#include "view-helper.h"
#include "vmt/vmt-helper.h"
#include "logging/logging.h"

void detailtexture::reset(Ui::MainWindow *ui)
{
	ui->doubleSpinBox_detailScale->setValue(4.0);
	ui->doubleSpinBox_detailScaleY->setValue(4.0);
	ui->checkBox_detailScaleUniform->setChecked(true);

	// we fake a change to quickly disable all widgets
	processDetailTextureChange("", ui);
}

bool detailtexture::hasChanged(Ui::MainWindow *ui)
{
	return !(ui->lineEdit_detail->text().isEmpty() &&
			 ui->comboBox_detailBlendMode->currentIndex() == 0 &&
			 utils::equal(ui->doubleSpinBox_detailScale, "4.") &&
			 utils::equal(ui->doubleSpinBox_detailScaleY, "4.") &&
			 utils::equal(ui->doubleSpinBox_detailAmount, "1.") &&
			 utils::equal(ui->doubleSpinBox_detailAmount2, "1.") &&
			 utils::equal(ui->doubleSpinBox_detailAmount3, "1.") &&
			 utils::equal(ui->doubleSpinBox_detailAmount4, "1."));
}

void detailtexture::processDetailTextureChange(const QString &text,
		Ui::MainWindow *ui)
{
	// TODO: Are those all widgets in the groupbox or why exactly those?
	const bool enable = !text.isEmpty();

	ui->label_5->setEnabled(enable);
	ui->comboBox_detailBlendMode->setEnabled(enable);
	ui->label_detailScale->setEnabled(enable);
	ui->label_detailAmount->setEnabled(enable);
	ui->doubleSpinBox_detailAmount->setEnabled(enable);
	ui->horizontalSlider_detailAmount->setEnabled(enable);
	ui->doubleSpinBox_detailScale->setEnabled(enable);
	ui->checkBox_detailScaleUniform->setEnabled(enable);
	ui->label_detailAmount2->setEnabled(enable);
	ui->label_detailAmount3->setEnabled(enable);
	ui->label_detailAmount4->setEnabled(enable);
	ui->horizontalSlider_detailAmount2->setEnabled(enable);
	ui->horizontalSlider_detailAmount3->setEnabled(enable);
	ui->horizontalSlider_detailAmount4->setEnabled(enable);
	ui->doubleSpinBox_detailAmount2->setEnabled(enable);
	ui->doubleSpinBox_detailAmount3->setEnabled(enable);
	ui->doubleSpinBox_detailAmount4->setEnabled(enable);

	if (!ui->checkBox_detailScaleUniform->isChecked()) {
		ui->doubleSpinBox_detailScaleY->setEnabled(enable);
	}
}

void detailtexture::toggledUniformScale(bool checked, Ui::MainWindow *ui)
{
	if (!ui->lineEdit_detail->text().isEmpty()) {
		ui->doubleSpinBox_detailScaleY->setEnabled(!checked);
	}
}

QString detailtexture::param::initialize(Ui::MainWindow *ui, VmtFile *vmt)
{
	const QString raw = vmt->parameters.take("$detail");

	if (!raw.isEmpty()) {
		if (vmt->parameters.contains("$seamless_scale")) {
			logging::error("$detail and $seamless_scale are not "
				"supported at the same time!", ui);
		}

		vmt->state.detailEnabled = true;
		vmt->state.showDetail = true;
	}

	return raw;
}

void processDetailScaleAsTuple(const QString &parameter, const QString &raw,
		Ui::MainWindow *ui)
{
	const utils::DoubleTuple tuple = utils::toDoubleTuple(raw, 2);

	if (tuple.valid) {
		const QString x = tuple.strings.at(0);
		const QString y = tuple.strings.at(1);
		if (x == "4." && y == "4.") {
			logging::error(parameter + " has default value: [4 4]",
				ui);
		} else {
			double xd = tuple.values.at(0);
			double yd = tuple.values.at(1);
			ui->doubleSpinBox_detailScale->setValue(xd);
			ui->doubleSpinBox_detailScaleY->setValue(yd);
			ui->checkBox_detailScaleUniform->setChecked(false);
		}
	} else {
		logging::error(parameter + " has invalid value: " + raw,
			ui);
	}
}

/*!
 * The parameter can be $detailscale 4.0 or $detailscale [4 3] and we need to
 * handle both cases correctly!
 */
void processDetailScale(const QString &parameter, const QString &value,
		Ui::MainWindow *ui)
{
	bool logErrors = false;
	const utils::DoubleResult result = utils::parseDouble(parameter, value,
			"4", ui, logErrors);

	if (result.invalid) {
		processDetailScaleAsTuple(parameter, result.string, ui);

	} else {
		if (result.notDefault) {
			ui->doubleSpinBox_detailScale->setValue(result.value);
			ui->checkBox_detailScaleUniform->setChecked(true);
			if (!ui->lineEdit_detail->text().isEmpty()) {
				ui->doubleSpinBox_detailScaleY->setEnabled(
					false);
			}
		} else {
			logging::error(parameter + " has default value: 4.0",
				ui);
		}
	}
}

void detailtexture::param::parse(const detailtexture::Parameter &parameter,
	Ui::MainWindow *ui, VmtFile *vmt)
{
	#define CHECK(p, m) { \
		if (vmt->parameters.contains(p)) { \
			if (!vmt->state.detailEnabled) { \
				ERROR(p " only works with $detail") \
			} \
			vmt->state.showDetail = true; \
			m(p, vmt->parameters.take(p), ui); \
		} \
		break; \
	}

	switch (parameter) {
	case detailtexture::detailscale:
		CHECK("$detailscale", processDetailScale)
	}
}
