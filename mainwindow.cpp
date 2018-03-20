/*
 * Accelerometer Chart
 * Copyright (C) 2015 Solution Engineering, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "Serial/inc/cic.h"
#include <QComboBox>

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow),
  cic_(nullptr),
  settings(nullptr),
  //settings(QStandardPaths::writableLocation(QStandardPaths::DataLocation)+ QDir::separator() + "settings.ini", QSettings::IniFormat),
  first(true),
  started(true)
{
  srand(QDateTime::currentDateTime().toTime_t());   
  ui->setupUi(this);

  const QString configPath = "settings.ini";
  settings = new QSettings(configPath, QSettings::IniFormat, this);
  
  //Customplot 1
  ui->customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes |
                                  QCP::iSelectLegend | QCP::iSelectPlottables);


  ui->customPlot->xAxis->setBasePen(QPen(Qt::white, 1));
  ui->customPlot->yAxis->setBasePen(QPen(Qt::white, 1));
  ui->customPlot->xAxis->setTickPen(QPen(Qt::white, 1));
  ui->customPlot->yAxis->setTickPen(QPen(Qt::white, 1));
  ui->customPlot->xAxis->setSubTickPen(QPen(Qt::white, 1));
  ui->customPlot->yAxis->setSubTickPen(QPen(Qt::white, 1));
  ui->customPlot->xAxis->setTickLabelColor(Qt::white);
  ui->customPlot->yAxis->setTickLabelColor(Qt::white);
  ui->customPlot->xAxis->grid()->setPen(QPen(QColor(140, 140, 140), 1, Qt::DotLine));
  ui->customPlot->yAxis->grid()->setPen(QPen(QColor(140, 140, 140), 1, Qt::DotLine));
  ui->customPlot->xAxis->grid()->setSubGridPen(QPen(QColor(80, 80, 80), 1, Qt::DotLine));
  ui->customPlot->yAxis->grid()->setSubGridPen(QPen(QColor(80, 80, 80), 1, Qt::DotLine));
  ui->customPlot->xAxis->grid()->setSubGridVisible(true);
  ui->customPlot->yAxis->grid()->setSubGridVisible(true);
  ui->customPlot->xAxis->grid()->setZeroLinePen(Qt::NoPen);
  ui->customPlot->yAxis->grid()->setZeroLinePen(Qt::NoPen);

  // Axes
  ui->customPlot->xAxis->setTickLabelType(QCPAxis::ltDateTime);
  ui->customPlot->xAxis->setDateTimeFormat("hh:mm:ss");
  ui->customPlot->xAxis->setAutoTickStep(false);
  ui->customPlot->xAxis->setTickStep(2);
//  ui->customPlot->xAxis->setLabel("Time");
  ui->customPlot->xAxis->setLabelColor(Qt::white);
  ui->customPlot->xAxis->setLabelPadding(0);
  ui->customPlot->xAxis->setSelectableParts(QCPAxis::spAxis);

  ui->customPlot->yAxis->setRange(-20, 20);
  ui->customPlot->yAxis->setLabel("Acceleration (m/s^2)");
  ui->customPlot->yAxis->setLabelColor(Qt::white);
  ui->customPlot->yAxis->setLabelPadding(0);
  ui->customPlot->yAxis->setSelectableParts(QCPAxis::spAxis);

  ui->customPlot->axisRect()->setupFullAxesBox();

  // Legend
  ui->customPlot->legend->setVisible(true);
  QFont legendFont = font();
  legendFont.setPointSize(10);
  ui->customPlot->legend->setFont(legendFont);
  ui->customPlot->legend->setSelectedFont(legendFont);
  ui->customPlot->legend->setSelectableParts(QCPLegend::spNone);

  QLinearGradient plotGradient;
  plotGradient.setStart(0, 0);
  plotGradient.setFinalStop(0, 350);
  plotGradient.setColorAt(0, QColor(80, 80, 80));
  plotGradient.setColorAt(1, QColor(50, 50, 50));
  ui->customPlot->setBackground(plotGradient);

  // X, Y, Z reading graphs
  ui->customPlot->addGraph(); // blue line
  ui->customPlot->graph(0)->setPen(QPen(Qt::blue));
  ui->customPlot->graph(0)->setName("X");
  ui->customPlot->graph(0)->setSelectable(false);
  ui->customPlot->addGraph(); // red line
  ui->customPlot->graph(1)->setPen(QPen(Qt::red));
  ui->customPlot->graph(1)->setName("Y");
  ui->customPlot->graph(1)->setSelectable(false);
  ui->customPlot->addGraph(); // green line
  ui->customPlot->graph(2)->setPen(QPen(Qt::green));
  ui->customPlot->graph(2)->setName("Z");
  ui->customPlot->graph(2)->setSelectable(false);

  // Scale in a particular direction when an axis is selected
  connect(ui->customPlot, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(mousePress()));
  connect(ui->customPlot, SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(mouseWheel()));

  // Make bottom and left axes transfer their ranges to top and right axes:
  connect(ui->customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlot->xAxis2, SLOT(setRange(QCPRange)));
  connect(ui->customPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlot->yAxis2, SLOT(setRange(QCPRange)));

  // Adjust the tick marks on x-axis scale
  connect(ui->customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), this, SLOT(xAxisChanged(QCPRange)));

  // Context menu popup
  ui->customPlot->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(ui->customPlot, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenuRequest(QPoint)));
  /******************************************************************/
  ui->customPlot_1->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes |
                                  QCP::iSelectLegend | QCP::iSelectPlottables);

  ui->customPlot_1->xAxis->setBasePen(QPen(Qt::white, 1));
  ui->customPlot_1->yAxis->setBasePen(QPen(Qt::white, 1));
  ui->customPlot_1->xAxis->setTickPen(QPen(Qt::white, 1));
  ui->customPlot_1->yAxis->setTickPen(QPen(Qt::white, 1));
  ui->customPlot_1->xAxis->setSubTickPen(QPen(Qt::white, 1));
  ui->customPlot_1->yAxis->setSubTickPen(QPen(Qt::white, 1));
  ui->customPlot_1->xAxis->setTickLabelColor(Qt::white);
  ui->customPlot_1->yAxis->setTickLabelColor(Qt::white);
  ui->customPlot_1->xAxis->grid()->setPen(QPen(QColor(140, 140, 140), 1, Qt::DotLine));
  ui->customPlot_1->yAxis->grid()->setPen(QPen(QColor(140, 140, 140), 1, Qt::DotLine));
  ui->customPlot_1->xAxis->grid()->setSubGridPen(QPen(QColor(80, 80, 80), 1, Qt::DotLine));
  ui->customPlot_1->yAxis->grid()->setSubGridPen(QPen(QColor(80, 80, 80), 1, Qt::DotLine));
  ui->customPlot_1->xAxis->grid()->setSubGridVisible(true);
  ui->customPlot_1->yAxis->grid()->setSubGridVisible(true);
  ui->customPlot_1->xAxis->grid()->setZeroLinePen(Qt::NoPen);
  ui->customPlot_1->yAxis->grid()->setZeroLinePen(Qt::NoPen);

  // Axes
  ui->customPlot_1->xAxis->setTickLabelType(QCPAxis::ltDateTime);
  ui->customPlot_1->xAxis->setDateTimeFormat("hh:mm:ss");
  ui->customPlot_1->xAxis->setAutoTickStep(false);
  ui->customPlot_1->xAxis->setTickStep(2);
  //ui->customPlot_1->xAxis->setLabel("Time");
  ui->customPlot_1->xAxis->setLabelColor(Qt::white);
  ui->customPlot_1->xAxis->setLabelPadding(0);
  ui->customPlot_1->xAxis->setSelectableParts(QCPAxis::spAxis);

  ui->customPlot_1->yAxis->setRange(-20, 20);
  ui->customPlot_1->yAxis->setLabel("Acceleration (m/s^2)");
  ui->customPlot_1->yAxis->setLabelColor(Qt::white);
  ui->customPlot_1->yAxis->setLabelPadding(0);
  ui->customPlot_1->yAxis->setSelectableParts(QCPAxis::spAxis);

  ui->customPlot_1->axisRect()->setupFullAxesBox();

  // Legend
  ui->customPlot_1->legend->setVisible(true);
  legendFont.setPointSize(10);
  ui->customPlot_1->legend->setFont(legendFont);
  ui->customPlot_1->legend->setSelectedFont(legendFont);
  ui->customPlot_1->legend->setSelectableParts(QCPLegend::spNone);

  plotGradient.setStart(0, 0);
  plotGradient.setFinalStop(0, 350);
  plotGradient.setColorAt(0, QColor(80, 80, 80));
  plotGradient.setColorAt(1, QColor(50, 50, 50));
  ui->customPlot_1->setBackground(plotGradient);

  // X, Y, Z reading graphs
  ui->customPlot_1->addGraph(); // blue line
  ui->customPlot_1->graph(0)->setPen(QPen(Qt::blue));
  ui->customPlot_1->graph(0)->setName("X");
  ui->customPlot_1->graph(0)->setSelectable(false);
  ui->customPlot_1->addGraph(); // red line
  ui->customPlot_1->graph(1)->setPen(QPen(Qt::red));
  ui->customPlot_1->graph(1)->setName("Y");
  ui->customPlot_1->graph(1)->setSelectable(false);
  ui->customPlot_1->addGraph(); // green line
  ui->customPlot_1->graph(2)->setPen(QPen(Qt::green));
  ui->customPlot_1->graph(2)->setName("Z");
  ui->customPlot_1->graph(2)->setSelectable(false);

  // Scale in a particular direction when an axis is selected
  connect(ui->customPlot_1, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(mousePress()));
  connect(ui->customPlot_1, SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(mouseWheel()));

  // Make bottom and left axes transfer their ranges to top and right axes:
  connect(ui->customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlot_1->xAxis2, SLOT(setRange(QCPRange)));
  connect(ui->customPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlot_1->yAxis2, SLOT(setRange(QCPRange)));

  // Adjust the tick marks on x-axis scale
  connect(ui->customPlot_1->xAxis, SIGNAL(rangeChanged(QCPRange)), this, SLOT(xAxisChanged(QCPRange)));

  // Context menu popup
  ui->customPlot_1->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(ui->customPlot_1, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenuRequest(QPoint)));
  /******************************************************************/
  ui->customPlot_2->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes |
                                  QCP::iSelectLegend | QCP::iSelectPlottables);

  ui->customPlot_2->xAxis->setBasePen(QPen(Qt::white, 1));
  ui->customPlot_2->yAxis->setBasePen(QPen(Qt::white, 1));
  ui->customPlot_2->xAxis->setTickPen(QPen(Qt::white, 1));
  ui->customPlot_2->yAxis->setTickPen(QPen(Qt::white, 1));
  ui->customPlot_2->xAxis->setSubTickPen(QPen(Qt::white, 1));
  ui->customPlot_2->yAxis->setSubTickPen(QPen(Qt::white, 1));
  ui->customPlot_2->xAxis->setTickLabelColor(Qt::white);
  ui->customPlot_2->yAxis->setTickLabelColor(Qt::white);
  ui->customPlot_2->xAxis->grid()->setPen(QPen(QColor(140, 140, 140), 1, Qt::DotLine));
  ui->customPlot_2->yAxis->grid()->setPen(QPen(QColor(140, 140, 140), 1, Qt::DotLine));
  ui->customPlot_2->xAxis->grid()->setSubGridPen(QPen(QColor(80, 80, 80), 1, Qt::DotLine));
  ui->customPlot_2->yAxis->grid()->setSubGridPen(QPen(QColor(80, 80, 80), 1, Qt::DotLine));
  ui->customPlot_2->xAxis->grid()->setSubGridVisible(true);
  ui->customPlot_2->yAxis->grid()->setSubGridVisible(true);
  ui->customPlot_2->xAxis->grid()->setZeroLinePen(Qt::NoPen);
  ui->customPlot_2->yAxis->grid()->setZeroLinePen(Qt::NoPen);

  // Axes
  ui->customPlot_2->xAxis->setTickLabelType(QCPAxis::ltDateTime);
  ui->customPlot_2->xAxis->setDateTimeFormat("hh:mm:ss");
  ui->customPlot_2->xAxis->setAutoTickStep(false);
  ui->customPlot_2->xAxis->setTickStep(2);
