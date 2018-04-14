#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "math.h"
#include <algorithm>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    inRotationUpdate(false),
    inScaleUpdate(false)
{
    ui->setupUi(this);
    connect(ui->mainView, &MainView::rotationChanged, this, &MainWindow::on_mainView_rotationChanged);
    connect(ui->mainView, &MainView::scaleChanged, this, &MainWindow::on_mainView_scaleChanged);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// --- Functions that listen for widget events
// forewards to the mainview

void MainWindow::on_ResetRotationButton_clicked(bool checked)
{
    Q_UNUSED(checked);
    ui->RotationDialX->setValue(0);
    ui->RotationDialY->setValue(0);
    ui->RotationDialZ->setValue(0);
    ui->mainView->setRotation(0, 0, 0);
}

void MainWindow::on_RotationDialX_valueChanged(int value)
{
    if(inRotationUpdate)
    {
        return;
    }

    inRotationUpdate = true;
    ui->mainView->setRotation(value,
                              ui->RotationDialY->value(),
                              ui->RotationDialZ->value());
    inRotationUpdate = false;
    ui->mainView->update();
}

void MainWindow::on_RotationDialY_valueChanged(int value)
{
    if(inRotationUpdate)
    {
        return;
    }

    inRotationUpdate = true;
    ui->mainView->setRotation(ui->RotationDialX->value(),
                              value,
                              ui->RotationDialZ->value());
    inRotationUpdate = false;
    ui->mainView->update();
}

void MainWindow::on_RotationDialZ_valueChanged(int value)
{
    if(inRotationUpdate)
    {
        return;
    }

    inRotationUpdate = true;
    ui->mainView->setRotation(ui->RotationDialX->value(),
                              ui->RotationDialY->value(),
                              value);
    inRotationUpdate = false;
    ui->mainView->update();
}

void MainWindow::on_ResetScaleButton_clicked(bool checked)
{
    Q_UNUSED(checked);
    ui->ScaleSlider->setValue(100);
    ui->mainView->setScale(100);
    ui->mainView->update();
}

void MainWindow::on_ScaleSlider_valueChanged(int value)
{
    if(inScaleUpdate)
    {
        return;
    }

    inScaleUpdate = true;
    ui->mainView->setScale(value);
    inScaleUpdate = false;

    ui->mainView->update();
}

void MainWindow::on_PhongButton_toggled(bool checked)
{
    if (checked)
    {
        ui->mainView->setShadingMode(MainView::PHONG);
        ui->mainView->update();
    }
}

void MainWindow::on_NormalButton_toggled(bool checked)
{
    if (checked)
    {
        ui->mainView->setShadingMode(MainView::NORMAL);
        ui->mainView->update();
    }
}

void MainWindow::on_GouraudButton_toggled(bool checked)
{
    if (checked)
    {
        ui->mainView->setShadingMode(MainView::GOURAUD);
        ui->mainView->update();
    }
}

void MainWindow::on_mainView_rotationChanged(QVector3D rotation)
{
    // Rounding in interval [0, 360) + epsilon error
    // Could result in a value less than 0, so cast to integer alone is unsafe.
    // Error is small (definitely smaller than 360), so we can add 360 making it positive
    // Then use mod to cut it off, also positive epsilon error can result in value >= 360, which is also taken care of with mod.
    int x, y, z;
    x = (((int)(round(rotation.x()))) + 360) % 360;
    y = (((int)(round(rotation.y()))) + 360) % 360;
    z = (((int)(round(rotation.z()))) + 360) % 360;

    inRotationUpdate = true;
    ui->RotationDialX->setValue(x);
    ui->RotationDialY->setValue(y);
    ui->RotationDialZ->setValue(z);
    inRotationUpdate = false;
}

void MainWindow::on_mainView_scaleChanged(float factor)
{
    inScaleUpdate = true;
    ui->ScaleSlider->setValue(round(factor));
    inScaleUpdate = false;
}

void MainWindow::on_LockSun_toggled(bool checked)
{
    if(checked)
    {
        ui->mainView->setCameraLock(0);
    }
}
void MainWindow::on_LockEarth_toggled(bool checked)
{
    if(checked)
    {
        ui->mainView->setCameraLock(2);
    }
}
void MainWindow::on_LockMars_toggled(bool checked)
{
    if(checked)
    {
        ui->mainView->setCameraLock(3);
    }
}
void MainWindow::on_LockHorse_toggled(bool checked)
{
    if(checked)
    {
        ui->mainView->setCameraLock(4);
    }
}
void MainWindow::on_LockHorse2_toggled(bool checked)
{
    if(checked)
    {
        ui->mainView->setCameraLock(5);
    }
}
