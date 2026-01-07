#ifndef WIDGETLOG_H
#define WIDGETLOG_H

#include <QWidget>
#include <QPlainTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QTextStream>
#include <QDateTime>
#include <QFile>

/**
 * @class WidgetLog
 * @brief A standalone widget for displaying, storing, and managing application logs.
 */
class WidgetLog : public QWidget {
  Q_OBJECT

public:
  explicit WidgetLog(QWidget* parent = nullptr) : QWidget(parent) {
    setupUI();
  }

public slots:
  /**
   * @brief Appends a new log message with a timestamp.
   * @param message The log content to be displayed.
   */
  void SlotAppendLog(const QString& message);
private slots:
  /**
   * @brief Opens a file dialog to save the current logs into a text file.
   */
  void saveLogToFile();
  /**
   * @brief Clears all logs from the display area.
   */
  void clearLogs();
private:
  /**
   * @brief Initializes the UI components and layouts.
   */
  void setupUI();
  QPlainTextEdit* m_logArea;
};

#endif // WIDGETLOG_H