//  ui->customPlot_2->xAxis->setLabel("Time");
  ui->customPlot_2->xAxis->setLabelColor(Qt::white);
  ui->customPlot_2->xAxis->setLabelPadding(0);
  ui->customPlot_2->xAxis->setSelectableParts(QCPAxis::spAxis);

  ui->customPlot_2->yAxis->setRange(-20, 20);
  ui->customPlot_2->yAxis->setLabel("Acceleration (m/s^2)");
  ui->customPlot_2->yAxis->setLabelColor(Qt::white);
  ui->customPlot_2->yAxis->setLabelPadding(0);
  ui->customPlot_2->yAxis->setSelectableParts(QCPAxis::spAxis);

  ui->customPlot_2->axisRect()->setupFullAxesBox();

  // Legend
  ui->customPlot_2->legend->setVisible(true);
  legendFont.setPointSize(10);
  ui->customPlot_2->legend->setFont(legendFont);
  ui->customPlot_2->legend->setSelectedFont(legendFont);
  ui->customPlot_2->legend->setSelectableParts(QCPLegend::spNone);

  plotGradient.setStart(0, 0);
  plotGradient.setFinalStop(0, 350);
  plotGradient.setColorAt(0, QColor(80, 80, 80));
  plotGradient.setColorAt(1, QColor(50, 50, 50));
  ui->customPlot_2->setBackground(plotGradient);

  // X, Y, Z reading graphs
  ui->customPlot_2->addGraph(); // blue line
  ui->customPlot_2->graph(0)->setPen(QPen(Qt::blue));
  ui->customPlot_2->graph(0)->setName("X");
  ui->customPlot_2->graph(0)->setSelectable(false);
  ui->customPlot_2->addGraph(); // red line
  ui->customPlot_2->graph(1)->setPen(QPen(Qt::red));
  ui->customPlot_2->graph(1)->setName("Y");
  ui->customPlot_2->graph(1)->setSelectable(false);
  ui->customPlot_2->addGraph(); // green line
  ui->customPlot_2->graph(2)->setPen(QPen(Qt::green));
  ui->customPlot_2->graph(2)->setName("Z");
  ui->customPlot_2->graph(2)->setSelectable(false);

  // Scale in a particular direction when an axis is selected
  connect(ui->customPlot_2, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(mousePress()));
  connect(ui->customPlot_2, SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(mouseWheel()));

  // Make bottom and left axes transfer their ranges to top and right axes:
  connect(ui->customPlot_2->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlot_2->xAxis2, SLOT(setRange(QCPRange)));
  connect(ui->customPlot_2->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlot_2->yAxis2, SLOT(setRange(QCPRange)));

  // Adjust the tick marks on x-axis scale
  connect(ui->customPlot_2->xAxis, SIGNAL(rangeChanged(QCPRange)), this, SLOT(xAxisChanged(QCPRange)));

  // Context menu popup
  ui->customPlot_2->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(ui->customPlot_2, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenuRequest(QPoint)));
  /******************************************************************/
  ui->customPlot_3->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes |
                                  QCP::iSelectLegend | QCP::iSelectPlottables);



  ui->customPlot_3->xAxis->setBasePen(QPen(Qt::white, 1));
  ui->customPlot_3->yAxis->setBasePen(QPen(Qt::white, 1));
  ui->customPlot_3->xAxis->setTickPen(QPen(Qt::white, 1));
  ui->customPlot_3->yAxis->setTickPen(QPen(Qt::white, 1));
  ui->customPlot_3->xAxis->setSubTickPen(QPen(Qt::white, 1));
  ui->customPlot_3->yAxis->setSubTickPen(QPen(Qt::white, 1));
  ui->customPlot_3->xAxis->setTickLabelColor(Qt::white);
  ui->customPlot_3->yAxis->setTickLabelColor(Qt::white);
  ui->customPlot_3->xAxis->grid()->setPen(QPen(QColor(140, 140, 140), 1, Qt::DotLine));
  ui->customPlot_3->yAxis->grid()->setPen(QPen(QColor(140, 140, 140), 1, Qt::DotLine));
  ui->customPlot_3->xAxis->grid()->setSubGridPen(QPen(QColor(80, 80, 80), 1, Qt::DotLine));
  ui->customPlot_3->yAxis->grid()->setSubGridPen(QPen(QColor(80, 80, 80), 1, Qt::DotLine));
  ui->customPlot_3->xAxis->grid()->setSubGridVisible(true);
  ui->customPlot_3->yAxis->grid()->setSubGridVisible(true);
  ui->customPlot_3->xAxis->grid()->setZeroLinePen(Qt::NoPen);
  ui->customPlot_3->yAxis->grid()->setZeroLinePen(Qt::NoPen);

  // Axes
  ui->customPlot_3->xAxis->setTickLabelType(QCPAxis::ltDateTime);
  ui->customPlot_3->xAxis->setDateTimeFormat("hh:mm:ss");
  ui->customPlot_3->xAxis->setAutoTickStep(false);
  ui->customPlot_3->xAxis->setTickStep(2);
  ui->customPlot_3->xAxis->setLabel("Time");
  ui->customPlot_3->xAxis->setLabelColor(Qt::white);
  ui->customPlot_3->xAxis->setLabelPadding(0);
  ui->customPlot_3->xAxis->setSelectableParts(QCPAxis::spAxis);

  ui->customPlot_3->yAxis->setRange(-20, 20);
  ui->customPlot_3->yAxis->setLabel("Acceleration (m/s^2)");
  ui->customPlot_3->yAxis->setLabelColor(Qt::white);
  ui->customPlot_3->yAxis->setLabelPadding(0);
  ui->customPlot_3->yAxis->setSelectableParts(QCPAxis::spAxis);

  ui->customPlot_3->axisRect()->setupFullAxesBox();

  // Legend
  ui->customPlot_3->legend->setVisible(true);
  legendFont.setPointSize(10);
  ui->customPlot_3->legend->setFont(legendFont);
  ui->customPlot_3->legend->setSelectedFont(legendFont);
  ui->customPlot_3->legend->setSelectableParts(QCPLegend::spNone);

  plotGradient.setStart(0, 0);
  plotGradient.setFinalStop(0, 350);
  plotGradient.setColorAt(0, QColor(80, 80, 80));
  plotGradient.setColorAt(1, QColor(50, 50, 50));
  ui->customPlot_3->setBackground(plotGradient);

  // X, Y, Z reading graphs
  ui->customPlot_3->addGraph(); // blue line
  ui->customPlot_3->graph(0)->setPen(QPen(Qt::blue));
  ui->customPlot_3->graph(0)->setName("X");
  ui->customPlot_3->graph(0)->setSelectable(false);
  ui->customPlot_3->addGraph(); // red line
  ui->customPlot_3->graph(1)->setPen(QPen(Qt::red));
  ui->customPlot_3->graph(1)->setName("Y");
  ui->customPlot_3->graph(1)->setSelectable(false);
  ui->customPlot_3->addGraph(); // green line
  ui->customPlot_3->graph(2)->setPen(QPen(Qt::green));
  ui->customPlot_3->graph(2)->setName("Z");
  ui->customPlot_3->graph(2)->setSelectable(false);

  // Scale in a particular direction when an axis is selected
  connect(ui->customPlot_3, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(mousePress()));
  connect(ui->customPlot_3, SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(mouseWheel()));

  // Make bottom and left axes transfer their ranges to top and right axes:
  connect(ui->customPlot_3->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlot_3->xAxis2, SLOT(setRange(QCPRange)));
  connect(ui->customPlot_3->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlot_3->yAxis2, SLOT(setRange(QCPRange)));

  // Adjust the tick marks on x-axis scale
  connect(ui->customPlot_3->xAxis, SIGNAL(rangeChanged(QCPRange)), this, SLOT(xAxisChanged(QCPRange)));

  // Context menu popup
  ui->customPlot_3->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(ui->customPlot_3, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenuRequest(QPoint)));
  /******************************************************************/

  // Buttons  
  connect(ui->buttonStartStop, SIGNAL(released()), this, SLOT(pressedStartStop()));
