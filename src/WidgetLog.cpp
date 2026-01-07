#include "WidgetLog.h"

void WidgetLog::SlotAppendLog(const QString& message) {
  //QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
  //QString logLine = QString("[%1] %2").arg(timestamp, message);
  QString logLine = QString("%1").arg(message);

  // Ensure thread-safety by using QueuedConnection for UI updates
  QMetaObject::invokeMethod(m_logArea, "appendPlainText",
    Qt::QueuedConnection, Q_ARG(QString, logLine));
}

void WidgetLog::saveLogToFile() {
  QString fileName = QFileDialog::getSaveFileName(this, "Save Log File",
    QDir::homePath(),
    "Text Files (*.txt);;All Files (*)");
  if (fileName.isEmpty()) return;

  QFile file(fileName);
  if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    QTextStream out(&file);
    out << m_logArea->toPlainText();
    file.close();
  }
}

void WidgetLog::clearLogs() {
  m_logArea->clear();
}


void WidgetLog::setupUI() {
  auto* layout = new QVBoxLayout(this);
  layout->setContentsMargins(5, 5, 5, 5);

  m_logArea = new QPlainTextEdit(this);
  QFont monoFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
  monoFont.setPointSize(12);
  m_logArea->setFont(monoFont);


  m_logArea->setReadOnly(true);
  m_logArea->setMaximumBlockCount(10000); // Limit logs to 10,000 lines for performance
  m_logArea->setPlaceholderText("Logs will be displayed here...");

  // Layout for control buttons
  auto* btnLayout = new QHBoxLayout();
  auto* btnSave = new QPushButton("Save to File", this);
  auto* btnClear = new QPushButton("Clear Logs", this);

  QObject::connect(btnSave, &QPushButton::clicked, this, &WidgetLog::saveLogToFile);
  QObject::connect(btnClear, &QPushButton::clicked, this, &WidgetLog::clearLogs);

  btnLayout->addStretch();
  btnLayout->addWidget(btnSave);
  btnLayout->addWidget(btnClear);

  layout->addWidget(m_logArea);
  layout->addLayout(btnLayout);

  this->setLayout(layout);
}