//  connect(ui->buttonAnalyze, SIGNAL(released()), this, SLOT(pressedAnalyze()));
//  connect(ui->buttonSave, SIGNAL(released()), this, SLOT(pressedSave()));

  //Combox
  //connect(ui->comboBox, SIGNAL(currentIndexChanged(int index)), this, SLOT(on_comboBox_currentIndexChanged(int index)));

  // Setup accelerometer
  accelerometer = new QAccelerometer(this);
  accelerometer->setAccelerationMode(QAccelerometer::Combined);
  accelerometer->setDataRate(30);
  accelerometer->addFilter(&filter);

  // We'll always keep it running since on the phone apparently calling
  // stop makes the whole application freeze. Thus, we'll just start/stop
  // recording data not actually requesting data from the accelerometer
  accelerometer->start();

  // Button enabled/disabled coloring
  button_enabled = ui->buttonStartStop->palette();
  button_disabled = ui->buttonStartStop->palette();
  button_enabled.setColor(QPalette::Button, QColor(Qt::white));
  button_disabled.setColor(QPalette::Button, QColor(Qt::lightGray));

  // Load previous settings
  bool timed = settings->value("timed").toBool();
//  ui->checkBoxTimed->setChecked(timed);

  // When we start the application, either start it automatically if we're
  // not using a fixed period of time or set it up so we can press Start to
  // start it if we are using a fixed period of time
  if (timed)
    stop();
  else
    start();

  // Setup a timer that repeatedly calls MainWindow::realtimeDataSlot
  connect(&dataTimer, SIGNAL(timeout()), this, SLOT(realtimeDataSlot()));

  // Stop it after a certain period of time
  connect(&finishTimer, SIGNAL(timeout()), this, SLOT(finishSlot()));

  // Start with delay to reduce effect of touching the "Start"
  // button when recording for set amount of time
  connect(&startTimer, SIGNAL(timeout()), this, SLOT(startSlot()));
}


MainWindow::~MainWindow()
{
  delete accelerometer;
  delete ui;
}

void MainWindow::buttonEnable(QPushButton* button, bool enabled)
{
  if (!button)
      return;

  button->setEnabled(enabled);
  button->setAutoFillBackground(true);
  button->setPalette((enabled)?button_enabled:button_disabled);
  button->update();
}

void MainWindow::start()
{
  started = true;
  first = true;

  // Stop calling start
  startTimer.stop();

  // Disable buttons
  buttonEnable(ui->buttonStartStop, true);
//  buttonEnable(ui->buttonAnalyze, false);
//  buttonEnable(ui->buttonSave, false);
//  ui->checkBoxTimed->setEnabled(false);
  ui->buttonStartStop->setText("Stop");

  // Save the last used setting
//  settings->setValue("timed", ui->checkBoxTimed->isChecked());

  // Clear old data
  ui->customPlot->graph(0)->removeDataAfter(0);
  ui->customPlot->graph(1)->removeDataAfter(0);
  ui->customPlot->graph(2)->removeDataAfter(0);

  ui->customPlot_1->graph(0)->removeDataAfter(0);
  ui->customPlot_1->graph(1)->removeDataAfter(0);
  ui->customPlot_1->graph(2)->removeDataAfter(0);

  ui->customPlot_2->graph(0)->removeDataAfter(0);
  ui->customPlot_2->graph(1)->removeDataAfter(0);
  ui->customPlot_2->graph(2)->removeDataAfter(0);

  ui->customPlot_3->graph(0)->removeDataAfter(0);
  ui->customPlot_3->graph(1)->removeDataAfter(0);
  ui->customPlot_3->graph(2)->removeDataAfter(0);

  // Start the recording
  filter.start();
  dataTimer.start(0); // Interval 0 means to refresh as fast as possible

  // If checked, only run for a certain period of time
//  if (ui->checkBoxTimed->isChecked())
//    finishTimer.start(5*1000);
}

void MainWindow::delayStart()
{
  started = true;

  // Disable all buttons until it starts running
  buttonEnable(ui->buttonStartStop, true);
//  buttonEnable(ui->buttonAnalyze, false);
//  buttonEnable(ui->buttonSave, false);
//  ui->checkBoxTimed->setEnabled(false);
  ui->buttonStartStop->setText("Stop");

  // Delay 2 seconds
  startTimer.start(2000);
}

void MainWindow::stop()
{
  started = false;

  // Stop recording
  filter.stop();
  dataTimer.stop();

  // If not recording yet, cancel starting it soon
  startTimer.stop();

  // Enable buttons
  ui->buttonStartStop->setText("Start");
  buttonEnable(ui->buttonStartStop, true);
//  buttonEnable(ui->buttonAnalyze, true);
//  buttonEnable(ui->buttonSave, true);
//  ui->checkBoxTimed->setEnabled(true);

  // You'll probably be looking for these analysis values if you're using
  // the set time mode. And, make sure there's actually data to display.
//  if (ui->checkBoxTimed->isChecked() && !filter.empty())
//      pressedAnalyze();
}

void MainWindow::finishSlot()
{
    stop();
}

void MainWindow::startSlot()
{
    start();
}

void MainWindow::xAxisChanged(QCPRange range)
{
  double diff = range.upper - range.lower;
  ui->customPlot->xAxis->setTickStep(diff/5);

  if (diff < 1)
    ui->customPlot->xAxis->setDateTimeFormat("hh:mm:ss.zzz");
  else
    ui->customPlot->xAxis->setDateTimeFormat("hh:mm:ss");
}

void MainWindow::mousePress()
{
  // if an axis is selected, only allow the direction of that axis to be dragged
  // if no axis is selected, both directions may be dragged  
  if (ui->customPlot->xAxis->selectedParts().testFlag(QCPAxis::spAxis))
    ui->customPlot->axisRect()->setRangeDrag(ui->customPlot->xAxis->orientation());
  else if (ui->customPlot->yAxis->selectedParts().testFlag(QCPAxis::spAxis))
    ui->customPlot->axisRect()->setRangeDrag(ui->customPlot->yAxis->orientation());
  else
    ui->customPlot->axisRect()->setRangeDrag(Qt::Horizontal|Qt::Vertical);
}


void MainWindow::mouseWheel()
{
  // if an axis is selected, only allow the direction of that axis to be zoomed
  // if no axis is selected, both directions may be zoomed
  
  if (ui->customPlot->xAxis->selectedParts().testFlag(QCPAxis::spAxis))
    ui->customPlot->axisRect()->setRangeZoom(ui->customPlot->xAxis->orientation());
  else if (ui->customPlot->yAxis->selectedParts().testFlag(QCPAxis::spAxis))
    ui->customPlot->axisRect()->setRangeZoom(ui->customPlot->yAxis->orientation());
  else
    ui->customPlot->axisRect()->setRangeZoom(Qt::Horizontal|Qt::Vertical);
}

void MainWindow::contextMenuRequest(QPoint pos)
{
  if (ui->customPlot->legend->selectTest(pos, false) >= 0) // context menu on legend requested
  {
    QMenu *menu = new QMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);
    menu->addAction("Move to top left", this, SLOT(moveLegend()))->setData((int)(Qt::AlignTop|Qt::AlignLeft));
    menu->addAction("Move to top center", this, SLOT(moveLegend()))->setData((int)(Qt::AlignTop|Qt::AlignHCenter));
    menu->addAction("Move to top right", this, SLOT(moveLegend()))->setData((int)(Qt::AlignTop|Qt::AlignRight));
    menu->addAction("Move to bottom right", this, SLOT(moveLegend()))->setData((int)(Qt::AlignBottom|Qt::AlignRight));
    menu->addAction("Move to bottom left", this, SLOT(moveLegend()))->setData((int)(Qt::AlignBottom|Qt::AlignLeft));
    menu->popup(ui->customPlot->mapToGlobal(pos));
  }
}


void MainWindow::moveLegend()
{
  if (QAction* contextAction = qobject_cast<QAction*>(sender())) // make sure this slot is really called by a context menu action, so it carries the data we need
  {
    bool ok;
    int dataInt = contextAction->data().toInt(&ok);
    if (ok)
    {
      ui->customPlot->axisRect()->insetLayout()->setInsetAlignment(0, (Qt::Alignment)dataInt);
      ui->customPlot->replot();
    }
  }
}


void MainWindow::realtimeDataSlot()
{
  if (first)
  {
    // These are here as well as in start() so that they actually do something
    // when the application is first started up.
    buttonEnable(ui->buttonStartStop, true);
//    buttonEnable(ui->buttonAnalyze, false);
//    buttonEnable(ui->buttonSave, false);
//    ui->checkBoxTimed->setEnabled(false);
  }

  bool newData = false;
  double key = QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0;
  static double lastPointKey = 0;

  if (key-lastPointKey > 0.01) // at most add point every 10 ms
  {
    //AccelerometerReadingDisplay reading = filter.get();
    //newData = reading.newData;

    //added by sundy
    //cic_->getData();

    //if (reading.newData)
//    if (cic_->new_data_){
//      //get current node number

//      Node curr_node;
//      curr_node.ac_x = 2;
//      curr_node.ac_y = 4;
//      curr_node.ac_z = 6;
//      switch(cur_index_){
//        case 0:
//          curr_node = cic_->nodeInfo[0];
//          break;
//      case 1:
//          curr_node = cic_->nodeInfo[1];
//          break;
//      case 2:
//          curr_node = cic_->nodeInfo[2];
//          break;
//      case 3:
//          curr_node = cic_->nodeInfo[3];
//          break;
//      case 4:
//          curr_node = cic_->nodeInfo[4];
//          break;
//      }
//      double x = curr_node.ac_x;
//      double y = curr_node.ac_y;
//      double z = curr_node.ac_z;
//      ui->customPlot->graph(0)->addData(key,x);
//      ui->customPlot->graph(1)->addData(key,y);
//      ui->customPlot->graph(2)->addData(key,z);
//    }else{

//      ui->customPlot->graph(0)->addData(key,qSin(key)+qrand()/(double)RAND_MAX*1*qSin(key/0.3843));
//      ui->customPlot->graph(1)->addData(key,qSin(key)+qrand()/(double)RAND_MAX*4*qSin(key/0.3843));
//      ui->customPlot->graph(2)->addData(key,qSin(key)+qrand()/(double)RAND_MAX*6*qSin(key/0.3843));
//      //ui->customPlot->graph(0)->addData(key, reading.x);
//      //ui->customPlot->graph(1)->addData(key, reading.y);
//      //ui->customPlot->graph(2)->addData(key, reading.z);
//    }

    //if(cur_index_ == 2){
        ui->customPlot->graph(0)->addData(key,qSin(key)+qrand()/(double)RAND_MAX*1*qSin(key/0.3843));
        ui->customPlot->graph(1)->addData(key,qSin(key)+qrand()/(double)RAND_MAX*4*qSin(key/0.3843));
        ui->customPlot->graph(2)->addData(key,qSin(key)+qrand()/(double)RAND_MAX*6*qSin(key/0.3843));

        ui->customPlot_1->graph(0)->addData(key,qSin(key)+qrand()/(double)RAND_MAX*1*qSin(key/0.3843));
        ui->customPlot_1->graph(1)->addData(key,qSin(key)+qrand()/(double)RAND_MAX*4*qSin(key/0.3843));
        ui->customPlot_1->graph(2)->addData(key,qSin(key)+qrand()/(double)RAND_MAX*6*qSin(key/0.3843));


        ui->customPlot_2->graph(0)->addData(key,qSin(key)+qrand()/(double)RAND_MAX*1*qSin(key/0.3843));
        ui->customPlot_2->graph(1)->addData(key,qSin(key)+qrand()/(double)RAND_MAX*4*qSin(key/0.3843));
        ui->customPlot_2->graph(2)->addData(key,qSin(key)+qrand()/(double)RAND_MAX*6*qSin(key/0.3843));


        ui->customPlot_3->graph(0)->addData(key,qSin(key)+qrand()/(double)RAND_MAX*1*qSin(key/0.3843));
        ui->customPlot_3->graph(1)->addData(key,qSin(key)+qrand()/(double)RAND_MAX*4*qSin(key/0.3843));
        ui->customPlot_3->graph(2)->addData(key,qSin(key)+qrand()/(double)RAND_MAX*6*qSin(key/0.3843));
    //}
    lastPointKey = key;
  }

  if (ui->customPlot->getPaused() == 0)
  {
    // make key axis range scroll with the data (at a constant range size of 8)
    // but only if we haven't scrolled to the left
    QCPRange xrange = ui->customPlot->xAxis->range();
    if ((xrange.upper >= key-0.5 && xrange.upper <= key+0.5) || first){
       ui->customPlot->xAxis->setRange(key+0.25, xrange.upper-xrange.lower, Qt::AlignRight);
       ui->customPlot_1->xAxis->setRange(key+0.25, xrange.upper-xrange.lower, Qt::AlignRight);
       ui->customPlot_2->xAxis->setRange(key+0.25, xrange.upper-xrange.lower, Qt::AlignRight);
       ui->customPlot_3->xAxis->setRange(key+0.25, xrange.upper-xrange.lower, Qt::AlignRight);
    }
    first = false;

    ui->customPlot->replot();
    ui->customPlot_1->replot();
    ui->customPlot_2->replot();
    ui->customPlot_3->replot();
  }


  // Calculate frames per second and readings per second
  static double lastFpsKey = 0;
  static int frameCount = 0;
  static int readingCount = 0;
  ++frameCount;

  if (newData)
    ++readingCount;

  if (key-lastFpsKey > 2) // average fps over 2 seconds
  {
    ui->statusBar->showMessage(
          QString("%1 FPS, %2 RPS, Readings: %3")
          .arg(frameCount/(key-lastFpsKey), 0, 'f', 0)
          .arg(readingCount/(key-lastFpsKey), 0, 'f', 0)
          .arg(ui->customPlot->graph(0)->data()->count())
          , 0);
    lastFpsKey = key;
    frameCount = 0;
    readingCount = 0;
  }
}


void MainWindow::pressedStartStop()
{
    int com_port = 3;
    com_port = settings->value("comm").toInt();
    qDebug()<<"comm: "<<com_port;
    if (started)
        stop();
//    else if (ui->checkBoxTimed->isChecked())
//        delayStart(); // Reduce effect of tapping "Start" on the readings
    else{
        cic_= std::make_shared<cic::CIC>(com_port);
        int ret = cic_->connectSerial();
        if(ret){
            start();
        }else{
            char err[64];
            sprintf_s(err, "COM%d is not connected.\n", com_port);
            QMessageBox::critical(this, "Error", err);
            stop();
        }
        start();   // sundy
    }
}

void MainWindow::pressedAnalyze()
{
    buttonEnable(ui->buttonStartStop, false);
//    buttonEnable(ui->buttonAnalyze, false);
//    buttonEnable(ui->buttonSave, false);

    std::vector<AccelerometerReading> history = filter.getAll();
    QString msg;

    if (history.empty())
    {
        msg = "No data.";
    }
    else
    {
      double sum_x = 0;
      double sum_y = 0;
      double sum_z = 0;
      double sum_xm2 = 0;
      double sum_ym2 = 0;
      double sum_zm2 = 0;
      double stdev_s_x = 0;
      double stdev_s_y = 0;
      double stdev_s_z = 0;

      std::for_each(history.begin(), history.end(),
          [&sum_x, &sum_y, &sum_z](const AccelerometerReading& r) {
          sum_x += r.x;
          sum_y += r.y;
          sum_z += r.z;
      });

      double avg_x = sum_x/history.size();
      double avg_y = sum_y/history.size();
      double avg_z = sum_z/history.size();

      std::for_each(history.begin(), history.end(),
          [&sum_xm2, &sum_ym2, &sum_zm2,
            &avg_x, &avg_y, &avg_z](const AccelerometerReading& r) {
          sum_xm2 += std::pow(1.0*r.x - avg_x, 2);
          sum_ym2 += std::pow(1.0*r.y - avg_y, 2);
          sum_zm2 += std::pow(1.0*r.z - avg_z, 2);
      });

      if (history.size() > 1)
      {
        stdev_s_x = std::sqrt(1.0/(history.size()-1)*sum_xm2);
        stdev_s_y = std::sqrt(1.0/(history.size()-1)*sum_ym2);
        stdev_s_z = std::sqrt(1.0/(history.size()-1)*sum_zm2);
      }

      QTextStream s(&msg);
      s << "Averages (m/s^2):" << endl
        << " X " << QString::number(avg_x, 'f', 8) << endl
        << " Y " << QString::number(avg_y, 'f', 8) << endl
        << " Z " << QString::number(avg_z, 'f', 8) << endl
        << endl
        << "Sample Stdev (m/s^2):" << endl
        << " X " << QString::number(stdev_s_x, 'f', 8) << endl
        << " Y " << QString::number(stdev_s_y, 'f', 8) << endl
        << " Z " << QString::number(stdev_s_z, 'f', 8) << endl;
    }

    QMessageBox m;
    m.setText(msg);
    m.setStandardButtons(QMessageBox::Ok);
    m.setDefaultButton(QMessageBox::Ok);
    m.exec();

    buttonEnable(ui->buttonStartStop, true);
//    buttonEnable(ui->buttonAnalyze, true);
//    buttonEnable(ui->buttonSave, true);
}

void MainWindow::pressedSave()
{
  buttonEnable(ui->buttonStartStop, false);
//  buttonEnable(ui->buttonAnalyze, false);
//  buttonEnable(ui->buttonSave, false);

  // Note, at the moment we get the following message and this is empty.
  //   io\qstandardpaths_winrt.cpp:118: class QString __cdecl
  //   QStandardPaths::writableLocation(enum QStandardPaths::
  //   StandardLocation): Unimplemented code.
  QString path = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);

  if (!path.isEmpty())
    path += QDir::separator();

  QString date = QDateTime::currentDateTime().toString("yyyyMMdd_hhmm");
  QString default_filename = path + "accelerometer_" + date + ".csv";
  QString filename = QFileDialog::getSaveFileName(this,
                        tr("Save File"), default_filename, tr("*.csv"));

  if (!filename.isEmpty())
  {
//    buttonEnable(ui->buttonSave, false);
    writeFile(filename);
  }

  buttonEnable(ui->buttonStartStop, true);
//  buttonEnable(ui->buttonAnalyze, true);
//  buttonEnable(ui->buttonSave, true);
}

void MainWindow::writeFile(const QString& filename)
{
  QFile file(filename);

  if (file.open(QFile::WriteOnly | QFile::Truncate | QFile::Text))
  {
    QTextStream f(&file);
    std::vector<AccelerometerReading> history = filter.getAll();

    f << "timestamp, x (m/s^2), y (m/s^2), z (m/s^2)" << endl;

    for (std::vector<AccelerometerReading>::const_iterator i = history.begin();
         i != history.end(); ++i)
      f << i->time << ", " << i->x << ", " << i->y << ", " << i->z << endl;
  }
  else
  {
      QMessageBox m;
      m.setText("Could not save file.");
      m.setStandardButtons(QMessageBox::Ok);
      m.setDefaultButton(QMessageBox::Ok);
      m.exec();
  }

//  buttonEnable(ui->buttonSave, true);
}

//void MainWindow::on_comboBox_currentIndexChanged(int index)
//{
//    cur_index_ = index;
//}
